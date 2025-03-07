/*
 * Copyright (C) 2017-2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_nrf5x_nrfble
 *
 * @note        This driver is not thread safe and should only be used by a
 *              single thread at once!
 * @{
 *
 * @file
 * @brief       Bluetooth low energy radio driver for nRF5x SoCs
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <errno.h>

#include "cpu.h"
#include "assert.h"
#include "nrf_clock.h"

#include "nrfble.h"
#include "net/netdev/ble.h"

#include "svc.h"

#define ENABLE_DEBUG            0
#include "debug.h"

/* driver specific device configuration */
#define CONF_MODE               RADIO_MODE_MODE_Ble_1Mbit
#define CONF_LEN                (8U)
#define CONF_S0                 (1U)
#define CONF_S1                 (0U)
#define CONF_STATLEN            (0U)
#define CONF_BASE_ADDR_LEN      (3U)
#define CONF_ENDIAN             RADIO_PCNF1_ENDIAN_Little
#define CONF_WHITENING          RADIO_PCNF1_WHITEEN_Enabled
#define CONF_TIFS               (150U)
#define CONF_CRC_LEN            (0X3 | RADIO_CRCCNF_SKIPADDR_Msk)
#define CONF_CRC_POLY           (0x00065b)

/* used shortcuts */
#define SHORTS_BASE             (RADIO_SHORTS_READY_START_Msk | \
                                 RADIO_SHORTS_END_DISABLE_Msk)
#define SHORTS_RX               (SHORTS_BASE | RADIO_SHORTS_DISABLED_TXEN_Msk)
#define SHORTS_TX               (SHORTS_BASE | RADIO_SHORTS_DISABLED_RXEN_Msk)

/* interrupt masks */
#define INT_DIS                 (RADIO_INTENCLR_DISABLED_Msk | \
                                 RADIO_INTENCLR_ADDRESS_Msk)
#define INT_EN                  (RADIO_INTENSET_DISABLED_Msk | \
                                 RADIO_INTENSET_ADDRESS_Msk)

/* driver internal radio states */
#define STATE_IDLE              (0x00)
#define STATE_RX                (0x01)
#define STATE_TX                (0x02)
#define STATE_BUSY              (0x80)

/* forward declaration of the netdev driver struct */
static const netdev_driver_t netdev_driver;

/* current radio state */
static volatile uint8_t _state = STATE_IDLE;

/* active radio context */
static netdev_ble_ctx_t *_ctx = NULL;

/* allocate the netdev device descriptor */
netdev_t _nrfble_dev;

/* map logical BLE channel values to actual radio frequencies */
static const uint8_t _ble_chan_map[40] = {
    [ 0] = 4,
    [ 1] = 6,
    [ 2] = 8,
    [ 3] = 10,
    [ 4] = 12,
    [ 5] = 14,
    [ 6] = 16,
    [ 7] = 18,
    [ 8] = 20,
    [ 9] = 22,
    [10] = 24,
    [11] = 28,
    [12] = 30,
    [13] = 32,
    [14] = 34,
    [15] = 36,
    [16] = 38,
    [17] = 40,
    [18] = 42,
    [19] = 44,
    [20] = 46,
    [21] = 48,
    [22] = 50,
    [23] = 52,
    [24] = 54,
    [25] = 56,
    [26] = 58,
    [27] = 60,
    [28] = 62,
    [29] = 64,
    [30] = 66,
    [31] = 68,
    [32] = 70,
    [33] = 72,
    [34] = 74,
    [35] = 76,
    [36] = 78,
    [37] = 2,
    [38] = 26,
    [39] = 80,
};

/**
 * @brief   Enable the radio
 */
static void _enable(void)
{
    clock_hfxo_request();
    Pip_out(PIP_NRF_RADIO_RADIO_EVENTS_DISABLED, 0);
    Pip_out(PIP_NRF_RADIO_RADIO_INTENSET, INT_EN);
}

/**
 * @brief   Set radio into idle (DISABLED) state
 */
static void _go_idle(void)
{
    if (!(_state & STATE_BUSY)) {
        Pip_out(PIP_NRF_RADIO_RADIO_INTENCLR, INT_DIS);
        Pip_out(PIP_NRF_RADIO_RADIO_SHORTS, 0);
        Pip_out(PIP_NRF_RADIO_RADIO_EVENTS_DISABLED, 0);
        Pip_out(PIP_NRF_RADIO_RADIO_TASKS_DISABLE, 1);
        while (Pip_in(PIP_NRF_RADIO_RADIO_EVENTS_DISABLED) == 0) {}
        clock_hfxo_release();
        _state = STATE_IDLE;
    }
}

/**
 * @brief   Set radio context (channel, CRC, whitening, and access address)
 */
