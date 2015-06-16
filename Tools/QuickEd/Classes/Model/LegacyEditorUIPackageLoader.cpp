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


#include "LegacyEditorUIPackageLoader.h"

#include "Base/ObjectFactory.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "UI/UIControl.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlHelpers.h"
#include "UI/UIPackage.h"
#include "UI/Components/UIComponent.h"
#include "UI/Layouts/UIAnchorHintComponent.h"
#include "StringUtils.h"

using namespace DAVA;

LegacyEditorUIPackageLoader::LegacyEditorUIPackageLoader(LegacyControlData *data)
    : legacyData(SafeRetain(data))
    , storeAggregatorName(false)
    , aggregatorName("")

{
    // for legacy loading
    
    // UIButton bg
    propertyNamesMap["UIButton"]["sprite"] = "stateSprite";
    propertyNamesMap["UIButton"]["drawType"] = "stateDrawType";
    propertyNamesMap["UIButton"]["align"] = "stateAlign";
    propertyNamesMap["UIButton"]["frame"] = "stateFrame";
    propertyNamesMap["UIButton"]["spriteModification"] = "stateSpriteModification";
    propertyNamesMap["UIButton"]["colorInherit"] = "stateColorInherit";
    propertyNamesMap["UIButton"]["perPixelAccuracy"] = "statePerPixelAccuracy";
    propertyNamesMap["UIButton"]["color"] = "stateColor";
    propertyNamesMap["UIButton"]["leftRightStretchCap"] = "leftRightStretchCap";
    propertyNamesMap["UIButton"]["font"] = "stateFont";
    propertyNamesMap["UIButton"]["text"] = "stateText";
    propertyNamesMap["UIButton"]["multiline"] = "stateMultiline";
    propertyNamesMap["UIButton"]["textColor"] = "stateTextcolor";
    propertyNamesMap["UIButton"]["shadowcolor"] = "stateShadowcolor";
    propertyNamesMap["UIButton"]["shadowoffset"] = "stateShadowoffset";
    propertyNamesMap["UIButton"]["fitting"] = "stateFittingOption";
    propertyNamesMap["UIButton"]["textalign"] = "stateTextAlign";
    propertyNamesMap["UIButton"]["textcolorInheritType"] = "stateTextColorInheritType";
    propertyNamesMap["UIButton"]["textUseRtlAlign"] = "stateTextUseRtlAlign";
    propertyNamesMap["UIButton"]["textMargins"] = "stateTextMargins";
    propertyNamesMap["UIButton"]["margins"] = "stateMargins";
    propertyNamesMap["UIStaticText"]["textColor"] = "textcolor";

    baseClasses["UIButton"] = "UIControl";
    baseClasses["UIListCell"] = "UIButton";
    
    legacyAlignsMap["leftEnabled"] = "leftAlignEnabled";
    legacyAlignsMap["left"] = "leftAlign";
    legacyAlignsMap["hCenterEnabled"] = "hcenterAlignEnabled";
    legacyAlignsMap["hCenter"] = "hcenterAlign";
    legacyAlignsMap["rightEnabled"] = "rightAlignEnabled";
    legacyAlignsMap["right"] = "rightAlign";
    legacyAlignsMap["topEnabled"] = "topAlignEnabled";
    legacyAlignsMap["top"] = "topAlign";
    legacyAlignsMap["vCenterEnabled"] = "vcenterAlignEnabled";
    legacyAlignsMap["vCenter"] = "vcenterAlign";
    legacyAlignsMap["bottomEnabled"] = "bottomAlignEnabled";
    legacyAlignsMap["bottom"] = "bottomAlign";

}

LegacyEditorUIPackageLoader::~LegacyEditorUIPackageLoader()
{
    SafeRelease(legacyData);
}

bool LegacyEditorUIPackageLoader::LoadPackage(const FilePath &packagePath, AbstractUIPackageBuilder *builder)
{
    RefPtr<YamlParser> parser(YamlParser::Create(packagePath));

    if (parser.Get() == NULL)
        return NULL;
    
    YamlNode *rootNode = parser->GetRootNode();
    if (!rootNode)//empty yaml equal to empty UIPackage
    {
        builder->BeginPackage(packagePath);
        builder->EndPackage();
        return true;
    }
    
    builder->BeginPackage(packagePath);
    
    UIControl *legacyControl = builder->BeginControlWithClass("UIControl");
    builder->BeginControlPropertiesSection("UIControl");
    const LegacyControlData::Data *data = legacyData ? legacyData->Get(packagePath.GetFrameworkPath()) : NULL;
    if (data)
    {
        if (storeAggregatorName)
        {
            aggregatorName = data->name;
            storeAggregatorName = false;
        }
        legacyControl->SetName(data->name);
        builder->ProcessProperty(legacyControl->TypeInfo()->Member("size"), VariantType(data->size));
    }
    else
    {
        legacyControl->SetName("LegacyControl");
    }
    builder->EndControlPropertiesSection();

    const YamlNode *childrenNode = rootNode->Get("children");
    if (!childrenNode)
        childrenNode = rootNode;

    for (int32 i = 0; i < (int)childrenNode->GetCount(); i++)
    {
        const YamlNode *childNode = childrenNode->Get(i);
        if (childNode->Get("type"))
        {
            String name = childrenNode->GetItemKeyName(i);
            LoadControl(name, childNode, builder);
        }
    }
    
    builder->EndControl(true);
    
    builder->EndPackage();
    
    return true;
}

