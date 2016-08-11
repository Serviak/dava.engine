#include "UnitTests/UnitTests.h"

#include "Command/Command.h"
#include "Command/CommandBatch.h"
#include "Command/CommandStack.h"

class TestCommand : public DAVA::Command
{
public:
    TestCommand(const DAVA::String& description = "")
        : DAVA::Command(description)
    {
    }

    void Redo() override
    {
        ++redoCounter;
    }

    void Undo()
    {
        ++undoCounter;
    }

    DAVA::int32 redoCounter = 0;
    DAVA::int32 undoCounter = 0;
};

class TestCommandClean : public TestCommand
{
public:
    TestCommandClean(const DAVA::String& description = "")
        : TestCommand(description)
    {
    }
    bool IsClean() const override
    {
        return true;
    }
};

DAVA_TESTCLASS (CommandsTest)
{
    DAVA_TEST (SimpleTest)
    {
        std::unique_ptr<DAVA::Command> command(new TestCommand("TestCommand"));
        TEST_VERIFY(command->GetDescription() == "TestCommand");
        TEST_VERIFY(command->IsClean() == false);

        command->Redo();
        command->Undo();
        command->Redo();

        TestCommand* testCommand = static_cast<TestCommand*>(command.get());
        TEST_VERIFY(testCommand->redoCounter == 2);
        TEST_VERIFY(testCommand->undoCounter == 1);

        std::unique_ptr<DAVA::Command> commandClean(new TestCommandClean("TestCommandClean"));
        TEST_VERIFY(commandClean->GetDescription() == "TestCommandClean");
        TEST_VERIFY(commandClean->IsClean() == true);
    }

    DAVA_TEST (CommandStackTest)
    {
        DAVA::CommandStack stack;
        TEST_VERIFY(stack.IsClean() == true);
        TEST_VERIFY(stack.CanUndo() == false);
        TEST_VERIFY(stack.CanRedo() == false);

        std::unique_ptr<DAVA::Command> command(new TestCommand("TestCommand"));
        TestCommand* testCommand = static_cast<TestCommand*>(command.get());
        stack.Exec(std::move(command));
        TEST_VERIFY(testCommand->redoCounter == 1);
        TEST_VERIFY(testCommand->undoCounter == 0);
        TEST_VERIFY(stack.IsClean() == false);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == false);

        stack.Undo();
        TEST_VERIFY(testCommand->redoCounter == 1);
        TEST_VERIFY(testCommand->undoCounter == 1);
        TEST_VERIFY(stack.IsClean() == true);
        TEST_VERIFY(stack.CanUndo() == false);
        TEST_VERIFY(stack.CanRedo() == true);

        stack.Redo();
        TEST_VERIFY(testCommand->redoCounter == 2);
        TEST_VERIFY(testCommand->undoCounter == 1);
        TEST_VERIFY(stack.IsClean() == false);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == false);

        stack.Exec(std::unique_ptr<DAVA::Command>(new TestCommand("TestCommand2")));
        TEST_VERIFY(testCommand->redoCounter == 2);
        TEST_VERIFY(testCommand->undoCounter == 1);
        TEST_VERIFY(stack.IsClean() == false);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == false);

        stack.Undo();
        TEST_VERIFY(testCommand->redoCounter == 2);
        TEST_VERIFY(testCommand->undoCounter == 1);
        TEST_VERIFY(stack.IsClean() == false);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == true);

        stack.Undo();
        TEST_VERIFY(testCommand->redoCounter == 2);
        TEST_VERIFY(testCommand->undoCounter == 2);
        TEST_VERIFY(stack.IsClean() == true);
        TEST_VERIFY(stack.CanUndo() == false);
        TEST_VERIFY(stack.CanRedo() == true);

        stack.Redo();
        TEST_VERIFY(testCommand->redoCounter == 3);
        TEST_VERIFY(testCommand->undoCounter == 2);
        TEST_VERIFY(stack.IsClean() == false);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == true);

        stack.Redo();
        TEST_VERIFY(testCommand->redoCounter == 3);
        TEST_VERIFY(testCommand->undoCounter == 2);
        TEST_VERIFY(stack.IsClean() == false);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == false);
    }

    DAVA_TEST (CommandStackCleanTest)
    {
        DAVA::CommandStack stack;
        stack.Exec(std::unique_ptr<DAVA::Command>(new TestCommandClean()));
        TEST_VERIFY(stack.IsClean() == true);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == false);
    }

    DAVA_TEST (CommandBatchTest)
    {
        std::unique_ptr<DAVA::Command> command(new TestCommand("TestCommand"));
        TestCommand* testCommand = static_cast<TestCommand*>(command.get());

        DAVA::CommandStack stack;
        stack.BeginBatch("TestBatch", 1);

        stack.Exec(std::move(command));
        TEST_VERIFY(testCommand->redoCounter == 1);
        TEST_VERIFY(testCommand->undoCounter == 0);

        stack.EndBatch();
        TEST_VERIFY(testCommand->redoCounter == 1);
        TEST_VERIFY(testCommand->undoCounter == 0);
        TEST_VERIFY(stack.IsClean() == false);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == false);

        stack.Undo();
        TEST_VERIFY(testCommand->redoCounter == 1);
        TEST_VERIFY(testCommand->undoCounter == 1);
        TEST_VERIFY(stack.IsClean() == true);
        TEST_VERIFY(stack.CanUndo() == false);
        TEST_VERIFY(stack.CanRedo() == true);

        stack.Redo();
        TEST_VERIFY(testCommand->redoCounter == 2);
        TEST_VERIFY(testCommand->undoCounter == 1);
        TEST_VERIFY(stack.IsClean() == false);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == false);
    }

    DAVA_TEST (CommandBatchInnerTest)
    {
        std::unique_ptr<DAVA::Command> command(new TestCommand("TestCommand"));
        TestCommand* testCommand = static_cast<TestCommand*>(command.get());
        TestCommand* testCommandInner = nullptr;

        DAVA::CommandStack stack;
        stack.BeginBatch("RootBatch", 1);
        stack.Exec(std::move(command));

        {
            stack.BeginBatch("InnerBatch", 2);

            std::unique_ptr<DAVA::Command> innerCommand(new TestCommand("TestCommand_inner"));
            testCommandInner = static_cast<TestCommand*>(innerCommand.get());

            stack.Exec(std::move(innerCommand));
            TEST_VERIFY(testCommandInner->redoCounter == 1);
            TEST_VERIFY(testCommandInner->undoCounter == 0);

            stack.Exec(std::unique_ptr<DAVA::Command>(new TestCommandClean("TestCommand_inner_clean")));
            TEST_VERIFY(testCommandInner->redoCounter == 1);
            TEST_VERIFY(testCommandInner->undoCounter == 0);

            stack.EndBatch();
            TEST_VERIFY(testCommandInner->redoCounter == 1);
            TEST_VERIFY(testCommandInner->undoCounter == 0);
        }

        stack.EndBatch();
        TEST_VERIFY(testCommand->redoCounter == 1);
        TEST_VERIFY(testCommand->undoCounter == 0);
        TEST_VERIFY(testCommandInner->redoCounter == 1);
        TEST_VERIFY(testCommandInner->undoCounter == 0);
        TEST_VERIFY(stack.IsClean() == false);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == false);

        stack.Undo();
        TEST_VERIFY(testCommand->redoCounter == 1);
        TEST_VERIFY(testCommand->undoCounter == 1);
        TEST_VERIFY(testCommandInner->redoCounter == 1);
        TEST_VERIFY(testCommandInner->undoCounter == 1);
        TEST_VERIFY(stack.IsClean() == true);
        TEST_VERIFY(stack.CanUndo() == false);
        TEST_VERIFY(stack.CanRedo() == true);

        stack.Redo();
        TEST_VERIFY(testCommand->redoCounter == 2);
        TEST_VERIFY(testCommand->undoCounter == 1);
        TEST_VERIFY(testCommandInner->redoCounter == 2);
        TEST_VERIFY(testCommandInner->undoCounter == 1);
        TEST_VERIFY(stack.IsClean() == false);
        TEST_VERIFY(stack.CanUndo() == true);
        TEST_VERIFY(stack.CanRedo() == false);
    }
};
