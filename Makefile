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
VERSION  = $(or $(shell test -d .git && git describe --tags), "untagged")
LIB_CFLAGS  += -Wall -Wextra -Werror -g -fstack-protector-all -Isrc -fPIC
LIB_CFLAGS  += -DVERSION=\"${VERSION}\"
LIB_LDFLAGS +=

BIN_CFLAGS  += -DVERSION=\"${VERSION}\"
BIN_CFLAGS  += -Wall -Wextra -Werror -g -fstack-protector-all -Isrc
BIN_CFLAGS  += $(shell pkg-config --cflags libxml-2.0)
BIN_LDFLAGS += $(shell pkg-config --libs libxml-2.0)
BIN_LDFLAGS += -L. -lsja1105

BIN_SRC  = src/common.c src/common.h
LIB_SRC  = src/common.c src/common.h
BIN_SRC += $(shell find src/tool -name "*.[c|h]")  # All .c and .h files
BIN_DEPS = $(patsubst %.c, %.o, $(BIN_SRC))        # All .o and .h files
BIN_OBJ  = $(filter %.o, $(BIN_DEPS))              # Only the .o files

LIB_SRC += $(shell find src/lib -name "*.[c|h]")   # All .c and .h files
LIB_DEPS = $(patsubst %.c, %.o, $(LIB_SRC))        # All .o and .h files
LIB_OBJ  = $(filter %.o, $(LIB_DEPS))              # Only the .o files

MANPAGES = docs/man/sja1105-tool.1 \
           docs/man/sja1105-tool-status.1 \
           docs/man/sja1105-tool-reset.1 \
           docs/man/sja1105-tool-config.1 \
           docs/man/sja1105-tool-config-format.5 \
           docs/man/sja1105-conf.5
SJA1105_BIN = sja1105-tool
SJA1105_LIB = libsja1105.so

build: $(SJA1105_LIB) $(SJA1105_BIN)

$(SJA1105_LIB): $(LIB_DEPS)
	$(CC) -shared $(LIB_OBJ) -o $@ $(LIB_LDFLAGS)

$(SJA1105_BIN): $(BIN_DEPS) $(SJA1105_LIB)
	$(CC) $(BIN_OBJ) -o $@ $(BIN_LDFLAGS)

src/common.o: src/common.c
	$(CC) $(LIB_CFLAGS) -c $^ -o $@

src/tool/%.o: src/tool/%.c
	$(CC) $(BIN_CFLAGS) -c $^ -o $@

src/lib/%.o: src/lib/%.c
	$(CC) $(LIB_CFLAGS) -c $^ -o $@

clean:
	rm -f $(SJA1105_BIN) $(BIN_OBJ) $(SJA1105_LIB) $(LIB_OBJ)

man: $(MANPAGES)

all: build man

docs/man/%: docs/md/%.md
	pandoc --standalone --to man $^ -o $@

.PHONY: clean build man