bool LegacyEditorUIPackageLoader::LoadControlByName(const DAVA::String &/*name*/, DAVA::AbstractUIPackageBuilder *builder)
{
    DVASSERT(false);
    return false;
}

void LegacyEditorUIPackageLoader::LoadControl(const DAVA::String &name, const YamlNode *node, DAVA::AbstractUIPackageBuilder *builder)
{
    UIControl *control = NULL;
    const YamlNode *type = node->Get("type");
    const YamlNode *baseType = node->Get("baseType");
    bool loadChildren = true;
    if (type->AsString() == "UIAggregatorControl")
    {
        loadChildren = false;
        const YamlNode *pathNode = node->Get("aggregatorPath");
        storeAggregatorName = true;
        aggregatorName = "";
        bool result = builder->ProcessImportedPackage(pathNode->AsString(), this);
        DVASSERT(result);
        DVASSERT(storeAggregatorName == false);
        DVASSERT(!aggregatorName.empty());
        control = builder->BeginControlWithPrototype(FilePath(pathNode->AsString()).GetBasename(), aggregatorName, nullptr, this);
        storeAggregatorName = false;
        aggregatorName = "";
    }
    else if (baseType)
        control = builder->BeginControlWithCustomClass(type->AsString(), baseType->AsString());
    else
        control = builder->BeginControlWithClass(type->AsString());
    

    if (control)
    {
        control->SetName(name);
        LoadControlPropertiesFromYamlNode(control, control->GetTypeInfo(), node, builder);
        LoadBgPropertiesFromYamlNode(control, node, builder);
        LoadInternalControlPropertiesFromYamlNode(control, node, builder);
        ProcessLegacyAligns(control, node, builder);

        // load children
        if (loadChildren)
        {
            const YamlNode * childrenNode = node->Get("children");
            if (childrenNode == NULL)
                childrenNode = node;
            for (uint32 i = 0; i < childrenNode->GetCount(); ++i)
            {
                const YamlNode *childNode = childrenNode->Get(i);
                if (childNode->Get("type"))
                {
                    String name = childrenNode->GetItemKeyName(i);
                    LoadControl(name, childNode, builder);
                }
            }
        }
        
        control->LoadFromYamlNodeCompleted();
    }
    builder->EndControl(false);
}

void LegacyEditorUIPackageLoader::LoadControlPropertiesFromYamlNode(UIControl *control, const InspInfo *typeInfo, const YamlNode *node, DAVA::AbstractUIPackageBuilder *builder)
{
    const InspInfo *baseInfo = typeInfo->BaseInfo();
    if (baseInfo)
        LoadControlPropertiesFromYamlNode(control, baseInfo, node, builder);
    
    builder->BeginControlPropertiesSection(typeInfo->Name());

    String className = control->GetClassName();
    for (int32 i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);
        String memberName = member->Name();
        
        memberName = GetOldPropertyName(className, memberName);
        
        VariantType res;
        if (node)
            res = ReadVariantTypeFromYamlNode(member, node, -1, memberName);
        
        builder->ProcessProperty(member, res);
    }
    builder->EndControlPropertiesSection();
}

