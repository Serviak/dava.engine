#pragma once

#include <TArc/DataProcessing/DataNode.h>
#include <Reflection/ReflectionRegistrator.h>

#include <memory>

class EditorTextSystem;
class TextModuleData : public DAVA::TArc::DataNode
{
public:
    static const char* drawingEnabledPropertyName;

    void SetDrawingEnabled(bool enabled);
    bool IsDrawingEnabled() const;

private:
    friend class TextModule;

    std::unique_ptr<EditorTextSystem> editorTextSystem;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TextModuleData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<TextModuleData>::Begin()
        .Field(drawingEnabledPropertyName, &TextModuleData::IsDrawingEnabled, &TextModuleData::SetDrawingEnabled)
        .End();
    }
};
