![top_banner](./.github/banner.jpg)

----------

### Waterlily
![lines of code badge](https://img.shields.io/endpoint?url=https://ghloc.vercel.app/api/israfiel-a/waterlily/badge?format=human&style=flat&color=blue&label=Lines%20of%20Code)
![build badge](https://img.shields.io/github/actions/workflow/status/israfiel-a/waterlily/build.yml?label=Build%20Test)

Waterlily is a game engine/toolkit built for pixelart, tile-based RPG games. It's built to be both very performant and extremely simple to use and utilize as little internal state as possible. It relies on no wrapper libraries, instead preferring to go straight for the backends.

----------

#### Dependencies
Waterlily is built to depend on little but what comes with your operating system installation. All it relies on are the following;

- [Vulkan](https://www.vulkan.org/): The Vulkan library is a high-performance and low-abstraction rendering library built directly over baremetal GPU-CPU communications.
- [Xkbcommon](https://xkbcommon.org/): Xkbcommon is a library for handling keyboard input, and it is how windowing systems communicate with the application.
- [Wayland](https://wayland.freedesktop.org/): The Wayland protocol is a set of specifications that describe a modern windowing system. 

----------

#### Versioning
Waterlily follows semantic versioning. The version number is detailed in the [CMakeLists.txt](./CMakeLists.txt) file, within the initial "[project](https://cmake.org/cmake/help/latest/command/project.html)" command.

----------

![bottom_banner](./.github/banner.jpg)