void LegacyEditorUIPackageLoader::LoadBgPropertiesFromYamlNode(UIControl *control, const YamlNode *node, DAVA::AbstractUIPackageBuilder *builder)
{
    String className = control->GetClassName();
    for (int32 i = 0; i < control->GetBackgroundComponentsCount(); i++)
    {
        UIControlBackground *bg = builder->BeginBgPropertiesSection(i, true);
        if (bg)
        {
            const InspInfo *insp = bg->GetTypeInfo();
            String bgName = control->GetBackgroundComponentName(i);
            
            for (int32 j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);
                String memberName = member->Name();
                int32 subNodeIndex = -1;
                
                memberName = GetOldPropertyName(className, memberName);
                if (memberName == "stateSprite")
                {
                    subNodeIndex = 0;
                    memberName = "stateSprite";
                }
                else if (memberName == "stateFrame")
                {
                    subNodeIndex = 1;
                    memberName = "stateSprite";
                }
                else if (memberName == "stateSpriteModification")
                {
                    subNodeIndex = 2;
                    memberName = "stateSprite";
                }
                
                memberName = GetOldBgPrefix(className, bgName) + memberName + GetOldBgPostfix(className, bgName);

                if (memberName == "minperPixelAccuracy")
                {
                    memberName = "minpixelAccuracy";
                }
                else if (memberName == "maxperPixelAccuracy")
                {
                    memberName = "maxpixelAccuracy";
                }
                
                VariantType res = ReadVariantTypeFromYamlNode(member, node, subNodeIndex, memberName);
                builder->ProcessProperty(member, res);
            }
        }
        builder->EndBgPropertiesSection();
    }
}

void LegacyEditorUIPackageLoader::LoadInternalControlPropertiesFromYamlNode(UIControl *control, const YamlNode *node, DAVA::AbstractUIPackageBuilder *builder)
{
    String className = control->GetClassName();
    for (int32 i = 0; i < control->GetInternalControlsCount(); i++)
    {
        UIControl *internalControl = builder->BeginInternalControlSection(i, true);
        if (internalControl)
        {
            const InspInfo *insp = internalControl->GetTypeInfo();
            String internalControlName = control->GetInternalControlName(i);
            
            for (int32 j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);
                String memberName = member->Name();
                memberName = GetOldPropertyName(className, memberName);
                memberName = GetOldBgPrefix(className, internalControlName) + memberName + GetOldBgPostfix(className, internalControlName);
                VariantType value = ReadVariantTypeFromYamlNode(member, node, -1, memberName);
                builder->ProcessProperty(member, value);
            }
        }
        builder->EndInternalControlSection();
    }
}

void LegacyEditorUIPackageLoader::ProcessLegacyAligns(UIControl *control, const YamlNode *node, AbstractUIPackageBuilder *builder)
{
    bool hasAnchorProperties = false;
    for (const auto &it : legacyAlignsMap)
    {
        if (node->Get(it.second))
        {
            hasAnchorProperties = true;
        }
    }
    
    if (hasAnchorProperties)
    {
        UIComponent *component = builder->BeginComponentPropertiesSection(UIComponent::ANCHOR_HINT_COMPONENT, 0);
        if (component)
        {
            const InspInfo *insp = component->GetTypeInfo();
            for (int32 j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);
                String name = legacyAlignsMap[String(member->Name())];
                VariantType res = ReadVariantTypeFromYamlNode(member, node, -1, name);
                if (name.find("Enabled") != String::npos)
                {
                    String anchorName = name.substr(0, name.length() - String("Enabled").length());
                    if (node->Get(anchorName) != nullptr)
                    {
                        res = VariantType(true);
                    }
                }
                builder->ProcessProperty(member, res);
            }
        }
        
        builder->EndComponentPropertiesSection();
    }
}

