/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include <libpng/png.h>
#include "Render/RenderManager.h"
#include "Render/2D/Sprite.h"
#include "Render/Texture.h"
#include "FileSystem/FileSystem.h"
#include "Render/Image/ImageConvert.h"
#include "Render/Image/LibPngHelpers.h"
#include "Render/PixelFormatDescriptor.h"

#if !defined(__DAVAENGINE_WIN32__)
#include <unistd.h>
#endif //#if !defined(__DAVAENGINE_WIN32__)

using namespace DAVA;

#define PNG_DEBUG 3

namespace
{
    struct	PngImageRawData
    {
        File *file;
    };

    void PngImageRead(png_structp pngPtr, png_bytep data, png_size_t size)
    {
        PngImageRawData * self = static_cast<PngImageRawData*>(png_get_io_ptr(pngPtr));
        self->file->Read(data, static_cast<uint32>(size));
    }

    void ReleaseWriteData(png_struct *& png_ptr, png_info *& info_ptr, unsigned char**& row_pointers, FILE *& fp, Image*& convertedImage)
    {
        free(row_pointers);
        row_pointers = nullptr;
        png_destroy_write_struct(&png_ptr, &info_ptr);
        png_ptr = nullptr;
        info_ptr = nullptr;
        if (fp != nullptr)
        {
            fclose(fp);
            fp = nullptr;
        }
        SafeRelease(convertedImage);
    }
}


void abort_(const char * s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}


LibPngWrapper::LibPngWrapper()
{
    supportedExtensions.push_back(".png");
}

bool LibPngWrapper::IsImage(File *file) const
{
    if (nullptr == file)
        return false;

    unsigned char sig[8];
    file->Read(sig, 8);
	bool retValue = (0 != png_check_sig(sig, 8));
    file->Seek(0,  File::SEEK_FROM_START);
    return retValue;
}

eErrorCode LibPngWrapper::ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap ) const
{
    Image* image = new Image();
    int innerRetCode = ReadPngFile(infile, image);

    if(innerRetCode == 1)
    {
        imageSet.push_back(image);
        return SUCCESS;
    }
    else
    {
        SafeRelease(image);
        return innerRetCode == 0 ? ERROR_FILE_FORMAT_INCORRECT : ERROR_READ_FAIL;
    }
}

