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


#include "Base/EventDispatcher.h"
#include <iterator>

namespace DAVA 
{

EventDispatcher::EventDispatcher()
{
}

EventDispatcher::~EventDispatcher()
{
}

void EventDispatcher::AddEvent(int32 eventType, const Message &msg)
{
	Event ev;
	ev.msg = msg;
	ev.eventType = eventType;
	events.push_back(ev);
}
	
bool EventDispatcher::RemoveEvent(int32 eventType, const Message &msg)
{
	List<Event>::iterator it = events.begin();
	for(; it != events.end(); it++)
	{
		//if(Event(*it).msg == msg && Event(*it).eventType == eventType)
		if((it->msg == msg) && (it->eventType == eventType))
		{
			events.erase(it);
			return true;
		}
	}
	return false;
}
	
bool EventDispatcher::RemoveAllEvents()
{
	int32 size = (int32)events.size();
	events.clear();
	return (size != 0);
}

void EventDispatcher::PerformEvent(int32 eventType)
{
	PerformEventWithData(eventType, this, NULL);
}

void EventDispatcher::PerformEvent(int32 eventType, BaseObject *eventParam)
{
    PerformEventWithData(eventType, eventParam, NULL);
}

void EventDispatcher::PerformEventWithData(int32 eventType, void *callerData)
{
	PerformEventWithData(eventType, this, callerData);
}
    
template< class T >
T* AddressOf(T& arg)
{
    return reinterpret_cast<T*>(
                &const_cast<char&>(
                    reinterpret_cast<const volatile char&>(arg)));
}
	
void EventDispatcher::PerformEventWithData(int32 eventType, BaseObject *eventParam, void *callerData)
{
    if(events.empty())
        return;

    eventsCopy.clear();
    eventsCopy.reserve(events.size());
    std::transform(events.begin(), events.end(), std::back_inserter(eventsCopy), DAVA::AddressOf<Event>);

    Vector<Event *>::const_iterator it = eventsCopy.begin();
    Vector<Event *>::const_iterator end = eventsCopy.end();
	for(; it != end; it++)
	{
		if((*it)->eventType == eventType)
		{
			(*it)->msg(eventParam, callerData);
		}
	}
}
	
void EventDispatcher::CopyDataFrom(EventDispatcher *srcDispatcher)
{
	events.clear();
	List<Event>::iterator it = srcDispatcher->events.begin();
	for(; it != srcDispatcher->events.end(); it++)
	{
		events.push_back(*it);
	}
}

int32 EventDispatcher::GetEventsCount() const
{
    return events.size();
}

}