	#include "QSceneGraphTreeView.h"

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Scene/SceneData.h"
#include "../Scene/SceneDataManager.h"

#include "../Commands/CommandsManager.h"
#include "../Commands/SceneGraphCommands.h"
#include "../Commands/LibraryCommands.h"

#include "SceneGraphModel.h"

#include <QKeyEvent>

QSceneGraphTreeView::QSceneGraphTreeView(QWidget *parent)
    :   QTreeView(parent),
		sceneGraphModel(NULL)
{
	sceneGraphModel = new SceneGraphModel();
    sceneGraphModel->SetScene(NULL);
	
	ConnectToSignals();
}

QSceneGraphTreeView::~QSceneGraphTreeView()
{
	DisconnectFromSignals();
	SafeDelete(sceneGraphModel);
}

void QSceneGraphTreeView::ConnectToSignals()
{
	connect(sceneGraphModel, SIGNAL(SceneNodeSelected(DAVA::SceneNode *)), this, SLOT(OnSceneNodeSelectedInGraph(DAVA::SceneNode *)));
	

	// Signals to rebuild the particular node and the whole graph.
	connect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRebuildNode(DAVA::SceneNode*)), this, SLOT(OnSceneGraphNeedRebuildNode(DAVA::SceneNode*)));
	connect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRebuild()), this, SLOT(OnSceneGraphNeedRebuild()));
	connect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRefreshLayer(DAVA::ParticleLayer*)), this, SLOT(OnSceneGraphNeedRefreshLayer(DAVA::ParticleLayer*)));

	connect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedSetScene(SceneData*, EditorScene*)),
			this, SLOT(OnSceneGraphNeedSetScene(SceneData*, EditorScene*)));
	connect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedSelectNode(SceneData*, DAVA::SceneNode*)),
			this, SLOT(OnSceneGraphNeedSelectNode(SceneData*, DAVA::SceneNode*)));

	// Signals related to the whole Scene.
	connect(SceneDataManager::Instance(), SIGNAL(SceneCreated(SceneData*)),	this, SLOT(OnSceneCreated(SceneData*)));
	connect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData*)), this, SLOT(OnSceneReleased(SceneData*)));
	connect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData*)),	this, SLOT(OnSceneActivated(SceneData*)));
	connect(SceneDataManager::Instance(), SIGNAL(SceneDeactivated(SceneData*)), this, SLOT(OnSceneDeactivated(SceneData*)));
}

void QSceneGraphTreeView::DisconnectFromSignals()
{
	disconnect(sceneGraphModel, SIGNAL(SceneNodeSelected(DAVA::SceneNode *)), this, SLOT(OnSceneNodeSelectedInGraph(DAVA::SceneNode *)));
	
	disconnect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRebuildNode(DAVA::SceneNode*)), this, SLOT(OnSceneGraphNeedRebuildNode(DAVA::SceneNode*)));
	disconnect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRebuild()), this, SLOT(OnSceneGraphNeedRebuild()));
	disconnect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRefreshLayer(DAVA::ParticleLayer*)), this, SLOT(OnSceneGraphNeedRefreshLayer(DAVA::ParticleLayer*)));

	
	disconnect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedSetScene(SceneData*, EditorScene*)),
			   this, SLOT(OnSceneGraphNeedSetScene(SceneData*, EditorScene*)));
	disconnect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedSelectNode(SceneData*, DAVA::SceneNode*)),
				this, SLOT(OnSceneGraphNeedSelectNode(SceneData*, DAVA::SceneNode*)));
	
	// Signals related to the whole Scene.
	disconnect(SceneDataManager::Instance(), SIGNAL(SceneCreated(SceneData*)),	this, SLOT(OnSceneCreated(SceneData*)));
	disconnect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData*)), this, SLOT(OnSceneReleased(SceneData*)));
	disconnect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData*)),	this, SLOT(OnSceneActivated(SceneData*)));
	disconnect(SceneDataManager::Instance(), SIGNAL(SceneDeactivated(SceneData*)), this, SLOT(OnSceneDeactivated(SceneData*)));
}

void QSceneGraphTreeView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_X)
    {
        SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
        if(screen)
        {
            screen->ProcessIsSolidChanging();
        }
    }
    
    QTreeView::keyPressEvent(event);
}

void QSceneGraphTreeView::OnSceneNodeSelectedInGraph(DAVA::SceneNode *node)
{
	// TODO: Yuri Coder, 12/21/2012. Think about the nicer method.
	SceneDataManager::Instance()->SceneNodeSelectedInSceneGraph(node);
}


