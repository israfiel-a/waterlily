###############################################################################
## This file provides the Make build script for the Waterlily game engine. It
## contains logic for cloning dependencies, managing build presets, and
## packaging the library itself.
##
## Copyright (c) 2025 Israfil Argos
## This program is free software: you can redistribute it and/or modify it 
## under the terms of the GNU General Public License as published by the Free 
## Software Foundation, either version 3 of the License, or (at your option) 
## any later version.
## 
## This program is distributed in the hope that it will be useful, but 
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
## for more details.
## 
## You should have received a copy of the GNU General Public License along 
## with this program.  If not, see <https://www.gnu.org/licenses/>.
###############################################################################

.PHONY: all clean prep install

###############################################################################
## Figure out the directory structure of the project build.
###############################################################################

ifeq ($(BUILD),)
BUILD:=$(abspath build)
else
BUILD:=$(abspath $(BUILD))
endif

ifeq ($(PREFIX),)
PREFIX:=/usr/local
else
PREFIX:=$(abspath $(PREFIX))
endif

SOURCE=$(abspath Source)
INCLUDE=$(abspath Include)

###############################################################################
## Set up the flags we're going to use in all compilations.
###############################################################################

CFLAGS=-std=gnu2x -Wall -Wextra -Wpedantic -Werror -I$(INCLUDE) 
DBGCFLAGS=-Og -g3 -ggdb -fanalyzer -fsanitize=leak -fsanitize=address$\ 
	-fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined
RELCFLAGS=-march=native -mtune=native -Ofast -flto

LDFLAGS=-nostartfiles
DBGLDFLAGS=-fsanitize=address -fsanitize=undefined
RELLDFLAGS=-Ofast -flto

ARFLAGS=rcs
DBGARFLAGS=v
RELARFLAGS=

###############################################################################
## Define the project's output files.
###############################################################################

OBJECTS=Engine.o Files.o Input.o Public.o Vulkan.o Window.o
OUTPUTS=$(foreach obj, $(OBJECTS), $(addprefix $(BUILD)/, $(obj)))
LIBRARY=$(BUILD)/libWaterlily.a

###############################################################################
## Define the project's various non-build tasks.
###############################################################################

EXPORT_COMMAND:=grep -wE '$(CC)'$\
	| grep -w '\-c'$\
	| jq -nR '[inputs|{directory:"$(BUILD)",$\
		command:.,$\
		file:match(" [^ ]+$$").string[1:],$\
		output:"$(BUILD)"+match(" [^ ]+$$").$\
			string[1+("$(SOURCE)"|length):-2]+".o"}]'$\
	> $(BUILD)/compile_commands.json

all: prep $(LIBRARY) 

prep:
	@mkdir -p $(BUILD)

install: all 
	install -d $(DESTDIR)$(PREFIX)/lib 
	install -d $(DESTDIR)$(PREFIX)/include 
	install -m 644 $(LIBRARY) $(DESTDIR)$(PREFIX)/lib
	install -m 644 $(INCLUDE)/Waterlily.h $(DESTDIR)$(PREFIX)/include

clean:
	rm -rf $(BUILD)

export_commands: prep
ifeq (, $(shell which jq))
	$(error "The JQ JSON utility was not found.")
endif
	$(MAKE) --always-make --dry-run | $(EXPORT_COMMAND) 

###############################################################################
## Define the project's build tasks.
###############################################################################

$(LIBRARY): $(OUTPUTS) $(INCLUDE)/Waterlily.h
	$(AR) $(ARFLAGS) $(LIBRARY) $(OUTPUTS)

$(BUILD)/%.o: $(SOURCE)/%.c $(SOURCE)/Internal.h
ifdef debug
	$(CC) -c $(CFLAGS) -DFILENAME=\"$(notdir $<)\" $(DBGCFLAGS) $(LDFLAGS)\
		$(DBGLDFLAGS) -o $@ $<
else
	$(CC) -c $(CFLAGS) -DFILENAME=\"$(notdir $<)\" $(RELCFLAGS) $(LDFLAGS)\
		$(RELLDFLAGS) -o $@ $<
endif

