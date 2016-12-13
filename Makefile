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

CFLAGS         += -Wall -Wextra -g -fstack-protector-all

CFLAGS += $(shell pkg-config --cflags libxml-2.0)
LDFLAGS+= $(shell pkg-config --libs   libxml-2.0)

SJA1105_SOURCE  = $(shell find src -name "*.[c|h]")
SJA1105         = sja1105-tool
PANDOC          = pandoc
MANPAGES        = sja1105-tool.1 \
                  sja1105-tool-config.1 \
                  sja1105-tool-status.1 \
                  sja1105-tool-reset.1 \
                  sja1105-tool-config-format.5 \
                  sja1105-conf.5

build: $(SJA1105)

$(SJA1105): $(SJA1105_SOURCE)
	$(CC) $(CFLAGS) $(filter %.c, $^) -o $@ $(LDFLAGS)

clean:
	rm -f $(SJA1105)
	rm -f $(MANPAGES)

man: $(MANPAGES)

all: build man

%.1: man/%.1.md
	$(PANDOC) --standalone --to man $^ -o $@
%.5: man/%.5.md
	$(PANDOC) --standalone --to man $^ -o $@

.PHONY: clean build
