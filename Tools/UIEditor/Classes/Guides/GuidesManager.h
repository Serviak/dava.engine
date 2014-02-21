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

#ifndef __GUIDESMANAGER__H__
#define __GUIDESMANAGER__H__

#include <DAVAEngine.h>
#include "GuideData.h"
#include "GuidesEnums.h"

#include <QObject>

namespace DAVA {

class GuidesManager : public QObject
{
    Q_OBJECT

public:
    GuidesManager();
    virtual ~GuidesManager();
    
    // Functionality related to adding new Guide.
    // Start to adding new guide is get - it might be cancelled though.
    void StartNewGuide(GuideData::eGuideType guideType);
    
    // New guide (guide candidate) is moved.
    void MoveNewGuide(const Vector2& pos);
    
    // Can we accept the new guide?
    bool CanAcceptNewGuide() const;
    
    // New guide is accepted and finally can be added to the guides list.
    const GuideData* AcceptNewGuide();
    
    // New quide adding is cancelled
    void CancelNewGuide();

    // Methods related to the moving existing guide.
    bool StartMoveGuide(const Vector2& pos);
    void MoveGuide(const Vector2& pos);
    
    Vector2 GetMoveGuideStartPos() const;
    const GuideData* AcceptMoveGuide();

    // Selected guides.
    bool AreGuidesSelected() const;
    List<GuideData> DeleteSelectedGuides();

    // Get all the guides.
    const List<GuideData*> GetGuides(bool includeNewGuide) const;

    // Add/remove/update.
    void AddGuide(const GuideData& guideData);
    bool RemoveGuide(const GuideData& guideData);
    bool UpdateGuidePosition(const GuideData& guideData, const Vector2& newPos);

    // Load/Save functionality.
    bool Load(const FilePath& fileName);
    bool Save(const FilePath& fileName, uint32 fileAttr);

    // Calculate the stick offset, return the combination of eGuideStickResult flags.
    int32 CalculateStickToGuides(const List<Rect>& controlsRectList, Vector2& offset) const;

    // Get the stick treshold.
    int32 GetGuideStickTreshold() const;

    // Set the stick mode.
    void SetStickMode(int32 mode);

    // Enable/disable guides.
    bool AreGuidesEnabled() const;
    void SetGuidesEnabled(bool value);

protected:
    // Check whether the same guide exists.
    bool IsGuideExist(GuideData* guideData) const;

    // Check whether the guide passed is on position passed.
    bool IsGuideOnPosition(GuideData* guide, const Vector2& pos) const;

    // Calculate the distance from rect to guide depending on magnet mode.
    Vector2 CalculateDistanceToGuide(GuideData* guide, const Rect& rect) const;

    // Cleanup the memory.
    void Cleanup();

private:
    List<GuideData*> activeGuides;
    
    // New guide candidate.
    GuideData* newGuide;
    
    // Move guide.
    GuideData* moveGuide;
    Vector2 moveGuideStartPos;
    
    // Current stick mode (may be a combination of different ones).
    int32 stickMode;
    
    // Whether the guides are enabled?
    bool guidesEnabled;
};

};

#endif /* defined(__GUIDESMANAGER__H__) */
