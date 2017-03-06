#pragma once

#include "Base/BaseTypes.h"
#include "Command/CommandBatch.h"

class RECommand;
class RECommandBatch final : public DAVA::CommandBatch
{
public:
    RECommandBatch(const DAVA::String& description, DAVA::uint32 commandsCount);

    void RemoveCommands(DAVA::uint32 commandId);

    RECommand* GetCommand(DAVA::uint32 index) const;

    bool MatchCommandID(DAVA::uint32 commandID) const;
};
