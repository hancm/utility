#include <stdlib.h>
#include <png.h>
#include "MyLog.h"

typedef struct {
    unsigned char *imageData;
    size_t size;
    size_t offset;
} ImageSource;

static void pngReaderCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    ImageSource *isource = (ImageSource*)png_get_io_ptr(png_ptr);

    if(isource->offset + length <= isource->size)
    {
        memcpy(data, isource->imageData + isource->offset, length);
        isource->offset += length;
    }
    else
    {
        png_error(png_ptr,"pngReaderCallback failed");
    }
}

int ReadPNGFromBuffer(const unsigned char *imageData, size_t dataSize, int &width, int &height)
{
    png_structp png_ptr = NULL;
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
    if (!png_ptr)
    {
        LOG_ERROR("ReadPngFile: Failed to create png_ptr");
        return -1;
    }

    png_infop info_ptr = NULL;
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        LOG_ERROR("ReadPngFile: Failed to create info_ptr");
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        LOG_ERROR("ReadPngFile: Failed to read the PNG file");
        return -1;
    }

    ImageSource imgsource;
    imgsource.imageData = (unsigned char*)imageData;
    imgsource.size = dataSize;
    imgsource.offset = 0;
    //define our own callback function for I/O instead of reading from a file
    png_set_read_fn(png_ptr, &imgsource, pngReaderCallback );

    /* **************************************************
    * The low-level read interface in libpng (http://www.libpng.org/pub/png/libpng-1.2.5-manual.html)
    * **************************************************
    */
    png_read_info(png_ptr, info_ptr);
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    //clean up after the read, and free any memory allocated
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 0;
}