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

.PHONY: all clean 

###############################################################################
## Figure out the source structure of the project.
###############################################################################

SOURCE_DIRECTORY_NAME:=src
INCLUDE_DIRECTORY_NAME:=include
INTERNAL_DIRECTORY_NAME:=internal
ARCHIVER_DIRECTORY_NAME:=archiver

PUBLIC_LIBRARY_INTERFACE_NAME:=waterlily
ARCHIVER_EXECUTABLE_ENTRY_NAME:=archiver
PUBLIC_LIBRARY_SOURCE_NAMES:=config files input logging vulkan window
ARCHIVER_EXECUTABLE_SOURCE_NAMES:=compressor parser shaders logging

SOURCE_DIRECTORY:=$(abspath $(SOURCE_DIRECTORY_NAME))
INTERNAL_SOURCE_DIRECTORY:=$(SOURCE_DIRECTORY)/$(INTERNAL_DIRECTORY_NAME)
ARCHIVER_SOURCE_DIRECTORY:=$(SOURCE_DIRECTORY)/$(ARCHIVER_DIRECTORY_NAME)

INCLUDE_DIRECTORY:=$(abspath $(INCLUDE_DIRECTORY_NAME))
# We don't actually give the compilation these variables for the sake of include clarity, but we use them to construct the full source file paths.
INTERNAL_INCLUDE_DIRECTORY:=$(INCLUDE_DIRECTORY)/$(INTERNAL_DIRECTORY_NAME)
ARCHIVER_INCLUDE_DIRECTORY:=$(INCLUDE_DIRECTORY)/$(ARCHIVER_DIRECTORY_NAME)

PUBLIC_LIBRARY_SOURCES:=$(foreach source,$(PUBLIC_LIBRARY_SOURCE_NAMES),$(abspath $(INTERNAL_SOURCE_DIRECTORY)/$(source).c)) $(abspath $(SOURCE_DIRECTORY)/$(PUBLIC_LIBRARY_INTERFACE_NAME).c)
ARCHIVER_EXECUTABLE_SOURCES:=$(foreach source,$(ARCHIVER_EXECUTABLE_SOURCE_NAMES),$(if $(wildcard $(ARCHIVER_SOURCE_DIRECTORY)/$(source).c),$(ARCHIVER_SOURCE_DIRECTORY)/$(source).c),$(INTERNAL_SOURCE_DIRECTORY)/$(source).c) $(abspath $(SOURCE_DIRECTORY)/$(ARCHIVER_EXECUTABLE_ENTRY_NAME).c)

PUBLIC_LIBRARY_INTERFACE:=$(abspath $(INCLUDE_DIRECTORY)/$(PUBLIC_LIBRARY_INTERFACE_NAME).h)
INTERNAL_INTERFACES:=$(foreach interface,$(PUBLIC_LIBRARY_SOURCE_NAMES),$(abspath $(INTERNAL_INCLUDE_DIRECTORY)/$(interface).h))
ARCHIVER_INTERFACES:=$(foreach interface,$(ARCHIVER_EXECUTABLE_SOURCE_NAMES),$(if $(wildcard $(ARCHIVER_SOURCE_DIRECTORY)/$(interface).c),$(abspath $(ARCHIVER_INCLUDE_DIRECTORY)/$(interface).h),$(INTERNAL_INCLUDE_DIRECTORY)/$(interface.h)))

###############################################################################
## Figure out the build structure of the project.
###############################################################################

BUILD_DIRECTORY_NAME:=build

PUBLIC_LIBRARY_NAME:=libwaterlily.a
ARCHIVER_EXECUTABLE_NAME:=waterlilyarchiver

BUILD_DIRECTORY:=$(abspath $(BUILD_DIRECTORY_NAME))
INTERNAL_BUILD_DIRECTORY:=$(BUILD_DIRECTORY)/$(INTERNAL_DIRECTORY_NAME)
ARCHIVER_BUILD_DIRECTORY:=$(BUILD_DIRECTORY)/$(ARCHIVER_DIRECTORY_NAME)

PUBLIC_LIBRARY_OUTPUTS:=$(foreach source,$(PUBLIC_LIBRARY_SOURCE_NAMES),$(abspath $(INTERNAL_BUILD_DIRECTORY)/$(source).o)) $(abspath $(BUILD_DIRECTORY)/$(PUBLIC_LIBRARY_INTERFACE_NAME).o)
ARCHIVER_EXECUTABLE_OUTPUTS:=$(foreach source,$(ARCHIVER_EXECUTABLE_SOURCE_NAMES),$(if $(wildcard $(ARCHIVER_SOURCE_DIRECTORY)/$(source).c),$(abspath $(ARCHIVER_BUILD_DIRECTORY)/$(source).o),$(INTERNAL_BUILD_DIRECTORY)/$(source).o)) $(abspath $(BUILD_DIRECTORY)/$(ARCHIVER_EXECUTABLE_ENTRY_NAME).o)

