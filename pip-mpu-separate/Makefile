###############################################################################
#  © Université de Lille, The Pip Development Team (2015-2022)                #
#                                                                             #
#  This software is a computer program whose purpose is to run a minimal,     #
#  hypervisor relying on proven properties such as memory isolation.          #
#                                                                             #
#  This software is governed by the CeCILL license under French law and       #
#  abiding by the rules of distribution of free software.  You can  use,      #
#  modify and/ or redistribute the software under the terms of the CeCILL     #
#  license as circulated by CEA, CNRS and INRIA at the following URL          #
#  "http://www.cecill.info".                                                  #
#                                                                             #
#  As a counterpart to the access to the source code and  rights to copy,     #
#  modify and redistribute granted by the license, users are provided only    #
#  with a limited warranty  and the software's author,  the holder of the     #
#  economic rights,  and the successive licensors  have only  limited         #
#  liability.                                                                 #
#                                                                             #
#  In this respect, the user's attention is drawn to the risks associated     #
#  with loading,  using,  modifying and/or developing or reproducing the      #
#  software by the user in light of its specific status of free software,     #
#  that may mean  that it is complicated to manipulate,  and  that  also      #
#  therefore means  that it is reserved for developers  and  experienced      #
#  professionals having in-depth computer knowledge. Users are therefore      #
#  encouraged to load and test the software's suitability as regards their    #
#  requirements in conditions enabling the security of their systems and/or   #
#  data to be ensured and,  more generally, to use and operate it in the      #
#  same conditions as regards security.                                       #
#                                                                             #
#  The fact that you are presently reading this means that you have had       #
#  knowledge of the CeCILL license and that you accept its terms.             #
###############################################################################

# Default configuration.
ROOT    := root.elf
PIP     := pip.elf

# User-defined configuration.
-include Makefile.config

PREFIX  := arm-none-eabi-

CC      := $(PREFIX)gcc
LD      := $(PREFIX)ld
READELF := $(PREFIX)readelf
OBJCOPY := $(PREFIX)objcopy
CAT     := cat
RM      := rm
AWK     := awk

CFLAGS    = -Wall
CFLAGS   += -Wextra
CFLAGS   += -Werror
CFLAGS   += -mthumb
CFLAGS   += -mcpu=cortex-m4
CFLAGS   += -ffreestanding
# Prevent GCC from generating code that uses the PIC
# register.
CFLAGS   += -ffixed-sl
CFLAGS   += -Os
CFLAGS   += -g3
CFLAGS   += -ggdb

all: pip+root.elf gdbinit

gdbinit: $(PIP) $(ROOT) crt0+meta.bin crt0/crt0.elf parser/common.awk parser/gdb.awk
	( LANG=C $(READELF) --wide --symbols $(PIP)\
        ; LANG=C $(READELF) --wide --symbols $(ROOT))\
        | $(AWK)\
          -f parser/common.awk\
          -f parser/gdb.awk\
          -v _crt0meta_size="$$(wc -c < crt0+meta.bin)"\
          -v pip=$(abspath $(PIP))\
          -v root=$(abspath $(ROOT))\
          -v crt0=$(abspath crt0/crt0.elf) > $@

pip+root.elf: pip+root.bin
# objcopy needs the output file not to be empty...
	@printf '\0' > $@
	$(OBJCOPY)\
          --input-target=binary\
          --output-target=elf32-littlearm\
          --wildcard\
          --remove-section '*'\
          --add-section .text=$<\
        $@

pip+root.bin: pip.bin crt0+meta+root.bin
	$(CAT) $^ > $@

crt0+meta+root.bin: crt0+meta.bin root.bin
	$(CAT) $^ > $@

crt0+meta.bin: crt0/crt0.bin meta.bin
	$(CAT) $^ > $@

meta.bin: $(ROOT) parser/common.awk parser/symbols.awk parser/patchinfo.awk
	LANG=C $(READELF) --wide --symbols $<\
        | LANG=C $(AWK) -f parser/common.awk -f parser/symbols.awk > $@
	LANG=C $(READELF) --wide --relocs  $<\
        | LANG=C $(AWK) -f parser/common.awk -f parser/patchinfo.awk >> $@

pip.bin: $(PIP) parser/common.awk parser/pad.awk
	$(OBJCOPY)\
          --input-target=elf32-littlearm\
          --output-target=binary\
          --pad-to="$$(\
              LANG=C $(READELF) --wide --symbols $(PIP)\
              | $(AWK) -f parser/common.awk -f parser/pad.awk\
          )"\
        $< $@

root.bin: $(ROOT)
	$(OBJCOPY)\
          --input-target=elf32-littlearm\
          --output-target=binary\
        $< $@

crt0/crt0.bin: crt0/crt0.elf
	$(OBJCOPY)\
          --input-target=elf32-littlearm\
          --output-target=binary\
        $< $@

crt0/crt0.elf: crt0/crt0.o
	$(LD) -T crt0/link.ld $< -o $@

crt0/crt0.o: crt0/crt0.c
	$(CC) $(CFLAGS) -c $< -o $@

realclean: clean
	$(RM) -f gdbinit\
                 crt0/crt0.elf\
                 pip+root.bin\
                 pip+root.elf

clean:
	$(RM) -f crt0/crt0.o\
                 crt0/crt0.bin\
                 meta.bin\
                 pip.bin\
                 root.bin\
                 crt0+meta.bin\
                 crt0+meta+root.bin

.PHONY: all realclean clean
