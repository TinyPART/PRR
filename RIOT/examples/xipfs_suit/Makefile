# name of your application
APPLICATION = xipfs

# If no BOARD is found in the environment, use this default:
BOARD ?= dwm1001

# Blacklisting msp430-based boards, as file syscalls are not supported
#FEATURES_BLACKLIST += arch_msp430

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Modules to include:
USEMODULE += shell
USEMODULE += shell_cmds_default
#USEMODULE += shell_cmd_suit
USEMODULE += ps

# Use the default file system
USEMODULE += vfs_default

# Networking
#
# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present

USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6_default
USEMODULE += sock_udp
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo

USEMODULE += suit suit_transport_coap
USEMODULE += suit_transport_vfs
USEMODULE += suit_storage_vfs

USE_ETHOS ?= 1
ifeq (1,$(USE_ETHOS))
  USEMODULE += stdio_ethos
  USEMODULE += gnrc_uhcpc
  ETHOS_BAUDRATE ?= 115200
  CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE)

  IFACE ?= riot0
  TERMPROG = $(RIOTTOOLS)/ethos/ethos
  TERMFLAGS = $(IFACE) $(PORT)
else
  USEMODULE += netdev_default
endif


# make sure ethos and uhcpd are built
TERMDEPS += host-tools
# enable VFS transport (only works on boards with external storage)

USEMODULE += xipfs

# Enable to automatically format if mount fails
#USEMODULE += vfs_auto_format

# allow to use large blocks to utilize large MTUs (802.15.4g, Ethernet, WiFi)
LARGE_BLOCKS ?= 0
ifeq (1, $(LARGE_BLOCKS))
  CFLAGS += -DCONFIG_NANOCOAP_BLOCKSIZE_DEFAULT=COAP_BLOCKSIZE_1024
else
# lower pktbuf size to something sufficient for this application
# Set GNRC_PKTBUF_SIZE via CFLAGS if not being set via Kconfig.
ifndef CONFIG_GNRC_PKTBUF_SIZE
  CFLAGS += -DCONFIG_GNRC_PKTBUF_SIZE=2000
endif
endif

.PHONY: host-tools

host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)

include $(RIOTBASE)/Makefile.include
