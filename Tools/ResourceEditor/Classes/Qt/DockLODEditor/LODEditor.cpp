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



#include "LODEditor.h"
#include "ui_LODEditor.h"

#include "EditorLODData.h"
#include "DistanceSlider.h"

#include "Scene/SceneSignals.h"
#include "Classes/Qt/Scene/SceneSignals.h"
#include "Classes/Qt/PlaneLODDialog/PlaneLODDialog.h"
#include "Classes/Qt/Main/mainwindow.h"

#include <QLabel>
#include <QWidget>
#include <QLineEdit>
#include <QInputDialog>


struct DistanceWidget
{
    QLabel *name;
    QDoubleSpinBox *distance;

    void SetVisible(bool visible)
    {
        name->setVisible(visible);
        distance->setVisible(visible);
    }
};

static DistanceWidget distanceWidgets[DAVA::LodComponent::MAX_LOD_LAYERS];


LODEditor::LODEditor(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::LODEditor)
{
	ui->setupUi(this);
    
    editedLODData = new EditorLODData();
    connect(editedLODData, SIGNAL(DataChanged()), SLOT(LODDataChanged()));

    SetupInternalUI();
    SetupSceneSignals();
    
    ForceDistanceStateChanged(Qt::Unchecked);
    LODDataChanged();
    
    posSaver.Attach(this);

	allSceneModeEnabled = SettingsManager::GetValue(Settings::Internal_LODEditorMode).AsBool();
	ui->checkBoxLodEditorMode->setChecked(allSceneModeEnabled);
	editedLODData->EnableAllSceneMode(allSceneModeEnabled);
}

LODEditor::~LODEditor()
{
    SafeDelete(editedLODData);
    
	delete ui;
}

void LODEditor::SetupInternalUI()
{
    ui->lodEditorSettingsButton->setStyleSheet("Text-align:left");
    ui->viewLODButton->setStyleSheet("Text-align:left");
    ui->editLODButton->setStyleSheet("Text-align:left");
    
    connect(ui->lodEditorSettingsButton, SIGNAL(released()), SLOT(LODEditorSettingsButtonReleased()));
    connect(ui->viewLODButton, SIGNAL(released()), SLOT(ViewLODButtonReleased()));
    connect(ui->editLODButton, SIGNAL(released()), SLOT(EditLODButtonReleased()));
    
    connect(ui->enableForceDistance, SIGNAL(stateChanged(int)), SLOT(ForceDistanceStateChanged(int)));
	connect(ui->enableForceDistance, SIGNAL(toggled(bool)), ui->forceSlider, SLOT(setEnabled(bool)));
    connect(ui->forceSlider, SIGNAL(valueChanged(int)), SLOT(ForceDistanceChanged(int)));
    ui->forceSlider->setRange(0, DAVA::LodComponent::MAX_LOD_DISTANCE);
    ui->forceSlider->setValue(0);
    
    connect(ui->distanceSlider, SIGNAL(DistanceChanged(const QVector<int> &, bool)), SLOT(LODDistanceChangedBySlider(const QVector<int> &, bool)));
    
    InitDistanceSpinBox(ui->lod0Name, ui->lod0Distance, 0);
    InitDistanceSpinBox(ui->lod1Name, ui->lod1Distance, 1);
    InitDistanceSpinBox(ui->lod2Name, ui->lod2Distance, 2);
    InitDistanceSpinBox(ui->lod3Name, ui->lod3Distance, 3);
    
    SetForceLayerValues(DAVA::LodComponent::MAX_LOD_LAYERS);
    connect(ui->forceLayer, SIGNAL(activated(int)), SLOT(ForceLayerActivated(int)));

	connect(ui->checkBoxLodEditorMode, SIGNAL(stateChanged(int)), this, SLOT(EditorModeChanged(int)));

    //TODO: remove after lod editing implementation
    connect(ui->lastLodToFrontButton, SIGNAL(clicked()), this, SLOT(CopyLODToLod0Clicked()));
    connect(ui->createPlaneLodButton, SIGNAL(clicked()), this, SLOT(CreatePlaneLODClicked()));
    connect(ui->buttonDeleteFirstLOD, SIGNAL(clicked()), editedLODData, SLOT(DeleteFirstLOD()));
    connect(ui->buttonDeleteLastLOD, SIGNAL(clicked()), editedLODData, SLOT(DeleteLastLOD()));
}

