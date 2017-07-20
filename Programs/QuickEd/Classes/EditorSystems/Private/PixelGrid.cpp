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
    bool startValueChanged = find(fields.begin(), fields.end(), CanvasDataAdapter::startValuePropertyName) != fields.end();
    bool lastValueChanged = find(fields.begin(), fields.end(), CanvasDataAdapter::lastValuePropertyName) != fields.end();
    bool scaleChanged = find(fields.begin(), fields.end(), CanvasDataAdapter::scalePropertyName) != fields.end();

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
    if (systemsManager->GetDisplayState() == EditorSystemsManager::Emulation)
    {
        return false;
    }

    DAVA::float32 scaleToDisplay = preferences.GetScaleToDisplay() / 100.0f;
    if (preferences.IsVisible() == false || scaleToDisplay < 1.0f)
    {
        return false;
    }

    if (canvasDataAdapter.GetScale() < scaleToDisplay)
    {
        return false;
    }

    return true;
}

void PixelGrid::UpdateGrid()
{
    using namespace std;
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
    float32 scale = canvasDataAdapter.GetScale();

    for (int i = 0; i < Vector2::AXIS_COUNT; ++i)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);
        UIControl* container = axis == Vector2::AXIS_X ? hLinesContainer.Get() : vLinesContainer.Get();

        //synchronize UIControls count and data
        {
            float32 distanceInPixels = ceilf((lastValue[axis] - startValue[axis]) / scale);
            size_t linesCount = static_cast<size_t>(distanceInPixels);
            List<UIControl*> children = container->GetChildren();
            size_t childrenCount = children.size();
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
        float32 value = 0.0f;

        for (UIControl* line : children)
        {
            UIControlBackground* background = line->GetOrCreateComponent<UIControlBackground>();
            background->SetColor(preferences.GetGridColor());
            float32 position = canvasDataAdapter.RelativeValueToPosition(value, axis);

            DAVA::Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;
            if (axis == Vector2::AXIS_X)
            {
                line->SetPosition(Vector2(position, 0.0f));
                line->SetSize(Vector2(1.0f, viewSize[oppositeAxis]));
            }
            else
            {
                line->SetPosition(Vector2(0.0f, position));
                line->SetSize(Vector2(viewSize[oppositeAxis], 1.0f));
            }

            value++;
        }
    }
}

void PixelGrid::OnDisplayStateChanged(EditorSystemsManager::eDisplayState /*currentState*/, EditorSystemsManager::eDisplayState /*previousState*/)
{
    updater.MarkDirty();
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
                              PREF_ARG("scaleToDisplay", 800.0f)
                              )
