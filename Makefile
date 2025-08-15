###############################################################################
## This file provides the Make build script for the Waterlily game engine. It
## contains logic for finding dependencies, managing build presets, and
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

.PHONY: all clean prep install export_commands

###############################################################################
## Utility functions/macros to simplify common tasks.
###############################################################################

define find_software
	echo "Finding $(2) at $(1)"
	$(if $(filter $(shell command -v $(1)),$(1)),echo "Found $(2).",$\
		$(error "Failed to find $(2)"))
endef

define find_library
	$(if $(findstring $(1),"vulkan"),$(if $(strip $(LD_LIBRARY_PATH)),FOUND_LIBS+= -L$(LD_LIBRARY_PATH); CFLAGS+=$$(VULKAN_SDK)/include,),$(if $(shell ldconfig -p | grep libvulkan),FOUND_LIBS+= -l$(1),$(error "Failed to find $(1)")))
endef

define find_pkg 
	$(if $(filter 0,$(shell pkg-config --exists $(1); echo $$?)),CFLAGS+=$(shell pkg-config --cflags $(1)),$\
		$(call find_library,$(1)))
endef

###############################################################################
## Figure out the directory and dependency structure of the project build.
###############################################################################

BUILD:=$(abspath build)
PREFIX=/usr
SOURCE:=$(abspath src)
INCLUDE:=$(abspath include)

PUBLIC_DIR=$(PREFIX)/include/waterlily
LIB_DIR=$(PREFIX)/lib
CONFIG_DIR=$(PREFIX)/share/pkgconfig
DEPENDENCIES=vulkan wayland-client xkbcommon

###############################################################################
## Set up the flags we're going to use in all compilations.
###############################################################################

CFLAGS:=-std=gnu2x -Wall -Wextra -Wpedantic -Werror -I$(INCLUDE)
CFLAGS+=$(if $(strip $(DEBUG)),-Og -g3 -ggdb -fanalyzer -fsanitize=leak $\
			-fsanitize=address -fsanitize=pointer-compare $\
			-fsanitize=pointer-subtract -fsanitize=undefined,-march=native $\
			-mtune=native -Ofast -flto)

ARFLAGS:=rcs

###############################################################################
## Define the project's output files.
###############################################################################

OBJECTS:=engine.o files.o input.o public.o vulkan.o window.o
OUTPUTS:=$(foreach obj, $(OBJECTS), $(addprefix $(BUILD)/, $(obj)))

LIBRARY_NAME:=libwaterlily.a
CONFIG_NAME:=waterlily.pc
PUBLIC_NAME:=waterlily.h
INTERNAL_NAME:=internal.h

LIBRARY:=$(BUILD)/$(LIBRARY_NAME)
CONFIG:=$(BUILD)/$(CONFIG_NAME)
PUBLIC:=$(INCLUDE)/$(PUBLIC_NAME)
INTERNAL:=$(SOURCE)/$(INTERNAL_NAME)

###############################################################################
## Define the project's setup tasks.
###############################################################################

EXPORT_COMMAND:=grep -wE '$(CC)'$\
	| grep -w '\-c'$\
	| jq -nR '[inputs|{directory:"$(BUILD)",$\
		command:.,$\
		file:match(" [^ ]+$$").string[1:],$\
		output:"$(BUILD)"+match(" [^ ]+$$").$\
			string[1+("$(SOURCE)"|length):-2]+".o"}]'$\
	> $(BUILD)/compile_commands.json

all: $(LIBRARY) | $(if $(strip $(EXPORT_COMMAND_RUN)),,export_commands) 

prep:
	$(foreach dep, $(DEPENDENCIES), $(eval $(call find_pkg,$(dep))))

clean:
	$(RM) -rf $(BUILD)

export_commands: | $(BUILD) 
	$(MAKE) EXPORT_COMMAND_RUN=on --always-make --dry-run | $(EXPORT_COMMAND) 

###############################################################################
## Define the project's build tasks.
###############################################################################

$(LIBRARY): $(OUTPUTS) $(PUBLIC) 
	$(AR) $(ARFLAGS) $(LIBRARY) $(OUTPUTS)

$(BUILD)/%.o: $(SOURCE)/%.c $(INTERNAL) | $(BUILD) prep
	$(CC) -c $(CFLAGS) -DFILENAME=\"$(notdir $<)\" -o $@ $<

$(BUILD):
	mkdir -p $(BUILD)

###############################################################################
## Define the project's installation tasks.
###############################################################################

install: $(LIBRARY) $(CONFIG) 
	install -m 644 -D $(LIBRARY) $(DESTDIR)$(LIB_DIR)/$(LIBRARY_NAME)
	install -m 644 -D $(PUBLIC) $(DESTDIR)$(PUBLIC_DIR)/$(PUBLIC_NAME)
	install -m 644 -D $(CONFIG) $(DESTDIR)$(CONFIG_DIR)/$(CONFIG_NAME)

$(CONFIG): | $(BUILD)
	$(file > $(CONFIG))
	$(file >> $(CONFIG),Name: Waterlily)
	$(file >> $(CONFIG),Description: A C library for creating RPG games.)
	$(file >> $(CONFIG),URL: https://github.com/israfiel-a/waterlily.git)
	$(file >> $(CONFIG),Version: 1.0.0)
	$(file >> $(CONFIG),Requires: $(DEPENDENCIES))
	$(file >> $(CONFIG),Cflags: -I$(PUBLIC_DIR))
	$(file >> $(CONFIG),Libs: -L$(LIB_DIR) -lwaterlily$(FOUND_LIBS))

