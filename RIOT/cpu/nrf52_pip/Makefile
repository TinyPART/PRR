# define the module that is build
MODULE = cpu

# add a list of subdirectories, that should also be build
DIRS = periph $(RIOTCPU)/cortexm_common_pip $(RIOTCPU)/nrf5x_common_pip vectors

# build the nrf802154 driver if selected
ifneq (,$(filter nrf802154,$(USEMODULE)))
    DIRS += radio/nrf802154
endif

include $(RIOTBASE)/Makefile.base
