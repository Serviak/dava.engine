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


#ifndef __QUICKED_TRANSFORM_SYSTEM_H__
#define __QUICKED_TRANSFORM_SYSTEM_H__

#include "Systems/BaseSystem.h"
#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "Document.h"

class TransformSystem final : public BaseSystem
{   
public:
    explicit TransformSystem(SystemsManager* parent);
    ~TransformSystem() = default;

    bool OnInput(DAVA::UIEvent *currentInput) override;

private:
    void OnActiveAreaChanged(const HUDAreaInfo& areaInfo);

    enum ACCUMULATE_OPERATIONS
    {
        ROTATE_OPERATION,
        MOVE_OPERATION,
        RESIZE_OPERATION,
        OPERATIONS_COUNT
    };
    bool ProcessKey(const DAVA::int32 key);

    bool ProcessDrag(const DAVA::Vector2 &pos);
    void ResizeControl(const DAVA::Vector2 &pos, bool withPivot, bool rateably);

    void MoveControl(const DAVA::Vector2 &pos);
    void MoveAllSelectedControls(const DAVA::Vector2 &delta);

    template <typename T>
    void AdjustProperty(ControlNode *node, const DAVA::String &propertyName, const T &value);

    void AccumulateOperation(ACCUMULATE_OPERATIONS operation, DAVA::Vector2 &delta);

    HUDAreaInfo::eArea activeArea = HUDAreaInfo::NO_AREA;
    ControlNode *activeControlNode = nullptr;
    DAVA::Vector2 prevPos;
    const DAVA::Array<int, OPERATIONS_COUNT> steps; //to transform with fixed step
    DAVA::Array<DAVA::Array<int, DAVA::Vector2::AXIS_COUNT>, OPERATIONS_COUNT> accumulates;

    SelectedControls selectedControlNodes;
};

#endif // __QUICKED_TRANSFORM_SYSTEM_H__
