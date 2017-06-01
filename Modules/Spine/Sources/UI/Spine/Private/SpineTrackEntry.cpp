#include "UI/Spine/Private/SpineTrackEntry.h"

#include <Debug/DVAssert.h>

#include <spine/spine.h>

namespace DAVA
{
SpineTrackEntry::SpineTrackEntry(spTrackEntry* track)
    : trackPtr(track)
{
    DVASSERT(trackPtr);
}

bool SpineTrackEntry::IsLoop() const
{
    if (trackPtr)
    {
        return trackPtr->loop != 0;
    }
    return false;
}
}