static void _set_context(netdev_ble_ctx_t *ctx)
{
    if (ctx) {
        assert(ctx->chan <= NRFBLE_CHAN_MAX);

        _ctx = ctx;
        Pip_out(PIP_NRF_RADIO_RADIO_FREQUENCY, _ble_chan_map[ctx->chan]);
        Pip_out(PIP_NRF_RADIO_RADIO_PREFIX0, ctx->aa.raw[3]);
        Pip_out(PIP_NRF_RADIO_RADIO_BASE0, (uint32_t)((ctx->aa.raw[0] << 8) |
                                        (ctx->aa.raw[1] << 16) |
                                        (ctx->aa.raw[2] << 24)));
        Pip_out(PIP_NRF_RADIO_RADIO_DATAWHITEIV, ctx->chan);
        Pip_out(PIP_NRF_RADIO_RADIO_CRCINIT, (ctx->crc & NETDEV_BLE_CRC_MASK));
    }
    else {
        _go_idle();
    }
}

static int16_t _nrfble_get_txpower(void)
{
    int8_t p;

    int8_t p = (int8_t)Pip_in(PIP_NRF_RADIO_RADIO_TXPOWER);
    if (p < 0) {
        return (int16_t)(0xff00 | p);
    }
    return (int16_t)p;
}

static void _nrfble_set_txpower(int16_t power)
{
    if (power > 2) {
        Pip(PIP_NRF_RADIO_RADIO_TXPOWER, RADIO_TXPOWER_TXPOWER_Pos4dBm);
    }
    else if (power > -2) {
        Pip(PIP_NRF_RADIO_RADIO_TXPOWER, RADIO_TXPOWER_TXPOWER_0dBm);
    }
    else if (power > -6) {
        Pip(PIP_NRF_RADIO_RADIO_TXPOWER, RADIO_TXPOWER_TXPOWER_Neg4dBm);
    }
    else if (power > -10) {
        Pip(PIP_NRF_RADIO_RADIO_TXPOWER, RADIO_TXPOWER_TXPOWER_Neg8dBm);
    }
    else if (power > -14) {
        Pip(PIP_NRF_RADIO_RADIO_TXPOWER, RADIO_TXPOWER_TXPOWER_Neg12dBm);
    }
    else if (power > -18) {
        Pip(PIP_NRF_RADIO_RADIO_TXPOWER,RADIO_TXPOWER_TXPOWER_Neg16dBm);
    }
    else if (power > -25) {
        Pip(PIP_NRF_RADIO_RADIO_TXPOWER, RADIO_TXPOWER_TXPOWER_Neg20dBm);
    }
    else {
        Pip(PIP_NRF_RADIO_RADIO_TXPOWER, RADIO_TXPOWER_TXPOWER_Neg30dBm);
    }
}

/**
 * @brief   Radio interrupt routine
 */
void isr_radio(void)
{
    if (Pip_in(PIP_NRF_RADIO_RADIO_EVENTS_ADDRESS)) {
        Pip_out(PIP_NRF_RADIO_RADIO_EVENTS_ADDRESS, 0);
        _state |= STATE_BUSY;
    }

    if (Pip_in(PIP_NRF_RADIO_RADIO_EVENTS_DISABLED)) {
        Pip_out(PIP_NRF_RADIO_RADIO_EVENTS_DISABLED, 0);

        if (_state == (STATE_BUSY | STATE_RX)) {
            _state = STATE_TX;
            if (Pip_in(PIP_NRF_RADIO_RADIO_CRCSTATUS)) {
                _ctx->crc |= NETDEV_BLE_CRC_OK;
            }
            else {
                _ctx->crc &= ~(NETDEV_BLE_CRC_OK);
            }
            _nrfble_dev.event_callback(&_nrfble_dev, NETDEV_EVENT_RX_COMPLETE);
        }
        else {  /* on TX done */
            _state = STATE_RX;
            _nrfble_dev.event_callback(&_nrfble_dev, NETDEV_EVENT_TX_COMPLETE);
        }
    }

    cortexm_isr_end();
}

netdev_t *nrfble_setup(void)
{
    _nrfble_dev.driver = &netdev_driver;
    _nrfble_dev.event_callback = NULL;
    _nrfble_dev.context = NULL;
    return &_nrfble_dev;
}

