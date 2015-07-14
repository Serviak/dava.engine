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


#include "PropertiesWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>
#include <QMenu>
#include <QItemSelection>

#include "UI/QtModelPackageCommandExecutor.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "Model/ControlProperties/StyleSheetProperty.h"
#include "Model/ControlProperties/StyleSheetSelectorProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"

#include <QAbstractItemModel>
#include "SharedData.h"

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"

#include "UI/Components/UIComponent.h"
#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
    , sharedData(nullptr)
    , addComponentAction(nullptr)
    , removeComponentAction(nullptr)
    , addStylePropertyAction(nullptr)
    , removeStylePropertyAction(nullptr)
    , addStyleSelectorAction(nullptr)
    , removeStyleSelectorAction(nullptr)
    , selectedComponentType(-1)
    , selectedComponentIndex(-1)
    , selectedStylePropertyIndex(-1)
{
    setupUi(this);
    treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));

    PrepareComponentActions();
    PrepareStyleActions();
}

void PropertiesWidget::OnDocumentChanged(SharedData *arg)
{
    sharedData = arg;
    UpdateSelection();
}

void PropertiesWidget::OnDataChanged(const QByteArray &role)
{
    if (role == "selection")
    {
        UpdateSelection();
    }
}

void PropertiesWidget::OnAddComponent(QAction *action)
{
    if (sharedData)
    {
        uint32 componentType = action->data().toUInt();
        if (componentType < UIComponent::COMPONENT_COUNT)
        {
            ControlNode *node = GetSelectedControlNode();
            sharedData->GetDocument()->GetCommandExecutor()->AddComponent(node, componentType);
        }
        else
        {
            DVASSERT(componentType < UIComponent::COMPONENT_COUNT);
        }
    }
}

void PropertiesWidget::OnRemoveComponent()
{
    if (sharedData)
    {
        if (0 <= selectedComponentType && selectedComponentType < UIComponent::COMPONENT_COUNT)
        {
            ControlNode *node = GetSelectedControlNode();
            sharedData->GetDocument()->GetCommandExecutor()->RemoveComponent(node, selectedComponentType, selectedComponentIndex);
        }
        else
        {
            DVASSERT(false);
        }
    }
    UpdateActions();
}

void PropertiesWidget::OnAddStyleProperty(QAction *action)
{
    if (sharedData)
    {
        uint32 propertyIndex = action->data().toUInt();
        if (propertyIndex < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT)
        {
            StyleSheetNode *node = GetSelectedStyleSheetNode();
            sharedData->GetDocument()->GetCommandExecutor()->AddStyleProperty(node, propertyIndex);
        }
        else
        {
            DVASSERT(propertyIndex < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT);
        }
    }
    
}

void PropertiesWidget::OnRemoveStyleProperty()
{
    if (sharedData)
    {
        if (0 <= selectedStylePropertyIndex && selectedStylePropertyIndex < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT)
        {
            StyleSheetNode *node = GetSelectedStyleSheetNode();
            sharedData->GetDocument()->GetCommandExecutor()->RemoveStyleProperty(node, selectedStylePropertyIndex);
        }
        else
        {
            DVASSERT(false);
        }
    }
    UpdateActions();
}

void PropertiesWidget::OnAddStyleSelector()
{
    if (sharedData)
    {
        sharedData->GetDocument()->GetCommandExecutor()->AddStyleSelector(GetSelectedStyleSheetNode());
    }
}

void PropertiesWidget::OnRemoveStyleSelector()
{
    if (sharedData)
    {
        
        QModelIndexList indices = treeView->selectionModel()->selectedIndexes();
        if (!indices.empty())
        {
            const QModelIndex &index = indices.first();
            AbstractProperty *property = static_cast<AbstractProperty*>(index.internalPointer());
            
            if ((property->GetFlags() & AbstractProperty::EF_CAN_REMOVE) != 0)
            {
                StyleSheetSelectorProperty *selectorProperty = dynamic_cast<StyleSheetSelectorProperty*>(property);
                if (selectorProperty)
                {
                    int32 index = property->GetParent()->GetIndex(selectorProperty);
                    if (index != -1)
                        sharedData->GetDocument()->GetCommandExecutor()->RemoveStyleSelector(GetSelectedStyleSheetNode(), index);
                }
            }
        }
        
    }
}

void PropertiesWidget::OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    UpdateActions();
}

void PropertiesWidget::PrepareComponentActions()
{
    QMenu *addComponentMenu = new QMenu(this);
    for (int32 i = 0; i < UIComponent::COMPONENT_COUNT; i++)
    {
        const char *name = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(i);
        QAction *componentAction = new QAction(name, this); // TODO: Localize name
        componentAction->setData(i);
        addComponentMenu->addAction(componentAction);
    }
    connect(addComponentMenu, &QMenu::triggered, this, &PropertiesWidget::OnAddComponent);
    
    addComponentAction = new QAction(tr("Add Component"), this);
    addComponentAction->setEnabled(false);
    addComponentAction->setMenu(addComponentMenu);
    
    removeComponentAction = new QAction(tr("Remove Component"), this);
    removeComponentAction->setEnabled(false);
    connect(removeComponentAction, &QAction::triggered, this, &PropertiesWidget::OnRemoveComponent);
    
    treeView->addAction(addComponentAction);
    treeView->addAction(removeComponentAction);
    
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    treeView->addAction(separator);
}

