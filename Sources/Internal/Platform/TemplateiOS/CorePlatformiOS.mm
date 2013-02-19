#include "Base/BaseTypes.h"
#include "Core/Core.h"


#if defined(__DAVAENGINE_IPHONE__)

#include <UIKit/UIKit.h>
#include <UIKit/UIDevice.h>

#define NO_DPI_INFO_FOUND   0
#define INFO_LIST_SIZE      5

namespace DAVA
{
    struct DeviceScreenInfo
    {
        uint32      minimumSideLength;
        uint32      screenDPI;
        String      deviceName;
        
        DeviceScreenInfo(uint32 _minimumSideLength, uint32 _screenDPI, String _deviceName)
        {
            minimumSideLength = _minimumSideLength;
            screenDPI         = _screenDPI;
            deviceName        = _deviceName;
        }
        
    };
    
    enum eIOS_DPI
    {
        IPHONE_3_IPAD_MINI      = 163,
        IPHONE_4_5              = 326,
        IPAD_1_2                = 132,
        IPAD_3_4                = 264
    };
    
        
    const static DeviceScreenInfo devicesInfoList[INFO_LIST_SIZE] =
    {
        DeviceScreenInfo(320, IPHONE_3_IPAD_MINI,  ""),
        DeviceScreenInfo(640, IPHONE_4_5,""),
        DeviceScreenInfo(768, IPAD_1_2,  ""),
        DeviceScreenInfo(768, IPHONE_3_IPAD_MINI, "mini"),
        DeviceScreenInfo(1536, IPAD_3_4, ""),

    };
    
    uint32 GetDPIInfoListByDimension(uint32 minDimension,  List<const DeviceScreenInfo*> & outputList)
    {
        outputList.clear();
        
        for (uint32 i = 0; i < INFO_LIST_SIZE; ++i)
        {
            if(devicesInfoList[i].minimumSideLength == minDimension)
            {
                outputList.push_back( &devicesInfoList[i]);
            }
        }
        
        return outputList.size();
    }
    
    uint32 DeterminateExactDPI(List<const DeviceScreenInfo*> &devList)
    {
        String name([[UIDevice currentDevice].name UTF8String]);
        uint32 retDPI = NO_DPI_INFO_FOUND;
        
        for (List<const DeviceScreenInfo*>::const_iterator it = devList.begin(); it != devList.end(); ++it)
        {
            if((*it)->deviceName == "" )
            {
                if(NO_DPI_INFO_FOUND == retDPI)
                {
                    retDPI = (*it)->screenDPI;// set default value
                }
            }
            else
            {
                if(String::npos !=  name.find((*it)->deviceName))
                {
                    retDPI = (*it)->screenDPI;
                    break;
                }
            }
        }
        return retDPI;
        
    }
    
    uint32 Core::GetScreenDPI()
    {
        //due to magnificent api of ios the only way of determination of dpi is hardcode
        CGRect screenRect = [[UIScreen mainScreen] bounds];
        CGFloat scale = [[UIScreen mainScreen] scale]; //support of retina
        CGFloat screenWidth = screenRect.size.width * scale;
        CGFloat screenHeight = screenRect.size.height * scale;
        
         //width and height could be swapped according orientation
        uint32 minRes = screenWidth > screenHeight ? screenHeight : screenWidth;

        List<const DeviceScreenInfo*> outputList;
        uint32 foundedDevices = GetDPIInfoListByDimension(minRes, outputList);
        
        if (foundedDevices == 0)
        {
            return NO_DPI_INFO_FOUND;
        }

        if(foundedDevices == 1)
        {
            return outputList.front()->screenDPI;
        }
        
        if (foundedDevices > 1)
        {
            return DeterminateExactDPI(outputList);
        }
        
        return NO_DPI_INFO_FOUND;
    }
    
}

#endif // #if defined(__DAVAENGINE_IPHONE__)
