#include "GraphBase.h"
#include "ControlsFactory.h"

#include "EditorSettings.h"

#if defined(DAVA_QT)
#include "../Qt/SceneData.h"
#include "../Qt/SceneDataManager.h"
#endif //#if defined(DAVA_QT)

GraphBase::GraphBase(GraphBaseDelegate *newDelegate, const Rect &rect)
    :   delegate(newDelegate)
    ,   workingScene(NULL)
{
    CreatePropertyPanel(rect);
}

GraphBase::~GraphBase()
{
    SafeRelease(refreshButton);
    
    SafeRelease(workingScene);

    SafeRelease(graphPanel);
    SafeRelease(propertyPanel);
    
    SafeRelease(graphTree);
    SafeRelease(propertyControl);
}

void GraphBase::CreateGraphPanel(const Rect &rect)
{
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();

    Rect leftRect = Rect(0, 0, leftSideWidth, rect.dy);
    graphPanel = ControlsFactory::CreatePanelControl(leftRect);
}

void GraphBase::CreatePropertyPanel(const Rect &rect)
{
    propertyControl = NULL;
    
    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth(); 
    
    propertyPanelRect = Rect(rect.dx - rightSideWidth, 0, rightSideWidth, rect.dy);
    propertyPanel = ControlsFactory::CreatePanelControl(propertyPanelRect, false);

    refreshButton = ControlsFactory::CreateButton(Rect(0, propertyPanelRect.dy - ControlsFactory::BUTTON_HEIGHT, 
                                                                 propertyPanelRect.dx, ControlsFactory::BUTTON_HEIGHT), 
                                                            LocalizedString(L"panel.refresh"));
    refreshButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &GraphBase::OnRefreshPropertyControl));
    
    propertyPanel->AddControl(refreshButton);
    
    propertyPanelRect.x = propertyPanelRect.y = 0;
    propertyPanelRect.dy -= ControlsFactory::BUTTON_HEIGHT;
}

UIControl * GraphBase::GetGraphPanel()
{
    return graphPanel;
}

UIControl * GraphBase::GetPropertyPanel()
{
    return propertyPanel;
}

void GraphBase::RefreshProperties()
{
#if !defined (DAVA_QT)
    NodesPropertyChanged();
#endif //#if !defined (DAVA_QT)
}


void GraphBase::SetScene(EditorScene *scene)
{
    SafeRelease(workingScene);
    workingScene = SafeRetain(scene);
}

void GraphBase::OnRefreshPropertyControl(DAVA::BaseObject *, void *, void *)
{
    RefreshProperties();
}

bool GraphBase::GraphOnScreen()
{
    return (graphPanel->GetParent() != NULL);
}

bool GraphBase::PropertiesOnScreen()
{
    return (propertyPanel->GetParent() != NULL);
}

void GraphBase::UpdatePropertiesForCurrentNode()
{
    if(propertyControl)
    {
        propertyControl->UpdateFieldsForCurrentNode();
    }
}

void GraphBase::RefreshGraph()
{
    graphTree->Refresh();
}


UIHierarchyCell * GraphBase::CellForNode(UIHierarchy *forHierarchy, void *node)
{
    UIHierarchyCell *c= forHierarchy->GetReusableCell("Graph cell"); //try to get cell from the reusable cells store
    if(!c)
    { 
        //if cell of requested type isn't find in the store create new cell
        int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
        c = new UIHierarchyCell(Rect(0, 0, leftSideWidth, ControlsFactory::CELL_HEIGHT), "Graph cell");
        
        UIControl *icon = new UIControl(Rect(0, 0, ControlsFactory::CELL_HEIGHT, ControlsFactory::CELL_HEIGHT));
        icon->SetName("_Icon_");
        icon->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
        c->text->AddControl(icon);

        UIControl *marker = new UIControl(Rect(0, 0, ControlsFactory::CELL_HEIGHT, ControlsFactory::CELL_HEIGHT));
        marker->SetName("_Marker_");
        marker->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
        c->text->AddControl(marker);
        
        UIStaticText *text = new UIStaticText(Rect(ControlsFactory::CELL_HEIGHT, 0, leftSideWidth - ControlsFactory::CELL_HEIGHT, ControlsFactory::CELL_HEIGHT));
        Font *font = ControlsFactory::GetFontDark();
        text->SetFont(font);
        text->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);
        text->SetName("_Text_");
        c->text->AddControl(text);
    }
    
    FillCell(c, node);
    
    ControlsFactory::CustomizeExpandButton(c->openButton);
    ControlsFactory::CustomizeSceneGraphCell(c);
    
    return c;
}

void GraphBase::OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell)
{
    DVASSERT(delegate);
    if(!delegate->LandscapeEditorActive())
    {
        SelectHierarchyNode(selectedCell->GetNode());
        
        //select 
        List<UIControl*> children = forHierarchy->GetVisibleCells();
        for(List<UIControl*>::iterator it = children.begin(); it != children.end(); ++it)
        {
            UIControl *ctrl = (*it);
            ctrl->SetSelected(false, false);
        }
        
        selectedCell->SetSelected(true, false);
    }
}


void GraphBase::NodesPropertyChanged()
{
#if defined (DAVA_QT)
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    activeScene->RebuildSceneGraph();
#else //#if defined (DAVA_QT)
    RefreshGraph();
#endif //#if defined (DAVA_QT)
}