void LODEditor::SetupSceneSignals()
{
    connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(SceneActivated(SceneEditor2 *)));
    connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(SceneDeactivated(SceneEditor2 *)));
}

void LODEditor::ForceDistanceStateChanged(int checked)
{
    bool enable = (checked == Qt::Checked);
    editedLODData->EnableForceDistance(enable);
    
    if(!enable)
    {
        int layer = ui->forceLayer->itemData(ui->forceLayer->currentIndex()).toInt();
        editedLODData->SetForceLayer(layer);
    }
    
	ui->enableForceDistance->setChecked(checked);
	ui->forceSlider->setValue(editedLODData->GetForceDistance());
    ui->forceLayer->setEnabled(!enable);
}

void LODEditor::ForceDistanceChanged(int distance)
{
    editedLODData->SetForceDistance(distance);
}


void LODEditor::InitDistanceSpinBox(QLabel *name, QDoubleSpinBox *spinbox, int index)
{
    spinbox->setRange(0.f, DAVA::LodComponent::MAX_LOD_DISTANCE);  //distance 
    spinbox->setProperty(ResourceEditor::TAG.c_str(), index);
    spinbox->setValue(0.f);
    spinbox->setFocusPolicy(Qt::WheelFocus);
    spinbox->setKeyboardTracking(false);
    
    connect(spinbox, SIGNAL(valueChanged(double)), SLOT(LODDistanceChangedBySpinbox(double)));
    
    distanceWidgets[index].name = name;
    distanceWidgets[index].distance = spinbox;
    
    distanceWidgets->SetVisible(false);
}


void LODEditor::SceneActivated(SceneEditor2 *scene)
{
    //TODO: set gloabal scene settings
}


void LODEditor::SceneDeactivated(SceneEditor2 *scene)
{
    //TODO: clear/save gloabal scene settings

    ForceDistanceStateChanged(Qt::Unchecked);
}


void LODEditor::LODDataChanged()
{
    DAVA::uint32 lodLayersCount = editedLODData->GetLayersCount();
    
    ui->distanceSlider->SetLayersCount(lodLayersCount);
    SetForceLayerValues(lodLayersCount);
    
    for (DAVA::uint32 i = 0; i < lodLayersCount; ++i)
    {
        distanceWidgets[i].SetVisible(true);
        
        DAVA::float32 distance = editedLODData->GetLayerDistance(i);
        
        SetSpinboxValue(distanceWidgets[i].distance, distance);
        ui->distanceSlider->SetDistance(i, distance);
        
        distanceWidgets[i].name->setText(Format("%d. (%d):", i, editedLODData->GetLayerTriangles(i)).c_str());
    }
    for (DAVA::int32 i = lodLayersCount; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        distanceWidgets[i].SetVisible(false);
    }
    
    UpdateWidgetVisibility();

    ui->lastLodToFrontButton->setEnabled(editedLODData->CanCreatePlaneLOD());
    ui->createPlaneLodButton->setEnabled(editedLODData->CanCreatePlaneLOD());

    UpdateDeleteLODButtons();
}

void LODEditor::LODDistanceChangedBySlider(const QVector<int> &changedLayers, bool continuous)
{
	ui->distanceSlider->LockDistances(true);

	if(changedLayers.size() != 0)
	{
		DAVA::Map<DAVA::uint32, DAVA::float32> lodDistances;
		for (int i = 0; i < changedLayers.size(); i++)
		{
			int layer = changedLayers[i];

			double value = ui->distanceSlider->GetDistance(layer);
			SetSpinboxValue(distanceWidgets[layer].distance, value);

			if(!continuous)
			{
				lodDistances[layer] = value;
			}
		}

		editedLODData->UpdateDistances(lodDistances);
	}

	ui->distanceSlider->LockDistances(false);
}

void LODEditor::LODDistanceChangedBySpinbox(double value)
{
    QDoubleSpinBox *spinBox = dynamic_cast<QDoubleSpinBox *>(sender());
    if(spinBox)
    {
        //TODO set new value to scene
        int lodLevel = spinBox->property(ResourceEditor::TAG.c_str()).toInt();

        {
            const bool wasBlocked = editedLODData->blockSignals(true);
            editedLODData->SetLayerDistance(lodLevel, value);
            editedLODData->blockSignals(wasBlocked);
        }
        {
            const bool wasBlocked = ui->distanceSlider->blockSignals(true);
            ui->distanceSlider->SetDistance(lodLevel, value);
            ui->distanceSlider->blockSignals(wasBlocked);
        }
    }
}

