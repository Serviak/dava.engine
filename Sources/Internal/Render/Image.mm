/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#import "Image.h"

#ifdef __DAVAENGINE_IPHONE__

#import <Foundation/Foundation.h>
#import <AssetsLibrary/AssetsLibrary.h>
#import <UIKit/UIKit.h>

#import "Render/Texture.h"
#import "Render/PixelFormatDescriptor.h"

namespace DAVA
{
    
    
void Image::SaveToSystemPhotos(SaveToSystemPhotoCallbackReceiver* callback)
{
    DVASSERT(format == FORMAT_RGBA8888);

    size_t bitsPerComponent = 8;
    size_t bitsPerPixel = PixelFormatDescriptor::GetPixelFormatSizeInBits(format);
    size_t bytesPerPixel = PixelFormatDescriptor::GetPixelFormatSizeInBytes(format);
    size_t bytesPerRow = width * bytesPerPixel;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, data, width * height * bytesPerPixel, NULL);
    
    CGImageRef imageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);

    UIImage* image = [UIImage imageWithCGImage:imageRef];
    
    ALAssetsLibrary *library = [[ALAssetsLibrary alloc] init];

    if(callback != 0)
    {
        [library writeImageToSavedPhotosAlbum:[image CGImage] orientation:(ALAssetOrientation)[image imageOrientation] completionBlock:^(NSURL *assetURL, NSError *error)
         {
             callback->SaveToSystemPhotosFinished();
        }];
    }
    else
    {
        [library writeImageToSavedPhotosAlbum:[image CGImage] orientation:(ALAssetOrientation)[image imageOrientation] completionBlock:NULL];
    }

    [library release];
}

bool Image::Save(const FilePath &path) const
{
    String ext = path.GetExtension();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    size_t bitsPerComponent = 8;
    size_t bitsPerPixel = PixelFormatDescriptor::GetPixelFormatSizeInBits(format);
    size_t bytesPerPixel = PixelFormatDescriptor::GetPixelFormatSizeInBytes(format);
    size_t bytesPerRow = width * bytesPerPixel;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, data, width * height * bytesPerPixel, NULL);
    CGImageRef imageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    
    UIImage* image = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);

    NSData* _data = nil;
    
    if (ext.compare(".png") == 0)
    {
        _data = UIImagePNGRepresentation(image);
    }
    else if (ext.compare(".jpeg") == 0 || ext.compare(".jpg") == 0)
    {
        _data = UIImageJPEGRepresentation(image, 1.0f);
    }
    
    if (_data != nil)
    {
        NSString *_path = [NSString stringWithCString:path.GetAbsolutePathname().c_str() encoding:NSASCIIStringEncoding];
        return ([_data writeToFile:_path atomically:YES] == YES) ? true : false;
    }

    return false;
}
    
}

#endif //__DAVAENGINE_IPHONE_

