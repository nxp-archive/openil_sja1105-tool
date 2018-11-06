##############################################################################
# Copyright (c) 2016, NXP Semiconductors
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived from this
# software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
##############################################################################

prefix ?= /usr
exec_prefix ?= ${prefix}
bindir ?= ${exec_prefix}/bin
libdir ?= ${exec_prefix}/lib
includedir ?= ${prefix}/include
datarootdir ?= ${prefix}/share
mandir ?= ${datarootdir}/man
sysconfdir ?= /etc
PKG_CONFIG ?= pkg-config

# Binaries

# If taken from git, report the version relative to the latest tag
# If not, default to the VERSION file
VERSION = $(or $(shell test -d .git && git describe --tags), $(shell cat VERSION))
LIB_CFLAGS  := $(CFLAGS)
LIB_LDFLAGS := $(LDFLAGS)
LIB_CFLAGS  += -Wall -Wextra -Werror -g -fstack-protector-all -Isrc -fPIC
LIB_CFLAGS  += -DVERSION=\"${VERSION}\"
LIB_LDFLAGS +=

BIN_CFLAGS  := $(CFLAGS)
BIN_LDFLAGS := $(LDFLAGS)
BIN_CFLAGS  += -DVERSION=\"${VERSION}\"
BIN_CFLAGS  += -Wall -Wextra -Werror -g -fstack-protector-all -Isrc
BIN_CFLAGS  += $(shell ${PKG_CONFIG} --cflags libxml-2.0)
BIN_LDFLAGS += $(shell ${PKG_CONFIG} --libs libxml-2.0)

BIN_SRC  := src/common.c src/common.h
LIB_SRC  := src/common.c src/common.h
BIN_SRC  += $(shell find src/tool -name "*.[c|h]")  # All .c and .h files
BIN_DEPS := $(patsubst %.c, %.o, $(BIN_SRC))        # All .o and .h files
BIN_OBJ  := $(filter %.o, $(BIN_DEPS))              # Only the .o files

LIB_SRC  += $(shell find src/lib -name "*.[c|h]")   # All .c and .h files
LIB_DEPS := $(patsubst %.c, %.o, $(LIB_SRC))        # All .o and .h files
LIB_OBJ  := $(filter %.o, $(LIB_DEPS))              # Only the .o files

# Handling for the O= Make variable which sets the path of intermediary objects
ifneq ($(O),)
    override O := $(addsuffix /,$(O))
else
    override O := ./
endif

BIN_DEPS := $(patsubst %.o, $(addprefix $(O),%.o), $(BIN_DEPS))
LIB_DEPS := $(patsubst %.o, $(addprefix $(O),%.o), $(LIB_DEPS))
BIN_OBJ  := $(addprefix $(O),$(BIN_OBJ))
LIB_OBJ  := $(addprefix $(O),$(LIB_OBJ))
BIN_LDFLAGS += -L$(O) -lsja1105

SJA1105_BIN  := $(O)sja1105-tool
SJA1105_LIB  := $(O)libsja1105.so
SJA1105_KMOD := src/kmod/sja1105.ko

# Make targets
build: $(SJA1105_LIB) $(SJA1105_BIN) $(SJA1105_KMOD)

$(SJA1105_LIB): $(LIB_DEPS)
	$(CC) -shared $(LIB_OBJ) -o $@ $(LIB_LDFLAGS)

$(SJA1105_BIN): $(BIN_DEPS) $(SJA1105_LIB)
	$(CC) $(BIN_OBJ) -o $@ $(BIN_LDFLAGS)

#ifneq (,$(filter $(SJA1105_KMOD),$(MAKECMDGOALS)))
  ifeq ($(KDIR),)
    $(error Please set KDIR variable to point to a kernel source tree.)
  endif
#endif

# Determine kmod dependencies by parsing its Kbuild file
include src/kmod/Kbuild
KMOD_DEPS := $(addprefix src/kmod/, $(sja1105-y))

$(SJA1105_KMOD): $(KMOD_DEPS)
	@mkdir -p $(dir $@)
	$(MAKE) -C $(KDIR) M=$$PWD/src/kmod

$(O)src/common.o: src/common.c
	@mkdir -p $(dir $@)
	$(CC) $(LIB_CFLAGS) -c $^ -o $@