PUBLIC_LIBRARY:=$(BUILD_DIRECTORY)/$(PUBLIC_LIBRARY_NAME)
ARCHIVER_EXECUTABLE:=$(BUILD_DIRECTORY)/$(ARCHIVER_EXECUTABLE_NAME)

COMPILE_COMMANDS:=$(BUILD_DIRECTORY)/compile_commands.json

###############################################################################
## Get together the proper flags to compile.
###############################################################################

define find_dependency
$(if $(shell pkg-config --exists --print-errors $(1)),$(if $(findstring $(1),vulkan),$(error "Missing Vulkan SDK path."),$(if $(strip $(VULKAN_SDK)),-L$(LD_LAYER_PATH) -lvulkan -I$(VULKAN_SDK)/include)),$(error "Failed to find package.")),$(shell pkg-config --cflags --libs $(1)))
endef

CFLAGS:=-std=gnu2x -Wall -Wextra -Wpedantic -Werror -I$(INCLUDE_DIRECTORY)

define add_flags
	CFLAGS+=$(1)
endef

DEPENDENCIES:=vulkan xkbcommon wayland-client
# We strip the output in case the cflags poll turns up empty. Makes the
# compilation command prettier when echoed.
CFLAGS+=$(foreach dep,$(DEPENDENCIES),$(strip $(call find_dependency,$(dep))))

###############################################################################
## Define the project's build recipies.
###############################################################################

all: $(PUBLIC_LIBRARY) $(ARCHIVER_EXECUTABLE)

clean:
	rm -rf $(BUILD_DIRECTORY)

$(COMPILE_COMMANDS): $(PUBLIC_LIBRARY_OUTPUTS) $(ARCHIVER_EXECUTABLE_OUTPUTS) | $(BUILD_DIRECTORY)
	$(if $(shell command -v compiledb),compiledb -n -o $(COMPILE_COMMANDS) $(MAKE) debug GENERATING=on)

debug: CFLAGS+=-Og -g3 -ggdb -fanalyzer -fsanitize=leak -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined
debug: $(PUBLIC_LIBRARY) $(ARCHIVER_EXECUTABLE) $(if $(strip $(GENERATING)),,$(COMPILE_COMMANDS))

release: CFLAGS+=-march=native -mtune=native -Ofast -flto
release: all

$(PUBLIC_LIBRARY): $(PUBLIC_LIBRARY_OUTPUTS) $(INTERNAL_INTERFACES) $(PUBLIC_LIBRARY_INTERFACE) 
	$(AR) -qcs $(PUBLIC_LIBRARY) $(PUBLIC_LIBRARY_OUTPUTS) 

$(ARCHIVER_EXECUTABLE): $(ARCHIVER_EXECUTABLE_OUTPUTS) $(ARCHIVER_INTERFACES)
	$(CC) $(ARCHIVER_EXECUTABLE_OUTPUTS) -o $(ARCHIVER_EXECUTABLE) $(CFLAGS)

$(BUILD_DIRECTORY)/waterlily.o: $(SOURCE_DIRECTORY)/waterlily.c | $(BUILD_DIRECTORY)
	$(CC) -c -DFILENAME=\"$(notdir $<)\" $(CFLAGS) -o $@ $< 

$(BUILD_DIRECTORY)/archiver.o: $(SOURCE_DIRECTORY)/archiver.c | $(BUILD_DIRECTORY)
	$(CC) -c -DFILENAME=\"$(notdir $<)\" $(CFLAGS) -o $@ $< 

$(INTERNAL_BUILD_DIRECTORY)/%.o: $(INTERNAL_SOURCE_DIRECTORY)/%.c | $(BUILD_DIRECTORY)
	$(CC) -c -DFILENAME=\"$(notdir $<)\" $(CFLAGS) -o $@ $<

$(ARCHIVER_BUILD_DIRECTORY)/%.o: $(ARCHIVER_SOURCE_DIRECTORY)/%.c | $(BUILD_DIRECTORY)
	$(CC) -c -DFILENAME=\"$(notdir $<)\" $(CFLAGS) -o $@ $<

$(BUILD_DIRECTORY):
	mkdir -p $(INTERNAL_BUILD_DIRECTORY) $(ARCHIVER_BUILD_DIRECTORY)

