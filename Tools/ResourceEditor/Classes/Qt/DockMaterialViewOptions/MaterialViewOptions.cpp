#include "MaterialViewOptions.h"
#include "ui_MaterialViewOptions.h"
#include <stdlib.h> 
#include "Project/ProjectManager.h"
#include "Classes/Qt/Main/QtMainWindowHandler.h"
#include "../SceneEditor/EditorConfig.h"

MaterialViewOptions::MaterialViewOptions(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::MaterialViewOptions)
{
	ui->setupUi(this);
	
	Init();
}

MaterialViewOptions::~MaterialViewOptions()
{
	delete ui;
}

void MaterialViewOptions::Init()
{
	QtMainWindowHandler * handler = QtMainWindowHandler::Instance();
	
	QComboBox * comboBox = ui->comboBox;
	
	for(int i = 0; i < Material::MATERIAL_VIEW_COUNT; i++)
	{
		switch (i)
		{
			case Material::MATERIAL_VIEW_COMBO:
				comboBox->addItem("Texture with lightmap");
				break;
			case Material::MATERIAL_VIEW_TEXTURE_ONLY:
				comboBox->addItem("Texture only");
				break;
			case Material::MATERIAL_VIEW_LIGHTMAP_ONLY:
				comboBox->addItem("Lightmap only");
				break;
		}
	}

	connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), handler, SLOT(MaterialViewOptionChanged(int)));

	handler->RegisterMaterialViewOptionsWidgets(ui->comboBox);

	handler->SetMaterialViewOptionsWidgetsState(true);
}
