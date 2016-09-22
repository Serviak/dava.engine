#pragma once

#include "Base/BaseTypes.h"
#include <iosfwd>

namespace DAVA
{
struct TraceEvent
{
    enum EventPhase
    {
        PHASE_BEGIN = 0,
        PHASE_END,
        PHASE_INSTANCE,
        PHASE_DURATION,

        PHASE_COUNT
    };

    TraceEvent(const FastName& _name, uint32 _processID, uint64 _threadID, uint64 _timestamp, uint64 _duration, EventPhase _phase)
        : name(_name)
        , timestamp(_timestamp)
        , duration(_duration)
        , threadID(_threadID)
        , processID(_processID)
        , phase(_phase)
    {
    }

    FastName name;
    uint64 timestamp;
    uint64 duration;
    uint64 threadID;
    uint32 processID;
    EventPhase phase;

    template <template <typename, typename> class Container, class TAlloc>
    static void DumpJSON(const Container<TraceEvent, TAlloc>& trace, std::ostream& stream);
};

template <template <typename, typename> class Container, class TAlloc>
void TraceEvent::DumpJSON(const Container<TraceEvent, TAlloc>& trace, std::ostream& stream)
{
    static const char* const PHASE_STR[PHASE_COUNT] = {
        "B", "E", "I", "X"
    };

    stream << "{ \"traceEvents\": [" << std::endl;

    auto begin = trace.begin(), end = trace.end();
    for (auto it = begin; it != end; ++it)
    {
        const TraceEvent& event = (*it);

        if (it != begin)
            stream << "," << std::endl;

        stream << "{ ";
        stream << "\"pid\": " << event.processID << ", ";
        stream << "\"tid\": " << event.threadID << ", ";
        stream << "\"ts\": " << event.timestamp << ", ";

        if (event.phase == PHASE_DURATION)
            stream << "\"dur\": " << event.duration << ", ";

        stream << "\"ph\": \"" << PHASE_STR[event.phase] << "\", ";
        stream << "\"name\": \"" << event.name << "\"";
        stream << " }";
    }

    stream << std::endl
           << "] }" << std::endl;

    stream.flush();
}

}; //ns DAVA