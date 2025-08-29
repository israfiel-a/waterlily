#include <archiver/compressor.h>
#include <internal/files.h>

void waterlily_flattenAssets(void)
{
    waterlily_file_t file = {
        .name = "assets",
        .type = WATERLILY_ARCHIVE_FILE,
    };

    waterlily_writeFile(&file, false);
}

void waterlily_compressAssets(void) {}

