#pragma once

#include "Math/Vector.h"
#include "UI/Layouts/Private/ControlLayoutData.h"

namespace DAVA
{
class Layouter
{
public:
    void ApplyLayout(UIControl* control);
    void ApplyLayoutNonRecursive(UIControl* control);

    void CollectControls(UIControl* control, bool recursive);
    void CollectControlChildren(UIControl* control, int32 parentIndex, bool recursive);

    void ProcessAxis(Vector2::eAxis axis, bool processSizes);
    void DoMeasurePhase(Vector2::eAxis axis);
    void DoLayoutPhase(Vector2::eAxis axis);

    void ApplySizesAndPositions();
    void ApplyPositions();

    void SetRtl(bool rtl);

private:
    Vector<ControlLayoutData> layoutData;
    bool isRtl = false;
};
}