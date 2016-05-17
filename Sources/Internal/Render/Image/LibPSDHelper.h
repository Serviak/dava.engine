#ifndef __DAVAENGINE_PSD_HELPER_H__
#define __DAVAENGINE_PSD_HELPER_H__

#include "Render/Image/ImageFormatInterface.h"
#include "Render/Image/Image.h"

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class LibPSDHelper : public ImageFormatInterface
{
public:
    LibPSDHelper();

    ImageFormat GetImageFormat() const override;

    bool CanProcessFile(File* file) const override;

    eErrorCode ReadFile(File* infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const override;

    //only RGBA8888 or RGB888
    eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    //only RGBA8888 or RGB888
    eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    ImageInfo GetImageInfo(File* infile) const override;
};

inline ImageFormat LibPSDHelper::GetImageFormat() const
{
    return IMAGE_FORMAT_PSD;
}
};

#endif // __DAVAENGINE_PSD_HELPER_H__