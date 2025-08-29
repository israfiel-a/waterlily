################################################################################
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
################################################################################

.PHONY: all clean debug release 

################################################################################
## Figure out the source structure of the project.
################################################################################

SOURCE_DIRECTORY_NAME:=src
INCLUDE_DIRECTORY_NAME:=include
INTERNAL_DIRECTORY_NAME:=internal
ARCHIVER_DIRECTORY_NAME:=archiver

PUBLIC_LIBRARY_INTERFACE_NAME:=waterlily
ARCHIVER_EXECUTABLE_ENTRY_NAME:=archiver
PUBLIC_LIBRARY_SOURCE_NAMES:=config files input logging vulkan window
ARCHIVER_EXECUTABLE_SOURCE_NAMES:=compressor parser shaders logging files

SOURCE_DIRECTORY:=$(abspath $(SOURCE_DIRECTORY_NAME))
INTERNAL_SOURCE_DIRECTORY:=$(SOURCE_DIRECTORY)/$(INTERNAL_DIRECTORY_NAME)
ARCHIVER_SOURCE_DIRECTORY:=$(SOURCE_DIRECTORY)/$(ARCHIVER_DIRECTORY_NAME)

INCLUDE_DIRECTORY:=$(abspath $(INCLUDE_DIRECTORY_NAME))
# We don't actually give the compilation these variables for the sake of 
# include clarity, but we use them to construct the full source file paths.
INTERNAL_INCLUDE_DIRECTORY:=$(INCLUDE_DIRECTORY)/$(INTERNAL_DIRECTORY_NAME)
ARCHIVER_INCLUDE_DIRECTORY:=$(INCLUDE_DIRECTORY)/$(ARCHIVER_DIRECTORY_NAME)

PUBLIC_LIBRARY_SOURCES:=$(foreach source,$\
	$(PUBLIC_LIBRARY_SOURCE_NAMES),$\
	$(INTERNAL_SOURCE_DIRECTORY)/$(source).c$\
) $(SOURCE_DIRECTORY)/$(PUBLIC_LIBRARY_INTERFACE_NAME).c
ARCHIVER_EXECUTABLE_SOURCES:=$(foreach source,$\
	$(ARCHIVER_EXECUTABLE_SOURCE_NAMES),$\
	$(if $(wildcard $(ARCHIVER_SOURCE_DIRECTORY)/$(source).c),$\
		$(ARCHIVER_SOURCE_DIRECTORY)/$(source).c,$\
		$(INTERNAL_SOURCE_DIRECTORY)/$(source).c$\
	)$\
) $(SOURCE_DIRECTORY)/$(ARCHIVER_EXECUTABLE_ENTRY_NAME).c

INTERNAL_INTERFACES:=$(foreach interface,$\
	$(PUBLIC_LIBRARY_SOURCE_NAMES),$\
	$(INTERNAL_INCLUDE_DIRECTORY)/$(interface).h$\
) $(INCLUDE_DIRECTORY)/$(PUBLIC_LIBRARY_INTERFACE_NAME).h
ARCHIVER_INTERFACES:=$(foreach interface,$\
	$(ARCHIVER_EXECUTABLE_SOURCE_NAMES),$\
	$(if $(wildcard $(ARCHIVER_SOURCE_DIRECTORY)/$(interface).c),$\
		$(ARCHIVER_INCLUDE_DIRECTORY)/$(interface).h,$\
		$(INTERNAL_INCLUDE_DIRECTORY)/$(interface).h$\
	)$\
)

################################################################################
## Figure out the build structure of the project.
################################################################################

BUILD_DIRECTORY_NAME:=build

PUBLIC_LIBRARY_NAME:=libwaterlily.a
ARCHIVER_EXECUTABLE_NAME:=waterlilyarchiver

BUILD_DIRECTORY:=$(abspath $(BUILD_DIRECTORY_NAME))
INTERNAL_BUILD_DIRECTORY:=$(BUILD_DIRECTORY)/$(INTERNAL_DIRECTORY_NAME)
ARCHIVER_BUILD_DIRECTORY:=$(BUILD_DIRECTORY)/$(ARCHIVER_DIRECTORY_NAME)

PUBLIC_LIBRARY_OUTPUTS:=$(foreach source,$\
	$(PUBLIC_LIBRARY_SOURCE_NAMES),$\
	$(INTERNAL_BUILD_DIRECTORY)/$(source).o$\
) $(BUILD_DIRECTORY)/$(PUBLIC_LIBRARY_INTERFACE_NAME).o
ARCHIVER_EXECUTABLE_OUTPUTS:=$(foreach source,$\
	$(ARCHIVER_EXECUTABLE_SOURCE_NAMES),$\
	$(if $(wildcard $(ARCHIVER_SOURCE_DIRECTORY)/$(source).c),$\
		$(ARCHIVER_BUILD_DIRECTORY)/$(source).o,$\
		$(INTERNAL_BUILD_DIRECTORY)/$(source).o$\
	)$\
) $(BUILD_DIRECTORY)/$(ARCHIVER_EXECUTABLE_ENTRY_NAME).o