VariantType LegacyEditorUIPackageLoader::ReadVariantTypeFromYamlNode(const InspMember *member, const YamlNode *node, int32 subNodeIndex, const String &propertyName)
{
    const YamlNode *valueNode = node->Get(propertyName);
    if (valueNode)
    {
        if (subNodeIndex != -1)
            valueNode = valueNode->Get(subNodeIndex);
        
        if (member->Desc().type == InspDesc::T_ENUM)
        {
            int32 val = 0;
            if (member->Desc().enumMap->ToValue(valueNode->AsString().c_str(), val))
            {
                return VariantType(val);
            }
            else
            {
                static const std::set<String> textFieldEnums = {
                    "autoCapitalizationType",
                    "autoCorrectionType",
                    "spellCheckingType",
                    "keyboardAppearanceType",
                    "keyboardType",
                    "returnKeyType"
                };

                if (strcmp(member->Name(), "multiline") == 0)
                {
                    if (valueNode->AsBool())
                    {
                        String multilineBySymbolProperty = "multilineBySymbol";
                        if (propertyName != "multiline")
                        {
                            multilineBySymbolProperty = propertyName;
                            DVVERIFY(FindAndReplace(multilineBySymbolProperty, "Multiline", "MultilineBySymbol"));
                        }

                        const YamlNode *bySymbolNode = node->Get(multilineBySymbolProperty);
                        if (bySymbolNode && bySymbolNode->AsBool())
                            return VariantType(UIStaticText::MULTILINE_ENABLED_BY_SYMBOL);
                        else
                            return VariantType(UIStaticText::MULTILINE_ENABLED);
                    }
                    else
                    {
                        return VariantType(UIStaticText::MULTILINE_DISABLED);
                    }
                }
                else if (textFieldEnums.find(propertyName) != textFieldEnums.end())
                {
                    return VariantType(valueNode->AsInt32());
                }
            }
        }
        else if (member->Desc().type == InspDesc::T_FLAGS)
        {
            if (valueNode->GetType() == YamlNode::TYPE_ARRAY)
            {
                int32 val = 0;
                for (uint32 i = 0; i < valueNode->GetCount(); i++)
                {
                    const YamlNode *flagNode = valueNode->Get(i);
                    int32 flag = 0;
                    if (member->Desc().enumMap->ToValue(flagNode->AsString().c_str(), flag))
                    {
                        val |= flag;
                    }
                    else
                    {
                        DVASSERT_MSG(false, Format("No convertion from string to flag value."
                                                   "\n Yaml property name: \"%s\""
                                                   "\n Introspection property name: \"%s\""
                                                   "\n String value: \"%s\"", propertyName.c_str(), member->Name(), flagNode->AsString().c_str()).c_str());
                    }
                }
                return VariantType(val);
            }
            else if (propertyName.find("stateFittingOption") != String::npos)
            {
                return VariantType(valueNode->AsInt32());
            }
        }
        else if (propertyName == "pivot")
        {
            const YamlNode *sizeNode = node->Get("size");
            const YamlNode *rectNode = node->Get("rect");
            DVASSERT(sizeNode || rectNode);

            Vector2 size = sizeNode ? sizeNode->AsVector2() : rectNode->AsRect().GetSize();
            Vector2 pivotPoint = valueNode->AsVector2();
            return VariantType(pivotPoint / size);
        }
        else if (propertyName == "angle")
        {
            return VariantType(RadToDeg(valueNode->AsFloat()));
        }
        else if (member->Type() == MetaInfo::Instance<bool>())
            return VariantType(valueNode->AsBool());
        else if (member->Type() == MetaInfo::Instance<int32>())
            return VariantType(valueNode->AsInt32());
        else if (member->Type() == MetaInfo::Instance<uint32>())
            return VariantType(valueNode->AsUInt32());
        else if (member->Type() == MetaInfo::Instance<String>())
            return VariantType(valueNode->AsString());
        else if (member->Type() == MetaInfo::Instance<WideString>())
            return VariantType(valueNode->AsWString());
        else if (member->Type() == MetaInfo::Instance<float32>())
            return VariantType(valueNode->AsFloat());
        else if (member->Type() == MetaInfo::Instance<Vector2>())
            return VariantType(valueNode->AsVector2());
        else if (member->Type() == MetaInfo::Instance<Color>())
            return VariantType(valueNode->AsColor());
        else if (member->Type() == MetaInfo::Instance<Vector4>())
            return VariantType(valueNode->AsVector4());
        else if (member->Type() == MetaInfo::Instance<FilePath>())
            return VariantType(FilePath(valueNode->AsString()));

        DVASSERT_MSG(false, Format("No legacy convertion for property."
                                   "\n Yaml property name: \"%s\""
                                   "\n Introspection property name: \"%s\"", propertyName.c_str(), member->Name()).c_str());
    }
    else
    {
        String name = member->Name();
        bool isPosition = name == "position";
        if (isPosition || name == "size")
        {
            valueNode = node->Get("rect");
            if (valueNode)
            {
                if (isPosition)
                    return VariantType(valueNode->AsRect().GetPosition());
                else
                    return VariantType(valueNode->AsRect().GetSize());
            }
        }
        
    }
    return VariantType();
}


String LegacyEditorUIPackageLoader::GetOldPropertyName(const String &controlClassName, const String &name) const
{
    auto mapIt = propertyNamesMap.find(controlClassName);
    if (mapIt != propertyNamesMap.end())
    {
        const Map<String, String> &map = mapIt->second;
        auto it = map.find(name);
        if (it != map.end())
            return it->second;
    }
    
    auto baseIt = baseClasses.find(controlClassName);
    if (baseIt != baseClasses.end())
        return GetOldPropertyName(baseIt->second, name);
    
    return name;
}

String LegacyEditorUIPackageLoader::GetOldBgPrefix(const String &controlClassName, const String &name) const
{
    if (controlClassName == "UISlider" && name != "Background")
        return name;
    else
        return "";
}

String LegacyEditorUIPackageLoader::GetOldBgPostfix(const String &controlClassName, const String &name) const
{
    if (controlClassName == "UIButton")
        return name;

    auto baseIt = baseClasses.find(controlClassName);
    if (baseIt != baseClasses.end())
        return GetOldBgPostfix(baseIt->second, name);

    return "";
}