eErrorCode LibPngWrapper::WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat format) const
{
    //	printf("* Writing PNG file (%d x %d): %s\n", width, height, file_name);
    DVASSERT(imageSet.size());
    int32 width = imageSet[0]->width;
    int32 height = imageSet[0]->height;
    uint8* imageData = imageSet[0]->data;
    Image* convertedImage = nullptr;
    if (FORMAT_RGB888 == imageSet[0]->format)
    {
        convertedImage = Image::Create(width, height, FORMAT_RGBA8888);
        ConvertDirect<RGB888, uint32, ConvertRGB888toRGBA8888> convert;
        convert(imageData, width, height, sizeof(RGB888)*width, convertedImage->data, width, height, sizeof(uint32)*width);
        imageData = convertedImage->data;
    }

    png_color_8 sig_bit;

    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;

    png_byte color_type = PNG_COLOR_TYPE_RGBA;
    png_byte bit_depth = 8;

    int32 bytes_for_color = 4;
    if (FORMAT_A8 == format)
    {
        color_type = PNG_COLOR_TYPE_GRAY;
        bytes_for_color = 1;
    }
    else if (FORMAT_A16 == format)
    {
        bit_depth = 16;
        color_type = PNG_COLOR_TYPE_GRAY;
        bytes_for_color = 2;
    }

    png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

    for (int y = 0; y < height; y++)
    {
        //		row_pointers[y] = (png_byte*) &data[y * width * 4];
        row_pointers[y] = (png_byte*)&imageData[y * width * bytes_for_color];
    }

    //create file
    FILE *fp = fopen(fileName.GetAbsolutePathname().c_str(), "wb");
    if (nullptr == fp)
    {
        Logger::Error("[LibPngWrapper::WritePngFile] File %s could not be opened for writing", fileName.GetAbsolutePathname().c_str());
        ReleaseWriteData(png_ptr, info_ptr, row_pointers, fp, convertedImage);
        return ERROR_FILE_NOTFOUND;
    }

    // initialize stuff
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (nullptr == png_ptr)
    {
        Logger::Error("[LibPngWrapper::WritePngFile] png_create_write_struct failed");
        ReleaseWriteData(png_ptr, info_ptr, row_pointers, fp, convertedImage);
        return ERROR_WRITE_FAIL;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (nullptr == info_ptr)
    {
        Logger::Error("[LibPngWrapper::WritePngFile] png_create_info_struct failed");
        ReleaseWriteData(png_ptr, info_ptr, row_pointers, fp, convertedImage);
        return ERROR_WRITE_FAIL;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        Logger::Error("[LibPngWrapper::WritePngFile] Error during init_io");
        ReleaseWriteData(png_ptr, info_ptr, row_pointers, fp, convertedImage);
        return ERROR_WRITE_FAIL;
    }

    png_init_io(png_ptr, fp);

    // write header
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        Logger::Error("[LibPngWrapper::WritePngFile] Error during writing header");
        ReleaseWriteData(png_ptr, info_ptr, row_pointers, fp, convertedImage);
        return ERROR_WRITE_FAIL;
    }

    png_set_IHDR(png_ptr, info_ptr, width, height,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    if (FORMAT_A8 == format)
    {
        sig_bit.red = 0;
        sig_bit.green = 0;
        sig_bit.blue = 0;

        sig_bit.gray = 8;
        sig_bit.alpha = 0;
    }
    else if (FORMAT_A16 == format)
    {
        sig_bit.red = 0;
        sig_bit.green = 0;
        sig_bit.blue = 0;

        sig_bit.gray = 16;
        sig_bit.alpha = 0;
    }
    else
    {
        sig_bit.red = 8;
        sig_bit.green = 8;
        sig_bit.blue = 8;
        sig_bit.alpha = 8;
    }

    png_set_sBIT(png_ptr, info_ptr, &sig_bit);

    png_write_info(png_ptr, info_ptr);
    png_set_shift(png_ptr, &sig_bit);
    png_set_packing(png_ptr);

    // write bytes
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        Logger::Error("[LibPngWrapper::WritePngFile] Error during writing bytes");
        ReleaseWriteData(png_ptr, info_ptr, row_pointers, fp, convertedImage);
        return ERROR_WRITE_FAIL;
    }

    png_write_image(png_ptr, row_pointers);

    // end write
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        Logger::Error("[LibPngWrapper::WritePngFile] Error during end of write");
        ReleaseWriteData(png_ptr, info_ptr, row_pointers, fp, convertedImage);
        return ERROR_WRITE_FAIL;
    }

    png_write_end(png_ptr, nullptr);

    ReleaseWriteData(png_ptr, info_ptr, row_pointers, fp, convertedImage);
    return SUCCESS;
}

eErrorCode LibPngWrapper::WriteFileAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat) const
{
    Logger::Error("[LibPngWrapper::WriteFileAsCubeMap] For png cubeMaps are not supported");
    return ERROR_WRITE_FAIL;
}

