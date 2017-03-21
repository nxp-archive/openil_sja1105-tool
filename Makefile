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
CFLAGS  += -DVERSION=\"${VERSION}\"
CFLAGS  += -Wall -Wextra -g -fstack-protector-all
CFLAGS  += $(shell pkg-config --cflags libxml-2.0)
LDFLAGS += $(shell pkg-config --libs libxml-2.0)
SRC      = $(shell find src -name "*.[c|h]")       # All .c and .h files
DEPS     = $(patsubst %.c, %.o, $(SRC))            # All .o and .h files
OBJ      = $(filter %.o, $(DEPS))                  # Only the .o files
MANPAGES = docs/man/sja1105-tool.1 \
           docs/man/sja1105-tool-status.1 \
           docs/man/sja1105-tool-reset.1 \
           docs/man/sja1105-tool-config.1 \
           docs/man/sja1105-tool-config-format.5 \
           docs/man/sja1105-conf.5
SJA1105  = sja1105-tool

build: $(SJA1105)

$(SJA1105): $(DEPS)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f $(SJA1105) $(OBJ)

man: $(MANPAGES)

all: build man

docs/man/%: docs/md/%.md
	pandoc --standalone --to man $^ -o $@

.PHONY: clean build man
