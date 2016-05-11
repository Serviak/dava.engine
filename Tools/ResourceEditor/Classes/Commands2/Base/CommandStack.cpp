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


#include "Commands2/Base/CommandStack.h"
#include "Commands2/Base/CommandAction.h"
#include "Commands2/NGTCommand.h"

#include "NgtTools/Common/GlobalContext.h"

#include <core_data_model/variant_list.hpp>
#include <core_data_model/i_value_change_notifier.hpp>
#include <core_command_system/command_instance.hpp>
#include <core_command_system/i_env_system.hpp>
#include <core_reflection/reflected_object.hpp>
#include <core_command_system/i_command_manager.hpp>

namespace CommandStackLocal
{
class CommandIdsAccumulator
{
public:
    CommandIdsAccumulator(DAVA::UnorderedSet<DAVA::int32>& commandIds_)
        : commandIds(commandIds_)
    {
    }

    void operator()(const CommandInstancePtr& instance)
    {
        Command2* cmd = instance->getArguments().getBase<Command2>();
        if (cmd != nullptr)
        {
            this->operator()(cmd);
        }
    }

    void operator()(const Command2* command)
    {
        DAVA::int32 commandId = command->GetId();
        if (commandId == CMDID_BATCH)
        {
            const CommandBatch* batch = static_cast<const CommandBatch*>(command);
            for (DAVA::uint32 i = 0; i < batch->Size(); ++i)
            {
                this->operator()(batch->GetCommand(i));
            }
        }
        else
        {
            commandIds.insert(commandId);
        }
    }

private:
    DAVA::UnorderedSet<DAVA::int32>& commandIds;
};
}

class CommandStack::ActiveCommandStack : public DAVA::StaticSingleton<ActiveCommandStack>
{
public:
    ActiveCommandStack()
    {
        envManager = NGTLayer::queryInterface<IEnvManager>();
        DVASSERT(envManager);
    }

    void CommandStackCreated(CommandStack* commandStack)
    {
        DVASSERT(commandStack != nullptr);
        commandStack->enviromentID = envManager->addEnv("");
    }

    void CommandStackDeleted(CommandStack* commandStack)
    {
        DVASSERT(commandStack != nullptr);
        if (activeStack == commandStack)
        {
            activeStack->DisableConnections();
            activeStack = nullptr;
        }
        envManager->removeEnv(commandStack->enviromentID);
    }

    bool IsActive(const CommandStack* commandStack)
    {
        return activeStack == commandStack;
    }

    void SetActive(CommandStack* commandStack)
    {
        DVASSERT(activationStack.empty());
        DVASSERT(commandStack != nullptr);
        if (activeStack != nullptr)
        {
            activeStack->DisableConnections();
        }

        SetActiveImpl(commandStack);
        activeStack->EnableConections();
    }

    void PushCommandStack(CommandStack* commandStack)
    {
        DVASSERT(commandStack != nullptr);

        bool sameStacks = activeStack == commandStack;
        if (activeStack != nullptr && sameStacks == false)
        {
            activeStack->DisableConnections();
        }

        activationStack.push(activeStack);
        SetActiveImpl(commandStack);

        if (sameStacks == false)
        {
            activeStack->EnableConections();
        }
    }

    void PopCommandStack()
    {
        DVASSERT(activeStack);
        DVASSERT(!activationStack.empty());
        CommandStack* cmdStack = activationStack.top();
        activationStack.pop();
        bool sameStacks = cmdStack == activeStack;
        if (sameStacks == false)
        {
            activeStack->DisableConnections();
        }

        SetActiveImpl(cmdStack);

        if (activeStack != nullptr && sameStacks == false)
        {
            activeStack->EnableConections();
        }
    }

private:
    void SetActiveImpl(CommandStack* cmdStack)
    {
        activeStack = cmdStack;
        if (activeStack)
        {
            envManager->selectEnv(activeStack->enviromentID);
        }
    }

private:
    CommandStack* activeStack = nullptr;
    DAVA::Stack<CommandStack*> activationStack;
    IEnvManager* envManager = nullptr;
};

class CommandStack::ActiveStackGuard
{
public:
    ActiveStackGuard(const CommandStack* commandStack)
    {
        CommandStack::ActiveCommandStack::Instance()->PushCommandStack(const_cast<CommandStack*>(commandStack));
    }

    ~ActiveStackGuard()
    {
        CommandStack::ActiveCommandStack::Instance()->PopCommandStack();
    }
};

CommandStack::CommandStack()
{
    commandManager = NGTLayer::queryInterface<ICommandManager>();
    DVASSERT(commandManager != nullptr);

    ActiveCommandStack::Instance()->CommandStackCreated(this);

    indexChanged = commandManager->signalPostCommandIndexChanged.connect(std::bind(&CommandStack::HistoryIndexChanged, this, std::placeholders::_1));
}

CommandStack::~CommandStack()
{
    DisconnectEvents();
    ActiveCommandStack::Instance()->CommandStackDeleted(this);
}

bool CommandStack::CanUndo() const
{
    ActiveStackGuard guard(this);
    return commandManager->canUndo();
}

bool CommandStack::CanRedo() const
{
    ActiveStackGuard guard(this);
    return commandManager->canRedo();
}

void CommandStack::Clear()
{
    ActiveStackGuard guard(this);
    commandManager->removeCommands([](const CommandInstancePtr&) { return true; });
    CleanCheck();
}