ImageInfo LibPngWrapper::GetImageInfo(File *infile) const
{
    if (nullptr == infile)
    {
        return ImageInfo();
    }

    char sig[8];
    infile->Read(sig, 8);
    if (!png_check_sig((unsigned char *)sig, 8))
    {
        return ImageInfo();
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (nullptr == png_ptr)
    {
        return ImageInfo();
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (nullptr == info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        return ImageInfo();
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return ImageInfo();
    }

    PngImageRawData	raw;
    raw.file = infile;
    png_set_read_fn(png_ptr, &raw, PngImageRead);

    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    int bit_depth;
    int color_type;

    png_uint_32 width;
    png_uint_32 height;

    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

    ImageInfo info;
    info.width = width;
    info.height = height;

    switch (color_type)
    {
        case PNG_COLOR_TYPE_RGB_ALPHA:
        {
            switch (bit_depth)
            {
                case 4: info.format = FORMAT_RGBA4444; break;
                case 8: info.format = FORMAT_RGBA8888; break;
                case 16: info.format = FORMAT_RGBA16161616; break;
                case 32: info.format = FORMAT_RGBA32323232; break;
                default: info.format = FORMAT_INVALID; break;
            }
            break;
        }
        case PNG_COLOR_TYPE_RGB:
        {
            switch (bit_depth)
            {
                case 5:
                case 6: info.format = FORMAT_RGB565; break;
                case 8: info.format = FORMAT_RGB888; break;
                default: info.format = FORMAT_INVALID; break;
            }
            break;
        }
        case PNG_COLOR_TYPE_GRAY:
        {
            switch (bit_depth)
            {
                case 8: info.format = FORMAT_A8; break;
                default: info.format = FORMAT_INVALID; break;
            }
            break;
        }
        case PNG_COLOR_TYPE_GRAY_ALPHA:
        {
            switch (bit_depth)
            {
                case 16: info.format = FORMAT_A16; break;
                default: info.format = FORMAT_INVALID; break;
            }
        }
        default: info.format = FORMAT_INVALID; break;
    }

    /* Clean up. */
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    return info;
}

int LibPngWrapper::ReadPngFile(File *infile, Image * image, PixelFormat targetFormat/* = FORMAT_INVALID*/)
{
    DVASSERT(targetFormat == FORMAT_INVALID || targetFormat == FORMAT_RGBA8888);

    png_structp png_ptr;
    png_infop info_ptr;

    char sig[8];

    int bit_depth;
    int color_type;

    png_uint_32 width;
    png_uint_32 height;
    unsigned int rowbytes;

    int i;
    png_bytepp row_pointers = nullptr;

    infile->Read(sig, 8);

    if (!png_check_sig((unsigned char *)sig, 8))
    {
        return 0;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (nullptr == png_ptr)
    {
        return 4;    /* out of memory */
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (nullptr == info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)nullptr, (png_infopp)nullptr);
        return 4;    /* out of memory */
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return 0;
    }

    PngImageRawData	raw;
    raw.file = infile;
    png_set_read_fn(png_ptr, &raw, PngImageRead);

    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
                 &color_type, nullptr, nullptr, nullptr);

    image->width = width;
    image->height = height;

    //1 bit images -> 8 bit
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    image->format = FORMAT_RGBA8888;
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        if (targetFormat == FORMAT_RGBA8888)
        {
            png_set_gray_to_rgb(png_ptr);
            png_set_filler(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
        }
        else
        {
            image->format = (bit_depth == 16) ? FORMAT_A16 : FORMAT_A8;
            if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            {
                png_set_strip_alpha(png_ptr);
            }
        }
    }
    else if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png_ptr);
        png_set_filler(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
    }
    else if (color_type == PNG_COLOR_TYPE_RGB)
    {
        png_set_filler(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
    }

    if (bit_depth > 8 && image->format != FORMAT_A16)
    {
        Logger::Error("Wrong image: must be 8bits on channel: %s", infile->GetFilename().GetAbsolutePathname().c_str());
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return 0;
    }


    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(png_ptr);
    }

    png_read_update_info(png_ptr, info_ptr);

    rowbytes = static_cast<uint32>(png_get_rowbytes(png_ptr, info_ptr));

    uint8 *image_data = new uint8[rowbytes * height];
    if (nullptr == image_data)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return 4;
    }

    row_pointers = (png_bytepp)malloc(height*sizeof(png_bytep));
    if (nullptr == row_pointers)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        delete[](image_data);
        image_data = nullptr;
        return 4;
    }


    /* set the individual row_pointers to point at the correct offsets */

    for (i = 0; i < static_cast<int>(height); ++i)
        row_pointers[i] = image_data + i * rowbytes;


    /* now we can go ahead and just read the whole image */
    png_read_image(png_ptr, row_pointers);

    /* and we're done!  (png_read_end() can be omitted if no processing of
    * post-IDAT text/time/etc. is desired) */

    /* Clean up. */
    free(row_pointers);

    /* Clean up. */
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    image->data = image_data;

    return 1;
}
