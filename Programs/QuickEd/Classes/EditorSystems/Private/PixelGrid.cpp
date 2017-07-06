#include "EditorSystems/PixelGrid.h"

#include "Modules/DocumentsModule/EditorCanvasData.h"

#include <TArc/Core/FieldBinder.h>

#include <UI/UIControl.h>

#include <Reflection/ReflectedTypeDB.h>
#include <Logger/Logger.h>
#include <Preferences/PreferencesStorage.h>
#include <Preferences/PreferencesRegistrator.h>

PixelGrid::PixelGrid(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor)
    : BaseEditorSystem(parent, accessor)
{
    updater.SetUpdater(DAVA::MakeFunction(this, &PixelGrid::DrawGrid));

    InitControls();
    preferences.settingsChanged.Connect(this, &PixelGrid::DrawGrid);
    BindFields();
}

PixelGrid::~PixelGrid() = default;

void PixelGrid::InitControls()
{
    using namespace DAVA;
    UIControl* gridControl = systemsManager->GetPixelGridControl();
    hLinesContainer.Set(new UIControl());
    hLinesContainer->SetName("container for horizontal pixel grid");
    gridControl->AddControl(hLinesContainer.Get());

    vLinesContainer.Set(new UIControl());
    vLinesContainer->SetName("container for vertical pixel grid");
    gridControl->AddControl(vLinesContainer.Get());
}

void PixelGrid::BindFields()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    fieldBinder.reset(new FieldBinder(accessor));
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<EditorCanvasData>();
        fieldDescr.fieldName = EditorCanvasData::startValuePropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &PixelGrid::OnVisualSettingsChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<EditorCanvasData>();
        fieldDescr.fieldName = EditorCanvasData::lastValuePropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &PixelGrid::OnVisualSettingsChanged));
    }
}

void PixelGrid::OnVisualSettingsChanged(const DAVA::Any&)
{
    updater.Update();
}

bool PixelGrid::CanShowGrid() const
{
    if (preferences.IsVisible() == false || preferences.GetScaleToDisplay() < 1.0f)
    {
        return false;
    }

    DAVA::TArc::DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        return false;
    }

    EditorCanvasData* canvasData = activeContext->GetData<EditorCanvasData>();
    if (canvasData->GetScale() < preferences.GetScaleToDisplay())
    {
        return false;
    }

    return true;
}

void PixelGrid::DrawGrid()
{
    using namespace DAVA;

    if (CanShowGrid() == false)
    {
        hLinesContainer->RemoveAllControls();
        vLinesContainer->RemoveAllControls();
        return;
    }

    DAVA::TArc::DataContext* activeContext = accessor->GetActiveContext();
    EditorCanvasData* canvasData = activeContext->GetData<EditorCanvasData>();

    Vector2 startValue = canvasData->GetStartValue();
    Vector2 lastValue = canvasData->GetLastValue();
    Vector2 viewSize = canvasData->GetViewSize();
    int32 scale = static_cast<int32>(canvasData->GetScale());

    for (int i = 0; i < Vector2::AXIS_COUNT; ++i)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);
        UIControl* container = axis == Vector2::AXIS_X ? hLinesContainer.Get() : vLinesContainer.Get();

        //synchronize UIControls count and data
        {
            std::size_t linesCount = static_cast<int32>(std::ceil((lastValue[axis] - startValue[axis]) / scale));
            List<UIControl*> children = container->GetChildren();
            std::size_t childrenCount = children.size();
            auto childrenIter = children.begin();
            while (linesCount < childrenCount)
            {
                container->RemoveControl(*childrenIter);
                ++childrenIter;
                --childrenCount;
            }
            while (linesCount > childrenCount)
            {
                RefPtr<UIControl> line(new UIControl());
                UIControlBackground* background = line->GetOrCreateComponent<UIControlBackground>();
                background->SetDrawType(UIControlBackground::DRAW_FILL);

                container->AddControl(line.Get());
                ++childrenCount;
            }

            DVASSERT(linesCount == container->GetChildren().size());
        }

        //setup UIControls
        List<UIControl*> children = container->GetChildren();
        int32 index = 0;
        int32 offset = static_cast<int32>(startValue[axis]) % scale;
        for (UIControl* line : children)
        {
            UIControlBackground* background = line->GetOrCreateComponent<UIControlBackground>();
            background->SetColor(preferences.GetGridColor());
            int32 position = scale - offset + index * scale;
            DAVA::Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;
            if (axis == Vector2::AXIS_X)
            {
                line->SetPosition(Vector2(static_cast<float32>(position), 0.0f));
                line->SetSize(Vector2(1.0f, viewSize[oppositeAxis]));
            }
            else
            {
                line->SetPosition(Vector2(0.0f, static_cast<float32>(position)));
                line->SetSize(Vector2(viewSize[oppositeAxis], 1.0f));
            }

            index++;
        }
    }
}

PixelGridPreferences::PixelGridPreferences()
{
    PreferencesStorage::Instance()->RegisterPreferences(this);
}

PixelGridPreferences::~PixelGridPreferences()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

DAVA::Color PixelGridPreferences::GetGridColor() const
{
    return gridColor;
}

void PixelGridPreferences::SetGridColor(const DAVA::Color& color)
{
    gridColor = color;
    settingsChanged.Emit();
}

bool PixelGridPreferences::IsVisible() const
{
    return isVisible;
}

void PixelGridPreferences::SetVisible(bool visible)
{
    isVisible = visible;
    settingsChanged.Emit();
}

DAVA::float32 PixelGridPreferences::GetScaleToDisplay() const
{
    return scaleToDisaply;
}

void PixelGridPreferences::SetScaleToDisplay(DAVA::float32 scale)
{
    scaleToDisaply = scale;
    settingsChanged.Emit();
}

REGISTER_PREFERENCES_ON_START(PixelGridPreferences,
                              PREF_ARG("gridColor", DAVA::Color(0.925f, 0.925f, 0.925f, 0.5f)),
                              PREF_ARG("isVisible", true),
                              PREF_ARG("scaleToDisplay", 8.0f)

                              )