void PropertiesWidget::PrepareStyleActions()
{
    addStyleSelectorAction = new QAction(tr("Add Style Selector"), this);
    addStyleSelectorAction->setEnabled(false);
    connect(addStyleSelectorAction, &QAction::triggered, this, &PropertiesWidget::OnAddStyleSelector);
    
    removeStyleSelectorAction = new QAction(tr("Remove Style Selector"), this);
    removeStyleSelectorAction->setEnabled(false);
    connect(removeStyleSelectorAction, &QAction::triggered, this, &PropertiesWidget::OnRemoveStyleSelector);
    
    treeView->addAction(addStyleSelectorAction);
    treeView->addAction(removeStyleSelectorAction);

    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    treeView->addAction(separator);
    
    QMenu *propertiesMenu = new QMenu(this);
    UIStyleSheetPropertyDataBase *db = UIStyleSheetPropertyDataBase::Instance();
    for (int32 i = 0; i < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT; i++)
    {
        const UIStyleSheetPropertyDescriptor &descr = db->GetStyleSheetPropertyByIndex(i);
        QAction *componentAction = new QAction(descr.name.c_str(), this);
        componentAction->setData(i);
        propertiesMenu->addAction(componentAction);
    }
    connect(propertiesMenu, &QMenu::triggered, this, &PropertiesWidget::OnAddStyleProperty);
    
    addStylePropertyAction = new QAction(tr("Add Style Property"), this);
    addStylePropertyAction->setEnabled(false);
    addStylePropertyAction->setMenu(propertiesMenu);
    
    removeStylePropertyAction = new QAction(tr("Remove Style Property"), this);
    removeStylePropertyAction->setEnabled(false);
    connect(removeStylePropertyAction, &QAction::triggered, this, &PropertiesWidget::OnRemoveStyleProperty);
    
    treeView->addAction(addStylePropertyAction);
    treeView->addAction(removeStylePropertyAction);
}

ControlNode *PropertiesWidget::GetSelectedControlNode() const
{
    if (!sharedData)
        return nullptr;
    
    const QList<PackageBaseNode*> &selection = sharedData->GetSelection();
    if (selection.empty())
        return nullptr;
    
    for (PackageBaseNode *node : selection)
    {
        ControlNode *control = dynamic_cast<ControlNode*>(node);
        if (control)
            return control;
    }
    return nullptr;
}

StyleSheetNode *PropertiesWidget::GetSelectedStyleSheetNode() const
{
    if (!sharedData)
        return nullptr;
    
    const QList<PackageBaseNode*> &selection = sharedData->GetSelection();
    if (selection.empty())
        return nullptr;
    
    for (PackageBaseNode *node : selection)
    {
        StyleSheetNode *styleSheet = dynamic_cast<StyleSheetNode*>(node);
        if (styleSheet)
        {
            return styleSheet;
        }
    }
    return nullptr;
}

void PropertiesWidget::UpdateSelection()
{
    QAbstractItemModel *prevModel = treeView->model();
    ControlNode *control = nullptr;
    StyleSheetNode *styleSheet = nullptr;
    if (nullptr == sharedData)
    {
        treeView->setModel(nullptr);
    }
    else
    {
        const QList<PackageBaseNode*> &selection = sharedData->GetSelection();
        
        for (PackageBaseNode *node : selection)
        {
            control = dynamic_cast<ControlNode*>(node);
            if (control)
            {
                treeView->setModel(new PropertiesModel(control, sharedData->GetDocument()->GetCommandExecutor()));
                break;
            }
            else
            {
                styleSheet = dynamic_cast<StyleSheetNode*>(node);
                if (styleSheet)
                {
                    treeView->setModel(new PropertiesModel(styleSheet, sharedData->GetDocument()->GetCommandExecutor()));
                    break;
                }
                else
                {
                    treeView->setModel(nullptr);
                }
            }
        }
        
        treeView->expandToDepth(0);
        treeView->resizeColumnToContents(0);
    }
    
    addComponentAction->setEnabled(control != nullptr);
    removeComponentAction->setEnabled(false);
    
    addStylePropertyAction->setEnabled(styleSheet != nullptr);
    removeStylePropertyAction->setEnabled(false);
    
    addStyleSelectorAction->setEnabled(styleSheet != nullptr);
    removeStyleSelectorAction->setEnabled(false);
    
    if (treeView->model() != nullptr)
    {
        connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PropertiesWidget::OnSelectionChanged);
    }
    
    delete prevModel;
}

void PropertiesWidget::UpdateActions()
{
    QModelIndexList indices = treeView->selectionModel()->selectedIndexes();
    if (!indices.empty())
    {
        const QModelIndex &index = indices.first();
        AbstractProperty *property = static_cast<AbstractProperty*>(index.internalPointer());
        
        bool removeComponentEnabled = false;
        bool removeStylePropertyEnabled = false;
        bool removeSelectorEnabled = false;
        
        if ((property->GetFlags() & AbstractProperty::EF_CAN_REMOVE) != 0)
        {
            ComponentPropertiesSection *section = dynamic_cast<ComponentPropertiesSection*>(property);
            if (section)
            {
                selectedComponentType = static_cast<int>(section->GetComponentType());
                selectedComponentIndex = static_cast<int>(section->GetComponentIndex());
                removeComponentEnabled = true;
            }
            else
            {
                StyleSheetProperty *styleProperty = dynamic_cast<StyleSheetProperty*>(property);
                if (styleProperty)
                {
                    removeStylePropertyEnabled = true;
                    selectedStylePropertyIndex = styleProperty->GetPropertyIndex();
                }
                else
                {
                    StyleSheetSelectorProperty *selectorProperty = dynamic_cast<StyleSheetSelectorProperty*>(property);
                    if (selectorProperty)
                    {
                        removeSelectorEnabled = true;
                    }
                }
            }
        }
        
        removeComponentAction->setEnabled(removeComponentEnabled);
        removeStylePropertyAction->setEnabled(removeStylePropertyEnabled);
        removeStyleSelectorAction->setEnabled(removeSelectorEnabled);
    }
}
