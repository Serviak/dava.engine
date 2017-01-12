#pragma once

#include "TArc/Core/ClientModule.h"

#include "Debug/DVAssert.h"

#include <gmock/gmock.h>

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif

namespace DAVA
{
namespace TArc
{
class MockClientModule : public ClientModule
{
public:
    MockClientModule()
    {
        using namespace ::testing;

        EXPECT_CALL(*this, PostInit());

        ON_CALL(*this, OnContextCreated(_))
        .WillByDefault(Invoke(this, &MockClientModule::OnContextCreatedImpl));
        ON_CALL(*this, OnContextDeleted(_))
        .WillByDefault(Invoke(this, &MockClientModule::OnContextDeletedImpl));
        ON_CALL(*this, OnWindowClosed(_))
        .WillByDefault(Invoke(this, &MockClientModule::OnWindowClosedImpl));
        ON_CALL(*this, OnContextWillBeChanged(_, _))
        .WillByDefault(Invoke(this, &MockClientModule::OnContextWillBeChangedImpl));
        ON_CALL(*this, OnContextWasChanged(_, _))
        .WillByDefault(Invoke(this, &MockClientModule::OnContextWasChangedImpl));
        ON_CALL(*this, PostInit())
        .WillByDefault(Invoke(this, &MockClientModule::PostInitImpl));
    }

    ~MockClientModule()
    {
    }

    MOCK_METHOD1(OnContextCreated, void(DataContext* context));
    MOCK_METHOD1(OnContextDeleted, void(DataContext* context));
    MOCK_METHOD1(OnWindowClosed, void(const WindowKey& key));
    MOCK_METHOD2(OnContextWillBeChanged, void(DataContext* current, DataContext* newOne));
    MOCK_METHOD2(OnContextWasChanged, void(DataContext* current, DataContext* oldOne));
    MOCK_METHOD0(PostInit, void());

    virtual void OnContextCreatedImpl(DataContext* context)
    {
    }
    virtual void OnContextDeletedImpl(DataContext* context)
    {
    }
    virtual void OnWindowClosedImpl(const WindowKey& key)
    {
    }
    virtual void OnContextWillBeChangedImpl(DataContext* current, DataContext* newOne)
    {
    }
    virtual void OnContextWasChangedImpl(DataContext* current, DataContext* oldOne)
    {
    }
    virtual void PostInitImpl()
    {
    }

    DAVA_VIRTUAL_REFLECTION(MockClientModule, ClientModule)
    {
        ReflectionRegistrator<MockClientModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

} // namespace TArc
} // namespace DAVA

#if __clang__
#pragma clang diagnostic pop
#endif
