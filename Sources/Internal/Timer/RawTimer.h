#ifndef __DAVAENGINE_RAW_TIMER_H__
#define __DAVAENGINE_RAW_TIMER_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
/*
 Raw Timer should be used when you need to get elapsed time in miliseconds from calling Start() to calling GetElapsed().
 It is not thread safe class.
*/

class RawTimer
{
public:
    /* 
       \brief Starts time calculation. Now GetElapsed() should return not 0
     */
    void Start();
    /*
     \brief Stops time calculation. GetElapsed() whould return 0.
     */
    void Stop();
    /*
     \brief Resumes stopped time calculation. It means that GetElapsed() will return time delta from calling Start().
     */
    void Resume();

    /*
     \brief Indicates if time calculation is started
     */
    bool IsStarted();
    /*
     \brief Returns time in ms elapsed from calling Start(). Returns 0 if timer is stopped.
     */
    uint64 GetElapsed();

private:
    uint64 timerStartTime;
    bool isStarted = false;
};
}
#endif //__DAVAENGINE_RAW_TIMER_H__
