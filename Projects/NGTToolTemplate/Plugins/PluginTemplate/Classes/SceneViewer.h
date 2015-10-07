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

#ifndef DAVAPLUGIN_SCENEVIEWER_H
#define DAVAPLUGIN_SCENEVIEWER_H

#include "core_ui_framework/i_view.hpp"

#include <memory>

#include <QObject>

class PluginGLWidget;

namespace DAVA
{
    class UI3DView;
    class Scene;
    class Entity;
}

class SceneViewer : public QObject
{
    Q_OBJECT
public:
    SceneViewer();
    ~SceneViewer();

    void Finalise();

    IView & GetView();
    Q_SLOT void OnOpenScene(std::string const & scenePath);

    Q_SIGNAL void SceneLoaded(DAVA::Scene * scene);

    void SetSelection(DAVA::Entity* entity);

private:
    Q_SLOT void OnGlInitialized();
    Q_SLOT void OnGlResized(int width, int height, int dpr);

private:
    std::unique_ptr<PluginGLWidget> glWidget;
    DAVA::UI3DView * uiView = nullptr;
};

#endif // DAVAPLUGIN_SCENEVIEWER_H