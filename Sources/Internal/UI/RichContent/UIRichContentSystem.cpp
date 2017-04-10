#include "FileSystem/XMLParser.h"
#include "UI/DefaultUIPackageBuilder.h"
#include "UI/UIPackageLoader.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIStaticText.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/RichContent/UIRichAliasMap.h"
#include "UI/RichContent/UIRichContentComponent.h"
#include "UI/RichContent/UIRichContentSystem.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "Utils/BiDiHelper.h"
#include "Utils/UTF8Utils.h"
#include "Utils/Utils.h"

namespace DAVA
{
class XMLRichContentBuilder final : public XMLParserDelegate
{
public:
    XMLRichContentBuilder(UIRichContentComponent* component)
        : component(component)
    {
        DVASSERT(component);
        PutClass(component->GetBaseClasses());
    }

    bool Build(const String& text)
    {
        controls.clear();
        RefPtr<XMLParser> parser(new XMLParser());
        return parser->ParseBytes(reinterpret_cast<const unsigned char*>(text.c_str()), static_cast<int32>(text.length()), this);
    }

    const Vector<RefPtr<UIControl>>& GetControls() const
    {
        return controls;
    }

    void PutClass(const String& clazz)
    {
        String compositeClass = GetClass() + " " + clazz;
        classesStack.push_back(compositeClass);
    }

    void PopClass()
    {
        classesStack.pop_back();
    }

    const String& GetClass() const
    {
        if (classesStack.empty())
        {
            static const String EMPTY;
            return EMPTY;
        }
        return classesStack.back();
    }

    void PrepareControl(UIControl* ctrl, bool autosize)
    {
        ctrl->SetClassesFromString(GetClass());

        if (autosize)
        {
            UISizePolicyComponent* sp = ctrl->GetOrCreateComponent<UISizePolicyComponent>();
            sp->SetHorizontalPolicy(UISizePolicyComponent::eSizePolicy::PERCENT_OF_CONTENT);
            sp->SetVerticalPolicy(UISizePolicyComponent::eSizePolicy::PERCENT_OF_CONTENT);
        }

        UIFlowLayoutHintComponent* flh = ctrl->GetOrCreateComponent<UIFlowLayoutHintComponent>();
        flh->SetContentDirection(direction);
        if (needLineBreak)
        {
            flh->SetNewLineBeforeThis(needLineBreak);
            needLineBreak = false;
        }
    }

    void OnElementStarted(const String& elementName, const String& namespaceURI, const String& qualifedName, const Map<String, String>& attributes) override
    {
        const UIRichAliasMap& aliases = component->GetAliases();
        if (aliases.HasAlias(elementName))
        {
            const UIRichAliasMap::Alias& alias = aliases.GetAlias(elementName);
            ProcessTagBegin(alias.tag, alias.attributes);
        }
        else
        {
            ProcessTagBegin(elementName, attributes);
        }
    }

    void OnElementEnded(const String& elementName, const String& namespaceURI, const String& qualifedName) override
    {
        const UIRichAliasMap& aliases = component->GetAliases();
        if (aliases.HasAlias(elementName))
        {
            const UIRichAliasMap::Alias& alias = aliases.GetAlias(elementName);
            ProcessTagEnd(alias.tag);
        }
        else
        {
            ProcessTagEnd(elementName);
        }
    }

    void OnFoundCharacters(const String& chars) override
    {
        ProcessText(chars);
    }

    void ProcessTagBegin(const String& tag, const Map<String, String>& attributes)
    {
        // Global attributes
        String classes;
        GetAttribute(attributes, "class", classes);
        PutClass(classes);

        // Tag
        if (tag == "p")
        {
            needLineBreak = true;
        }
        else if (tag == "br")
        {
            needLineBreak = true;
        }
        else if (tag == "ul")
        {
            needLineBreak = true;
        }
        else if (tag == "li")
        {
            needLineBreak = true;
            ProcessText("*"); // TODO: Change to create "bullet" control
        }
        else if (tag == "img")
        {
            String src;
            if (GetAttribute(attributes, "src", src))
            {
                UIControl* img = new UIControl();
                PrepareControl(img, true);
                UIControlBackground* bg = img->GetOrCreateComponent<UIControlBackground>();
                bg->SetSprite(FilePath(src));
                controls.emplace_back(img);
            }
        }
        else if (tag == "object")
        {
            String path;
            GetAttribute(attributes, "path", path);
            String controlName;
            GetAttribute(attributes, "control", controlName);
            String prototypeName;
            GetAttribute(attributes, "prototype", prototypeName);
            String name;
            GetAttribute(attributes, "name", name);

            if (!path.empty() && (!controlName.empty() || !prototypeName.empty()))
            {
                DefaultUIPackageBuilder pkgBuilder;
                UIPackageLoader().LoadPackage(path, &pkgBuilder);
                UIControl* obj = nullptr;
                if (!controlName.empty())
                {
                    obj = pkgBuilder.GetPackage()->GetControl(controlName);
                }
                else if (!prototypeName.empty())
                {
                    obj = pkgBuilder.GetPackage()->GetPrototype(prototypeName);
                }
                if (obj != nullptr)
                {
                    obj = obj->Clone(); // Clone control from package
                    PrepareControl(obj, false);
                    if (!name.empty())
                    {
                        obj->SetName(name);
                    }
                    component->onCreateObject.Emit(obj);
                    controls.emplace_back(obj);
                }
            }
        }
    }

