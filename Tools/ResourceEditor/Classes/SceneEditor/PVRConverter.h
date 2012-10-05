#ifndef __PVR_CONVERTER_H__
#define __PVR_CONVERTER_H__

#include "DAVAEngine.h"

class PVRConverter: public DAVA::Singleton<PVRConverter>
{    
public:
 
	PVRConverter();
	virtual ~PVRConverter();

    DAVA::String ConvertPngToPvr(const DAVA::String & fileToConvert, DAVA::int32 format, bool generateMimpaps);

protected:

	DAVA::String dataFolderPath;
};



#endif // __PVR_CONVERTER_H__