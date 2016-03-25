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


#ifndef __RESOURCEEDITORQT__CUSTOMCOLORSPANEL__
#define __RESOURCEEDITORQT__CUSTOMCOLORSPANEL__

#include "LandscapeEditorBasePanel.h"
#include "DAVAEngine.h"
#include "Main/Request.h"

using namespace DAVA;

class QComboBox;
class QPushButton;
class SliderWidget;

class CustomColorsPanel : public LandscapeEditorBasePanel
{
    Q_OBJECT

public:
    static const int DEF_BRUSH_MIN_SIZE = 3;
    static const int DEF_BRUSH_MAX_SIZE = 40;

    explicit CustomColorsPanel(QWidget* parent = 0);

private slots:
    void ProjectOpened(const QString& path);

    void SetBrushSize(int brushSize);
    void SetColor(int color);
    bool SaveTexture();
    void LoadTexture();
    void SaveTextureIfNeeded(SceneEditor2* scene);

    void IncreaseBrushSize();
    void DecreaseBrushSize();
    void IncreaseBrushSizeLarge();
    void DecreaseBrushSizeLarge();

    void PrevTexture();
    void NextTexture();

protected:
    bool GetEditorEnabled();

    void SetWidgetsState(bool enabled) override;
    void BlockAllSignals(bool block) override;

    void InitUI() override;
    void ConnectToSignals() override;

    void StoreState() override;
    void RestoreState() override;

    void ConnectToShortcuts() override;
    void DisconnectFromShortcuts() override;

private:
    void InitColors();
    int32 BrushSizeUIToSystem(int32 uiValue);
    int32 BrushSizeSystemToUI(int32 systemValue);

private:
    QComboBox* comboColor = nullptr;
    SliderWidget* sliderWidgetBrushSize = nullptr;
    QPushButton* buttonSaveTexture = nullptr;
    QPushButton* buttonLoadTexture = nullptr;
};

#endif /* defined(__RESOURCEEDITORQT__CUSTOMCOLORSPANEL__) */