    void ProcessTagEnd(const String& tag)
    {
        PopClass();

        if (tag == "p")
        {
            needLineBreak = true;
        }
        else if (tag == "ul")
        {
            needLineBreak = true;
        }
        else if (tag == "li")
        {
            needLineBreak = true;
        }
    }

    void ProcessText(const String& text)
    {
        const static String LTR_MARK = UTF8Utils::EncodeToUTF8(L"\u200E");
        const static String RTL_MARK = UTF8Utils::EncodeToUTF8(L"\u200F");

        Vector<String> tokens;
        Split(text, " \n\r\t", tokens);
        for (String& token : tokens)
        {
            BiDiHelper::Direction wordDirection = bidiHelper.GetDirectionUTF8String(token);
            if (wordDirection == BiDiHelper::Direction::NEUTRAL)
            {
                if (direction == BiDiHelper::Direction::RTL)
                {
                    token = RTL_MARK + token;
                }
                else if (direction == BiDiHelper::Direction::LTR)
                {
                    token = LTR_MARK + token;
                }
            }
            else
            {
                direction = wordDirection;
            }

            UIStaticText* ctrl = new UIStaticText();
            PrepareControl(ctrl, true);
            ctrl->SetUtf8Text(token);
#if _DEBUG // TODO: Remove before merge
            ctrl->SetForceBiDiSupportEnabled(true);
#endif
            controls.emplace_back(ctrl);
        }
    }

private:
    bool needLineBreak = false;
    BiDiHelper::Direction direction = BiDiHelper::Direction::NEUTRAL;
    Vector<String> classesStack;
    Vector<RefPtr<UIControl>> controls;
    BiDiHelper bidiHelper;
    UIRichContentComponent* component = nullptr;
};

/*******************************************************************************************************/

void UIRichContentSystem::RegisterControl(UIControl* control)
{
    UISystem::RegisterControl(control);
    UIRichContentComponent* component = control->GetComponent<UIRichContentComponent>();
    if (component)
    {
        AddLink(component);
    }
}

void UIRichContentSystem::UnregisterControl(UIControl* control)
{
    UIRichContentComponent* component = control->GetComponent<UIRichContentComponent>();
    if (component)
    {
        RemoveLink(component);
    }

    UISystem::UnregisterControl(control);
}

void UIRichContentSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    UISystem::RegisterComponent(control, component);

    if (component->GetType() == UIRichContentComponent::C_TYPE)
    {
        AddLink(static_cast<UIRichContentComponent*>(component));
    }
}

void UIRichContentSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIRichContentComponent::C_TYPE)
    {
        RemoveLink(static_cast<UIRichContentComponent*>(component));
    }

    UISystem::UnregisterComponent(control, component);
}

void UIRichContentSystem::Process(float32 elapsedTime)
{
    // Remove empty links
    if (!links.empty())
    {
        links.erase(std::remove_if(links.begin(), links.end(), [](const Link& l) {
                        return l.component == nullptr;
                    }),
                    links.end());
    }

    // Process links
    for (Link& l : links)
    {
        DVASSERT(l.component);
        if (l.component->IsModified())
        {
            l.component->SetModified(false);

            UIControl* root = l.component->GetControl();
            root->RemoveAllControls();

            XMLRichContentBuilder builder(l.component);
            if (builder.Build("<span>" + l.component->GetText() + "</span>"))
            {
                for (const RefPtr<UIControl>& ctrl : builder.GetControls())
                {
                    root->AddControl(ctrl.Get());
                }
            }
        }
    }
}

void UIRichContentSystem::AddLink(UIRichContentComponent* component)
{
    DVASSERT(component);
    component->SetModified(true);
    links.emplace_back(component);
}

void UIRichContentSystem::RemoveLink(UIRichContentComponent* component)
{
    DVASSERT(component);
    auto findIt = std::find_if(links.begin(), links.end(), [&component](const Link& l) {
        return l.component == component;
    });
    DVASSERT(findIt != links.end());
    findIt->component->GetControl()->RemoveAllControls();
    findIt->component = nullptr; // mark link for delete
}
}
