#pragma once

#include <TArc/DataProcessing/DataWrapper.h>

#include <Math/Vector.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
}
}

class CanvasData;
class CentralWidgetData;

class CanvasDataAdapter : public DAVA::ReflectionBase
{
public:
    explicit CanvasDataAdapter(DAVA::TArc::ContextAccessor* accessor);
    ~CanvasDataAdapter() override;

    static DAVA::FastName scalePropertyName;

    static DAVA::FastName positionPropertyName;
    static DAVA::FastName minimumPositionPropertyName;
    static DAVA::FastName maximumPositionPropertyName;

    //values displayed on screen. Used by rulers, guides and grid
    static DAVA::FastName startValuePropertyName;
    static DAVA::FastName lastValuePropertyName;

    //current screen position in abs coordinates
    static DAVA::FastName movableControlPositionPropertyName;

    DAVA::Vector2 GetPosition() const;
    void SetPosition(const DAVA::Vector2& pos);

    DAVA::Vector2 GetMinimumPosition() const;
    DAVA::Vector2 GetMaximumPosition() const;

    DAVA::Vector2 GetStartValue() const;
    DAVA::Vector2 GetLastValue() const;

    DAVA::float32 GetScale() const;
    void SetScale(DAVA::float32 scale);
    void SetScale(DAVA::float32 scale, const DAVA::Vector2& referencePoint);

    DAVA::Vector2 GetMovableControlPosition() const;

    DAVA::Vector2 GetViewSize() const;

private:
    DAVA::TArc::DataWrapper canvasDataWrapper;

    const CanvasData* GetCanvasData() const;

    DAVA::TArc::ContextAccessor* accessor = nullptr;

    DAVA_VIRTUAL_REFLECTION(CanvasDataAdapter, DAVA::ReflectionBase);
};