$(O)src/tool/%.o: src/tool/%.c
	@mkdir -p $(dir $@)
	$(CC) $(BIN_CFLAGS) -c $^ -o $@

$(O)src/lib/%.o: src/lib/%.c
	@mkdir -p $(dir $@)
	$(CC) $(LIB_CFLAGS) -c $^ -o $@

# Manpages

# Inputs
MD_DOCS  := $(wildcard docs/md/*.md)
# Outputs
PDF_DOCS := $(patsubst docs/md/%.md, $(O)docs/pdf/%.pdf, $(MD_DOCS))
MANPAGES := $(patsubst docs/md/%.md, $(O)docs/man/%, $(MD_DOCS))

# Input: file in the format docs/man/sja1105-conf.5
# Output: 5
define get_man_section
    $(lastword $(subst ., ,$1))
endef

# Input: path to manpage file from sources
# Output: DESTDIR-prefixed install location
define get_manpage_destination
    $(eval section := $(call get_man_section,$1))             \
    $(eval base_name := $(subst docs/man/,,$1))               \
    $(eval dest_prefix := $(DESTDIR)$(mandir)/man$(section)/) \
    $(addprefix $(dest_prefix), $(base_name))
endef

man: $(MANPAGES)

pdf: $(PDF_DOCS)

$(O)docs/man/%: docs/md/%.md
	@mkdir -p $(dir $@)
	pandoc --standalone --to man $^ -o $@

$(O)docs/pdf/%.pdf: docs/md/%.md
	@mkdir -p $(dir $@)
	pandoc --standalone -t latex $^ -o $@

# Headers

HEADERS := $(wildcard src/lib/include/*.h)

# Input: path to header file from sources
# Output: DESTDIR-prefixed install location
define get_header_destination
    $(eval new_prefix := $(DESTDIR)$(includedir)/sja1105) \
    $(patsubst src/lib/include/%, $(new_prefix)/%, $1)
endef

# Installation

install: install-binaries install-configs install-manpages install-headers

install-binaries: $(SJA1105_LIB) $(SJA1105_BIN)
	install -m 0755 -D $(SJA1105_LIB) $(DESTDIR)${libdir}/$(notdir $(SJA1105_LIB))
	install -m 0755 -D $(SJA1105_BIN) $(DESTDIR)${bindir}/$(notdir $(SJA1105_BIN))
	install -m 0755 -D etc/etsec_mdio $(DESTDIR)${bindir}/etsec_mdio

install-configs: etc/sja1105-init etc/sja1105.conf
	install -m 0644 -D etc/sja1105.conf $(DESTDIR)${sysconfdir}/sja1105/sja1105.conf

install-manpages: $(MANPAGES)
	$(foreach manpage, $^, install -m 0644 -D $(manpage) \
		$(call get_manpage_destination,$(manpage));)

install-headers: $(HEADERS)
	$(foreach header, $^, install -m 0644 -D $(header) \
		$(call get_header_destination,$(header));)

all: install

# Leave the directory structure in place
uninstall:
	$(foreach manpage, $(MANPAGES), \
		rm -rf $(call get_manpage_destination,$(manpage));)
	$(foreach header, $(HEADERS), \
		rm -rf $(call get_header_destination,$(header));)
	rm -rf $(DESTDIR)${libdir}/libsja1105.so
	rm -rf $(DESTDIR)${bindir}/sja1105-tool
	rm -rf $(DESTDIR)${bindir}/etsec_mdio
	rm -rf $(DESTDIR)${sysconfdir}/init.d/S46sja1105-link-speed-fixup
	rm -rf $(DESTDIR)${sysconfdir}/init.d/S45sja1105
	rm -rf $(DESTDIR)${sysconfdir}/sja1105/sja1105.conf

clean:
	rm -f $(SJA1105_BIN) $(BIN_OBJ) $(SJA1105_LIB) $(LIB_OBJ)
	find . -name '.*.cmd' -delete
	$(MAKE) -C $(KDIR) M=$$PWD/src/kmod clean

.PHONY: clean uninstall build man pdf install install-binaries \
	install-configs install-headers install-manpages $(KMOD_DEPS)
