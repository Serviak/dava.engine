#include "Platform/DeviceInfo.h"

#ifdef __DAVAENGINE_IPHONE__

#include "Utils/StringFormat.h"

#import <UIKit/UIDevice.h>
#import <Foundation/NSLocale.h>
#import <sys/utsname.h>

namespace DAVA
{

String DeviceInfo::GetVersion()
{
	NSString* systemVersion = [[UIDevice currentDevice] systemVersion];
	return String([systemVersion UTF8String]);
}

String DeviceInfo::GetManufacturer()
{
	return "Apple inc.";
}

String DeviceInfo::GetModel()
{
	String model = "";

	if (GetPlatform() == PLATFORM_IOS_SIMULATOR)
	{
		model = [[[UIDevice currentDevice] model] UTF8String];
	}
	else
	{
		struct  utsname systemInfo;
		uname(&systemInfo);

		NSString* modelName = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];

		//General
		if ([modelName hasPrefix:@"iPhone"])
			model = [modelName UTF8String];
		if ([modelName hasPrefix:@"iPad"])
			model = [modelName UTF8String];
		if ([modelName hasPrefix:@"iPod"])
			model = [modelName UTF8String];
		if ([modelName hasPrefix:@"AppleTV"])
			model = [modelName UTF8String];

		// iPhone
		if ([modelName hasPrefix:@"iPhone1,1"])
			model = "iPhone 1G";
		if ([modelName hasPrefix:@"iPhone1,2"])
			model = "iPhone 3G";
		if ([modelName hasPrefix:@"iPhone2,1"])
			model = "iPhone 3GS";

		if ([modelName hasPrefix:@"iPhone3,1"])
			model = "iPhone 4 GSM";
		if ([modelName hasPrefix:@"iPhone3,3"])
			model = "iPhone 4 CDMA";

		if ([modelName hasPrefix:@"iPhone4,1"])
			model = "iPhone 4S";

		if ([modelName hasPrefix:@"iPhone5,1"])
			model = "iPhone 5 GSM LTE";
		if ([modelName hasPrefix:@"iPhone5,2"])
			model = "iPhone 5 CDMA LTE";

		// iPad
		if ([modelName hasPrefix:@"iPad1,1"])
			model = "iPad 1";

		if ([modelName hasPrefix:@"iPad2,1"])
			model = "iPad 2 WiFi";
		if ([modelName hasPrefix:@"iPad2,2"])
			model = "iPad 2 3G GSM";
		if ([modelName hasPrefix:@"iPad2,3"])
			model = "iPad 2 3G CDMA";
		if ([modelName hasPrefix:@"iPad2,4"])
			model = "iPad 2 WiFi";

		if ([modelName hasPrefix:@"iPad2,5"])
			model = "iPad Mini WiFi";
		if ([modelName hasPrefix:@"iPad2,6"])
			model = "iPad Mini GSM LTE";
		if ([modelName hasPrefix:@"iPad2,7"])
			model = "iPad Mini GSM CDMA LTE";

		if ([modelName hasPrefix:@"iPad3,1"])
			model = "iPad 3 WiFi";
		if ([modelName hasPrefix:@"iPad3,2"])
			model = "iPad 3 CDMA LTE";
		if ([modelName hasPrefix:@"iPad3,3"])
			model = "iPad 3 GSM LTE";

		if ([modelName hasPrefix:@"iPad3,4"])
			model = "iPad 4 WiFi";
		if ([modelName hasPrefix:@"iPad3,5"])
			model = "iPad 4 GSM LTE";
		if ([modelName hasPrefix:@"iPad3,6"])
			model = "iPad 4 CDMA LTE";

		// iPod
		if ([modelName hasPrefix:@"iPod1,1"])
			model = "iPod Touch";
		if ([modelName hasPrefix:@"iPod2,1"])
			model = "iPod Touch 2G";
		if ([modelName hasPrefix:@"iPod3,1"])
			model = "iPod Touch 3G";
		if ([modelName hasPrefix:@"iPod4,1"])
			model = "iPod Touch 4G";
		if ([modelName hasPrefix:@"iPod5,1"])
			model = "iPod Touch 5G";

		//AppleTV
		if ([modelName hasPrefix:@"AppleTV1,1"])
			model = "AppleTV";
		if ([modelName hasPrefix:@"AppleTV2,1"])
			model = "AppleTV 2G";
		if ([modelName hasPrefix:@"AppleTV3,1"])
			model = "AppleTV 3G early 2012";
		if ([modelName hasPrefix:@"AppleTV3,2"])
			model = "AppleTV 3G early 2013";
		
		if (model.empty())
		{
			// Unknown at this moment, return what is returned by system.
			model = [modelName UTF8String];
		}
	}

	return model;
}

String DeviceInfo::GetLocale()
{
	NSLocale *english = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];

	NSString* langID = [[NSLocale preferredLanguages] objectAtIndex:0];
	NSString *lang = [english displayNameForKey:NSLocaleLanguageCode value:langID];

	String res = Format("%s (%s)", [langID UTF8String], [lang UTF8String]);
	return res;
}

String DeviceInfo::GetRegion()
{
	NSLocale *english = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];

	NSString *countryCode = [[NSLocale currentLocale] objectForKey: NSLocaleCountryCode];
	NSString *country = [english displayNameForKey: NSLocaleCountryCode value: countryCode];

	String res = Format("%s (%s)", [countryCode UTF8String], [country UTF8String]);
	return res;
}

String DeviceInfo::GetTimeZone()
{
	NSTimeZone *localTime = [NSTimeZone systemTimeZone];
	return [[localTime name] UTF8String];
}

}

#endif