static int _nrfble_init(netdev_t *dev)
{
    (void)dev;
    assert(_nrfble_dev.driver && _nrfble_dev.event_callback);

    /* power cycle the radio to reset it */
    Pip_out(PIP_NRF_RADIO_RADIO_POWER, 0);
    Pip_out(PIP_NRF_RADIO_RADIO_POWER, 1);
    /* configure variable parameters to default values */
    Pip_out(PIP_NRF_RADIO_RADIO_TXPOWER, NRFBLE_TXPOWER_DEFAULT);
    /* always send from and listen to logical address 0 */
    Pip_out(PIP_NRF_RADIO_RADIO_TXADDRESS, 0x00UL);
    Pip_out(PIP_NRF_RADIO_RADIO_RXADDRESSES, 0x01UL);
    /* load driver specific configuration */
    Pip_out(PIP_NRF_RADIO_RADIO_MODE, CONF_MODE);
    /* configure data fields and packet length whitening and endianness */
    Pip_out(PIP_NRF_RADIO_RADIO_PCNF0, ((CONF_S1 << RADIO_PCNF0_S1LEN_Pos) |
                        (CONF_S0 << RADIO_PCNF0_S0LEN_Pos) |
                        (CONF_LEN << RADIO_PCNF0_LFLEN_Pos)));
    Pip_out(PIP_NRF_RADIO_RADIO_PCNF1, ((CONF_WHITENING << RADIO_PCNF1_WHITEEN_Pos) |
                        (CONF_ENDIAN << RADIO_PCNF1_ENDIAN_Pos) |
                        (CONF_BASE_ADDR_LEN << RADIO_PCNF1_BALEN_Pos) |
                        (CONF_STATLEN << RADIO_PCNF1_STATLEN_Pos) |
                        (NETDEV_BLE_PDU_MAXLEN << RADIO_PCNF1_MAXLEN_Pos)));
    /* set inter frame spacing to 150us */
    Pip_out(PIP_NRF_RADIO_RADIO_TIFS, CONF_TIFS);
    /* configure CRC length and polynomial */
    Pip_out(PIP_NRF_RADIO_RADIO_CRCCNF, CONF_CRC_LEN);
    Pip_out(PIP_NRF_RADIO_RADIO_CRCPOLY, CONF_CRC_POLY);
    /* enable global interrupts, but mask all local interrupts for now */
    Pip_out(PIP_NRF_RADIO_RADIO_INTENCLR, 0xffffffff);
    NVIC_EnableIRQ(RADIO_IRQn);

    DEBUG("[nrfble] initialization successful\n");

    /* signal link UP */
    dev->event_callback(dev, NETDEV_EVENT_LINK_UP);

    return 0;
}

static int _nrfble_send(netdev_t *dev, const iolist_t *data)
{
    (void)dev;
    assert(data);

    Pip_out(PIP_NRF_RADIO_RADIO_PACKETPTR, (uint32_t)data->iol_base);
    Pip_out(PIP_NRF_RADIO_RADIO_SHORTS, SHORTS_TX);

    /* in case no trx sequence is active, we start a new one now */
    if (_state == STATE_IDLE) {
        _state = STATE_TX;
        _enable();
        Pip_out(PIP_NRF_RADIO_RADIO_TASKS_TXEN, 1);
    }

    return 0;
}

static int _nrfble_recv(netdev_t *dev, void *buf, size_t len, void *info)
{
    (void)dev;
    (void)len;
    (void)info;
    assert(buf && (len == sizeof(netdev_ble_pkt_t)));

    Pip_out(PIP_NRF_RADIO_RADIO_PACKETPTR, (uint32_t)buf);
    Pip_out(PIP_NRF_RADIO_RADIO_SHORTS, SHORTS_RX);

    /* in case no trx sequence is active, we start a new one now */
    if (_state == STATE_IDLE) {
        _state = STATE_RX;
        _enable();
        Pip_out(PIP_NRF_RADIO_RADIO_TASKS_RXEN, 1);
    }

    return 0;
}

static int _nrfble_get(netdev_t *dev, netopt_t opt, void *val, size_t max_len)
{
    (void)dev;
    (void)max_len;

    switch (opt) {
        case NETOPT_TX_POWER:
            assert(max_len >= sizeof(int16_t));
            *((int16_t *)val) = _nrfble_get_txpower();
            return sizeof(int16_t);
        case NETOPT_DEVICE_TYPE:
            assert(max_len >= sizeof(uint16_t));
            *((uint16_t *)val) = NETDEV_TYPE_BLE;
            return sizeof(uint16_t);
        default:
            return -ENOTSUP;
    }
}

static int _nrfble_set(netdev_t *dev, netopt_t opt, const void *val, size_t len)
{
    (void)dev;
    (void)len;

    switch (opt) {
        case NETOPT_BLE_CTX:
            _set_context((netdev_ble_ctx_t *)val);
            return sizeof(netdev_ble_ctx_t);
        case NETOPT_TX_POWER:
            assert(len == sizeof(int16_t));
            _nrfble_set_txpower(*((const int16_t *)val));
            return sizeof(int16_t);
        default:
            return -ENOTSUP;
    }
}

/* export of the netdev interface */
static const netdev_driver_t netdev_driver = {
    .send = _nrfble_send,
    .recv = _nrfble_recv,
    .init = _nrfble_init,
    .isr  = NULL,
    .get  = _nrfble_get,
    .set  = _nrfble_set
};