void QSceneGraphTreeView::OnSceneCreated(SceneData* scene)
{
	this->sceneGraphModel->SetScene(scene->GetScene());
}

void QSceneGraphTreeView::OnSceneReleased(SceneData* scene)
{
	this->sceneGraphModel->SetScene(NULL);
}

void QSceneGraphTreeView::OnSceneActivated(SceneData* scene)
{
	this->sceneGraphModel->SetScene(scene->GetScene());
    this->sceneGraphModel->Activate(this);
    
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnSceneGraphContextMenuRequested(const QPoint &)));
}

void QSceneGraphTreeView::OnSceneDeactivated(SceneData* scene)
{
    sceneGraphModel->Deactivate();
    
	disconnect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnSceneGraphContextMenuRequested(const QPoint &)));
}

void QSceneGraphTreeView::OnSceneGraphNeedRebuildNode(DAVA::SceneNode* node)
{
	sceneGraphModel->RebuildNode(node);
}

void QSceneGraphTreeView::OnSceneGraphNeedRebuild()
{
	sceneGraphModel->Rebuild();
}

void QSceneGraphTreeView::OnSceneGraphNeedSetScene(SceneData *sceneData, EditorScene *scene)
{
	sceneGraphModel->SetScene(scene);
}

void QSceneGraphTreeView::OnSceneGraphNeedSelectNode(SceneData *sceneData, DAVA::SceneNode* node)
{
	sceneGraphModel->SelectNode(node);
}

void QSceneGraphTreeView::OnSceneGraphContextMenuRequested(const QPoint &point)
{
    QModelIndex itemIndex = indexAt(point);
    ShowSceneGraphMenu(itemIndex, QCursor::pos());
}

void QSceneGraphTreeView::ShowSceneGraphMenu(const QModelIndex &index, const QPoint &point)
{
    if(!index.isValid())
    {
        return;
    }
    
    QMenu menu;
    
	// For "custom" Particles Editor nodes the "generic" ones aren't needed".
    if (sceneGraphModel->GetParticlesEditorSceneModelHelper().NeedDisplaySceneEditorPopupMenuItems(index))
    {
		AddActionToMenu(&menu, QString("Remove Root Nodes"), new CommandRemoveRootNodes());
		AddActionToMenu(&menu, QString("Look at Object"), new CommandLockAtObject());
		AddActionToMenu(&menu, QString("Remove Object"), new CommandRemoveSceneNode());
	
		AddActionToMenu(&menu, QString("Debug Flags"), new CommandDebugFlags());
    
		SceneNode *node = static_cast<SceneNode *>(sceneGraphModel->ItemData(index));
		if (node)
		{
			KeyedArchive *properties = node->GetCustomProperties();
			if (properties && properties->IsKeyExists(String("editor.referenceToOwner")))
			{
				String filePathname = properties->GetString(String("editor.referenceToOwner"));
				AddActionToMenu(&menu, QString("Edit Model"), new CommandEditScene(filePathname));
				AddActionToMenu(&menu, QString("Reload Model"), new CommandReloadScene(filePathname));
			}
		}
	}
	
	// For "custom" Particles Editor nodes the "generic" ones aren't needed".
    // We might need more menu items/actions for Particles Editor.
    sceneGraphModel->GetParticlesEditorSceneModelHelper().AddPopupMenuItems(menu, index);

    connect(&menu, SIGNAL(triggered(QAction *)), this, SLOT(SceneGraphMenuTriggered(QAction *)));
    menu.exec(point);
}

void QSceneGraphTreeView::SceneGraphMenuTriggered(QAction *action)
{
	ProcessContextMenuAction(action);
}

void QSceneGraphTreeView::AddActionToMenu(QMenu *menu, const QString &actionTitle, Command *command)
{
    QAction *action = menu->addAction(actionTitle);
    action->setData(PointerHolder<Command *>::ToQVariant(command));
}

void QSceneGraphTreeView::ProcessContextMenuAction(QAction *action)
{
	Command *command = PointerHolder<Command *>::ToPointer(action->data());
	ExecuteCommand(command);
}

void QSceneGraphTreeView::ExecuteCommand(Command *command)
{
	CommandsManager::Instance()->Execute(command);
	SafeRelease(command);
}

void QSceneGraphTreeView::OnSceneGraphNeedRefreshLayer(DAVA::ParticleLayer* layer)
{
	sceneGraphModel->RefreshParticlesLayer(layer);
}
