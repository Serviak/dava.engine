#include "EditorSystems/PixelGrid.h"

#include "UI/Preview/Data/CanvasDataAdapter.h"

#include <TArc/Core/ContextAccessor.h>

#include <UI/UIControl.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Logger/Logger.h>
#include <Preferences/PreferencesStorage.h>
#include <Preferences/PreferencesRegistrator.h>

PixelGrid::PixelGrid(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor)
    : BaseEditorSystem(parent, accessor)
    , canvasDataAdapter(accessor)
{
    canvasDataAdapterWrapper = accessor->CreateWrapper([this](const DAVA::TArc::DataContext*) { return DAVA::Reflection::Create(&canvasDataAdapter); });
    canvasDataAdapterWrapper.SetListener(this);

    updater.SetCallback(DAVA::MakeFunction(this, &PixelGrid::UpdateGrid));

    InitControls();
    preferences.settingsChanged.Connect(&updater, &DirtyFrameUpdater::MarkDirty);
}

PixelGrid::~PixelGrid() = default;

void PixelGrid::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    bool startValueChanged = std::find(fields.begin(), fields.end(), CanvasDataAdapter::startValuePropertyName) != fields.end();
    bool lastValueChanged = std::find(fields.begin(), fields.end(), CanvasDataAdapter::lastValuePropertyName) != fields.end();
    bool scaleChanged = std::find(fields.begin(), fields.end(), CanvasDataAdapter::scalePropertyName) != fields.end();

    if (startValueChanged || lastValueChanged || scaleChanged)
    {
        updater.MarkDirty();
    }
}

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

bool PixelGrid::CanShowGrid() const
{
    if (preferences.IsVisible() == false || preferences.GetScaleToDisplay() < 1.0f)
    {
        return false;
    }

    if (canvasDataAdapter.GetScale() < preferences.GetScaleToDisplay())
    {
        return false;
    }

    return true;
}

void PixelGrid::UpdateGrid()
{
    using namespace DAVA;

    if (CanShowGrid() == false)
    {
        hLinesContainer->RemoveAllControls();
        vLinesContainer->RemoveAllControls();
        return;
    }

    Vector2 startValue = canvasDataAdapter.GetStartValue();
    Vector2 lastValue = canvasDataAdapter.GetLastValue();
    Vector2 viewSize = canvasDataAdapter.GetViewSize();
    int32 scale = static_cast<int32>(canvasDataAdapter.GetScale());

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
            int32 position = index * scale;
            if (offset > 0)
            {
                position += scale - offset;
            }
            else if (offset < 0)
            {
                position -= offset;
            }
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