PUBLIC_LIBRARY:=$(BUILD_DIRECTORY)/$(PUBLIC_LIBRARY_NAME)
ARCHIVER_EXECUTABLE:=$(BUILD_DIRECTORY)/$(ARCHIVER_EXECUTABLE_NAME)

COMPILEDB:=$(BUILD_DIRECTORY)/compile_commands.json

################################################################################
## Get together the proper flags to compile.
################################################################################

define setup_vulkan_sdk
	-L$(LD_LIBRARY_PATH-=) -l$(1) -I$(VULKAN_SDK)/include
endef

define find_library
	$(if $(findstring $(1),"vulkan"),$\
		$(if $(strip $(LD_LIBRARY_PATH)),$\
			$(call setup_vulkan_sdk,$(1)),),$\
		$(if $(shell ldconfig -p | grep libvulkan),$\
			-l$(1),$\
			$(error "Failed to find $(1)")))
endef

define find_dependencies 
	$(foreach dep,$($(1)_DEPENDENCIES),$\
		$(if $(filter 0,$(shell pkg-config --exists $(dep); echo $$?)),$\
			$(shell pkg-config --cflags --libs $(dep)),$\
			$(call find_library,$(dep))$\
		)$\
	)
endef

CFLAGS:=-std=gnu2x -Wall -Wextra -Wpedantic -Werror -I$(INCLUDE_DIRECTORY)

PUBLIC_LIBRARY_DEPENDENCIES:=vulkan xkbcommon wayland-client
ARCHIVER_EXECUTABLE_DEPENDENCIES:=glslang

PUBLIC_LIBRARY_DEPENDENCY_FLAGS:=$(strip $(call find_dependencies,PUBLIC_LIBRARY))
ARCHIVER_EXECUTABLE_DEPENDENCY_FLAGS:=$(strip $(call find_dependencies,ARCHIVER_EXECUTABLE))

################################################################################
## Define the project's build recipes.
################################################################################

define create_library
	$(AR) -qcs $($(1)) $($(1)_OUTPUTS) -l $\
		"$($(1)_DEPENDENCY_FLAGS) $(strip $(LDFLAGS))"
endef

define create_executable
	$(CC) $($(1)_OUTPUTS) -o $($(1)) $(CFLAGS) $($(1)_DEPENDENCY_FLAGS)
endef

define compile_file
	$(CC) -c -DFILENAME=\"$(notdir $<)\" $(CFLAGS) -o $@ $< $($(1)_DEPENDENCY_FLAGS) 
endef

define find_mode
	$(if $(wildcard $(BUILD_DIRECTORY)/$(1).mode),clean,) 
endef

all: $(BUILD_DIRECTORY) $(PUBLIC_LIBRARY) $(ARCHIVER_EXECUTABLE) 

clean:
	rm -rf $(BUILD_DIRECTORY)

debug: CFLAGS+=-Og -g3 -ggdb -fanalyzer -fsanitize=address,leak,undefined $\
	-fsanitize=pointer-compare,pointer-subtract 
debug: LDFLAGS+=-fsanitize=leak,address,undefined
debug: $(call find_mode,release) all $(COMPILEDB) $(BUILD_DIRECTORY)/debug.mode

release: CFLAGS+=-march=native -mtune=native -Ofast -flto
release: $(call find_mode,debug) all $(BUILD_DIRECTORY)/release.mode

$(BUILD_DIRECTORY)/debug.mode:
	touch $(BUILD_DIRECTORY)/debug.mode

$(BUILD_DIRECTORY)/release.mode:
	touch $(BUILD_DIRECTORY)/release.mode

$(COMPILEDB):
	$(if $(strip $(GENERATING)),,$\
		$(if $(shell command -v compiledb),$\
			compiledb -n -o $(COMPILEDB) $(MAKE) debug GENERATING=on,$\
		)$\
	)

$(PUBLIC_LIBRARY): $(PUBLIC_LIBRARY_OUTPUTS) $(INTERNAL_INTERFACES)
	$(call create_library,PUBLIC_LIBRARY)

$(ARCHIVER_EXECUTABLE): $(ARCHIVER_EXECUTABLE_OUTPUTS) $(ARCHIVER_INTERFACES)
	$(call create_executable,ARCHIVER_EXECUTABLE)

$(BUILD_DIRECTORY)/%.o: $(SOURCE_DIRECTORY)/%.c 
	$(call compile_file,PUBLIC_LIBRARY)

$(INTERNAL_BUILD_DIRECTORY)/%.o: $(INTERNAL_SOURCE_DIRECTORY)/%.c
	$(call compile_file,PUBLIC_LIBRARY)

$(ARCHIVER_BUILD_DIRECTORY)/%.o: $(ARCHIVER_SOURCE_DIRECTORY)/%.c
	$(call compile_file,ARCHIVER_EXECUTABLE)

$(BUILD_DIRECTORY):
	mkdir -p $(INTERNAL_BUILD_DIRECTORY) $(ARCHIVER_BUILD_DIRECTORY)

