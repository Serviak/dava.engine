/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#include "Base/BaseTypes.h"

#ifdef __DAVAENGINE_IPHONE__
#import "UIKit/UIKit.h"
#include "Input/AccelerometeriPhone.h"
#include "FileSystem/Logger.h"

@interface UIAccelerometerEventCatcher : NSObject<UIAccelerometerDelegate>
{
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration;

@end


UIAccelerometerEventCatcher *realCatcher;

namespace DAVA 
{
AccelerometeriPhoneImpl::AccelerometeriPhoneImpl()
{
	[UIAccelerometer sharedAccelerometer].delegate = nil;
	realCatcher = [[UIAccelerometerEventCatcher alloc] init];
	Logger::Debug("Accelerometer iPhone");
};
	
AccelerometeriPhoneImpl::~AccelerometeriPhoneImpl()
{
	Logger::Debug("Accelerometer iPhone destructor");
};

void AccelerometeriPhoneImpl::Enable(float32 updateRate)
{
	[UIAccelerometer sharedAccelerometer].delegate = realCatcher;
	[UIAccelerometer sharedAccelerometer].updateInterval = updateRate;
}

void AccelerometeriPhoneImpl::Disable()
{
	[UIAccelerometer sharedAccelerometer].delegate = nil;
}

bool AccelerometeriPhoneImpl::IsEnabled() const
{
    return nil != [UIAccelerometer sharedAccelerometer].delegate;
}

void AccelerometeriPhoneImpl::SetAccelerationData(float x, float y, float z)
{
	accelerationData.x = x;
	accelerationData.y = y;
	accelerationData.z = z;
}

EventDispatcher * AccelerometeriPhoneImpl::GetEventDispatcher()
{
	return &eventDispatcher;
}
};


@implementation UIAccelerometerEventCatcher

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
	DAVA::AccelerometeriPhoneImpl * impl = (DAVA::AccelerometeriPhoneImpl *)DAVA::Accelerometer::Instance();
	impl->SetAccelerationData((float)acceleration.x, (float)acceleration.y, (float)acceleration.z);
	impl->GetEventDispatcher()->PerformEvent(DAVA::Accelerometer::EVENT_ACCELLEROMETER_DATA);
}

@end
#endif