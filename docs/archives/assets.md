![top_banner](../../.github/banner.jpg)

----------

### Asset Archive
The asset archive is one of the most central archive types to any Waterlily game. It includes the compressed textures, the compiled shaders, color/theme information, and more. While separate from other important things like the [dialogue archive](./dialogue.md) and the [configuration file](../config.md), the game could not run without a properly formatted asset archive. The archive is, for the most part, a binary format. This document shows a detailed specification for said format, alongside any quirks that may pop up while trying to contruct your own.

----------

#### Specification
The binary format is fairly simple. It contains a block of data formatted into sections, each except the first beginning with the hexidecimal number `0xA7` (§). The sections are detailed below.

Shaders:
    Section ID (byte 0): `0x0`
    Shaders (rest of section):
        Shader Stage ID (byte 0):
            Vertex: `0x0`
            Fragment: `0x1`
        Shader Length (bytes 1, 2)

----------

![top_banner](../../.github/banner.jpg)