void CommandStack::RemoveCommands(DAVA::int32 commandId)
{
    ActiveStackGuard guard(this);
    commandManager->removeCommands([&commandId](const CommandInstancePtr& instance)
                                   {
                                       Command2* cmd = instance->getArguments().getBase<Command2>();
                                       if (cmd->GetId() == CMDID_BATCH)
                                       {
                                           CommandBatch* batch = static_cast<CommandBatch*>(cmd);

                                           batch->RemoveCommands(commandId);
                                           return batch->Size() == 0;
                                       }

                                       return cmd->GetId() == commandId;
                                   });
    CleanCheck();
}

void CommandStack::Activate()
{
    ActiveCommandStack::Instance()->SetActive(this);
    EmitUndoRedoStateChanged();
}

void CommandStack::Undo()
{
    ActiveStackGuard guard(this);
    if (CanUndo())
    {
        commandManager->undo();
    }
}

void CommandStack::Redo()
{
    ActiveStackGuard guard(this);
    if (CanRedo())
    {
        commandManager->redo();
    }
}

void CommandStack::Exec(Command2::Pointer&& command)
{
    if (CanRedo())
    {
        SetClean(false);
    }

    ActiveStackGuard guard(this);
    if (command != nullptr)
    {
        uncleanCommandIds.insert(command->GetId());
        if (command->CanUndo() && curBatchCommand != nullptr)
        {
            curBatchCommand->AddAndExec(std::move(command));
        }
        else
        {
            commandManager->queueCommand(getClassIdentifier<NGTCommand>(), ObjectHandle(std::move(command)));
        }
    }
}

bool CommandStack::IsUncleanCommandExists(DAVA::int32 commandId) const
{
    return uncleanCommandIds.count(commandId) > 0;
}

void CommandStack::BeginBatch(const DAVA::String& text, DAVA::uint32 commandsCount)
{
    if (nestedBatchesCounter++ == 0)
    {
        curBatchCommand.reset(new CommandBatch(text, commandsCount));
    }
    else
    {
        DVASSERT(curBatchCommand);
    }
}

void CommandStack::EndBatch()
{
    ActiveStackGuard guard(this);
    if (curBatchCommand != nullptr)
    {
        --nestedBatchesCounter;
        DVASSERT(nestedBatchesCounter >= 0);

        if (nestedBatchesCounter > 0)
            return;

        if (curBatchCommand->Size() > 0)
        {
            // As Command2 hierarchy don't have NGT reflection, we have to cast manually
            // to be sure that ObjectHandle will get correct <typename T>
            Command2* batchCommand = curBatchCommand.release();
            commandManager->queueCommand(getClassIdentifier<NGTCommand>(), ObjectHandle(Command2::Pointer(batchCommand)));
        }
        else
        {
            curBatchCommand.reset();
        }
    }
}

bool CommandStack::IsClean() const
{
    if (nextAfterCleanCommandIndex == nextCommandIndex)
    {
        return true;
    }

    if (nextAfterCleanCommandIndex == EMPTY_INDEX)
    {
        return false;
    }

    ActiveStackGuard guard(this);
    DAVA::int32 nextCommand = DAVA::Max(nextCommandIndex, 0);
    DAVA::int32 startCommandIndex = DAVA::Min(nextAfterCleanCommandIndex, nextCommand);
    DAVA::int32 endCommandIndex = DAVA::Max(nextAfterCleanCommandIndex, nextCommand);

    const VariantList& history = commandManager->getHistory();
    DVASSERT(endCommandIndex < static_cast<DAVA::int32>(history.size()));
    for (DAVA::int32 i = startCommandIndex; i < endCommandIndex; ++i)
    {
        CommandInstancePtr instance = history[i].value<CommandInstancePtr>();
        Command2* cmd = instance->getArguments().getBase<Command2>();
        if (cmd == nullptr || cmd->IsModifying())
            return false;
    }

    return true;
}

void CommandStack::SetClean(bool clean)
{
    if (clean)
    {
        nextAfterCleanCommandIndex = nextCommandIndex;
    }
    else
    {
        nextAfterCleanCommandIndex = EMPTY_INDEX;
    }

    ActiveStackGuard guard(this);
    uncleanCommandIds.clear();
    const VariantList& history = commandManager->getHistory();
    CommandStackLocal::CommandIdsAccumulator functor(uncleanCommandIds);
    DAVA::int32 historySize = static_cast<DAVA::int32>(history.size());
    DVASSERT(nextAfterCleanCommandIndex < historySize);
    for (int i = nextAfterCleanCommandIndex + 1; i < historySize; ++i)
    {
        functor(history[i].value<CommandInstancePtr>());
    }

    CleanCheck();
}

void CommandStack::CleanCheck()
{
    if (lastCheckCleanState != IsClean())
    {
        lastCheckCleanState = IsClean();
        EmitCleanChanged(lastCheckCleanState);
    }
}

void CommandStack::commandExecuted(const CommandInstance& commandInstance, CommandOperation operation)
{
    Command2* cmd = commandInstance.getArguments().getBase<Command2>();
    if (cmd != nullptr)
    {
        EmitNotify(cmd, operation != CommandOperation::UNDO);
    }
}

void CommandStack::HistoryIndexChanged(int currentIndex)
{
    nextCommandIndex = currentIndex;
    CleanCheck();
    EmitUndoRedoStateChanged();
}

void CommandStack::DisconnectEvents()
{
    indexChanged.disconnect();
}

void CommandStack::EnableConections()
{
    commandManager->registerCommandStatusListener(this);
    indexChanged.enable();
}

void CommandStack::DisableConnections()
{
    commandManager->deregisterCommandStatusListener(this);
    indexChanged.disable();
}