void LODEditor::SetSpinboxValue(QDoubleSpinBox *spinbox, double value)
{
    bool wasBlocked = spinbox->blockSignals(true);
    spinbox->setValue(value);
    spinbox->blockSignals(wasBlocked);
}

void LODEditor::ForceLayerActivated(int index)
{
    int layer = ui->forceLayer->itemData(index).toInt();
    editedLODData->SetForceLayer(layer);
}

void LODEditor::SetForceLayerValues(int layersCount)
{
    ui->forceLayer->clear();
    
    ui->forceLayer->addItem("Auto", QVariant(DAVA::LodComponent::INVALID_LOD_LAYER));

	int requestedIndex = editedLODData->GetForceLayer() + 1;
	int itemsCount = Max(requestedIndex, layersCount);
	for(DAVA::int32 i = 0; i < itemsCount; ++i)
    {
        ui->forceLayer->addItem(Format("%d", i).c_str(), QVariant(i));
    }
    
	ui->forceLayer->setCurrentIndex(requestedIndex);
}

void LODEditor::LODEditorSettingsButtonReleased()
{
    InvertFrameVisibility(ui->frameLodEditorSettings, ui->lodEditorSettingsButton);
}

void LODEditor::ViewLODButtonReleased()
{
    InvertFrameVisibility(ui->frameViewLOD, ui->viewLODButton);
    
    if(ui->frameViewLOD->isVisible() == false)
    {
        editedLODData->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
        editedLODData->SetForceLayer(DAVA::LodComponent::INVALID_LOD_LAYER);
        
        ui->enableForceDistance->setCheckState(Qt::Unchecked);
        ui->forceLayer->setCurrentIndex(0);
    }
}

void LODEditor::EditLODButtonReleased()
{
    InvertFrameVisibility(ui->frameEditLOD, ui->editLODButton);
}

void LODEditor::InvertFrameVisibility(QFrame *frame, QPushButton *frameButton)
{
    bool visible = frame->isVisible();
    frame->setVisible(!visible);

	QIcon icon = (frame->isVisible()) ? QIcon(":/QtIcons/advanced.png") : QIcon(":/QtIcons/play.png");
	frameButton->setIcon(icon);
}


void LODEditor::UpdateWidgetVisibility()
{
    bool visible = (editedLODData->GetLayersCount() != 0);
    
    ui->viewLODButton->setVisible(visible);
    ui->frameViewLOD->setVisible(visible);
    ui->editLODButton->setVisible(visible);
    ui->frameEditLOD->setVisible(visible);
}

void LODEditor::CopyLODToLod0Clicked()
{
    if(editedLODData->CanCreatePlaneLOD())
        editedLODData->CopyLastLodToLod0();
}

void LODEditor::CreatePlaneLODClicked()
{
    if(editedLODData->CanCreatePlaneLOD())
    {
        FilePath defaultTexturePath = editedLODData->GetDefaultTexturePathForPlaneEntity();

        PlaneLODDialog dialog(editedLODData->GetLayersCount(), defaultTexturePath, this);
        if(dialog.exec() == QDialog::Accepted)
        {
            QtMainWindow::Instance()->WaitStart("Creating Plane LOD", "Please wait...");

            editedLODData->CreatePlaneLOD(dialog.GetSelectedLayer(), dialog.GetSelectedTextureSize(), dialog.GetSelectedTexturePath());

            QtMainWindow::Instance()->WaitStop();
        }
    }
}

void LODEditor::EditorModeChanged(int newMode)
{
	allSceneModeEnabled = (newMode == Qt::Checked);
	DAVA::VariantType value(allSceneModeEnabled);
	SettingsManager::SetValue(Settings::Internal_LODEditorMode, value);

	editedLODData->EnableAllSceneMode(allSceneModeEnabled);
    
    UpdateDeleteLODButtons();
}

void LODEditor::UpdateDeleteLODButtons()
{
    bool canDeleteLOD = editedLODData->CanDeleteLod();
    
    ui->buttonDeleteFirstLOD->setEnabled(canDeleteLOD);
    ui->buttonDeleteLastLOD->setEnabled(canDeleteLOD);
}


