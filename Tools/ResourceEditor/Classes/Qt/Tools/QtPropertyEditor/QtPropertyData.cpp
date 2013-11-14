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



#include <QPushButton>
#include "QtPropertyData.h"

QtPropertyData::QtPropertyData()
	: curFlags(0)
	, parent(NULL)
	, optionalWidgetViewport(NULL)
	, updatingValue(false)
	, model(NULL)
{ }

QtPropertyData::QtPropertyData(const QVariant &value, Qt::ItemFlags flags)
	: curValue(value)
	, curFlags(flags)
	, parent(NULL)
	, optionalWidgetViewport(NULL)
	, updatingValue(false)
	, model(NULL)
{ }

QtPropertyData::~QtPropertyData() 
{
	for(int i = 0; i < childrenData.size(); ++i)
	{
		QtPropertyData *data = childrenData.at(i);
		if(NULL != data)
		{
			delete data;
		}
	}

	for (int i = 0; i < optionalWidgets.size(); i++)
	{
		if(NULL != optionalWidgets.at(i).widget)
		{
			delete optionalWidgets.at(i).widget;
		}
	}
}

QVariant QtPropertyData::GetValue() const
{
	if(curValue.isNull() || !curValue.isValid())
	{
		curValue = GetValueInternal();
	}
	else
	{
		const_cast<QtPropertyData*>(this)->UpdateValue();
	}

	return curValue;
}

void QtPropertyData::SetValue(const QVariant &value, ValueChangeReason reason)
{
	QVariant oldValue = curValue;

	curValue = value;
	SetValueInternal(curValue);

	if(curValue != oldValue)
	{
		UpdateDown();
		UpdateUp();

		EmitDataChanged(reason);
	}
}

bool QtPropertyData::UpdateValue()
{
	bool ret = false;

	if(!updatingValue)
	{
		updatingValue = true;

		if(UpdateValueInternal())
		{
			curValue = GetValueInternal();
			EmitDataChanged(VALUE_SOURCE_CHANGED);

			ret = true;
		}

		updatingValue = false;
	}

	return ret;
}

QVariant QtPropertyData::GetAlias() const
{
	// this will force update internalValue if 
	// it source was changed 
	GetValue();

	return GetValueAlias();
}

void QtPropertyData::SetName(const QString &_name)
{
	name = _name;
}

QString QtPropertyData::GetName() const
{
	return name;
}

void QtPropertyData::SetIcon(const QIcon &icon)
{
	curIcon = icon;
}

QIcon QtPropertyData::GetIcon() const
{
	return curIcon;
}

Qt::ItemFlags QtPropertyData::GetFlags() const
{
	return curFlags;
}

void QtPropertyData::SetFlags(Qt::ItemFlags flags)
{
	curFlags = flags;
}

void QtPropertyData::SetCheckable(bool checkable)
{
	(checkable) ? (curFlags |= Qt::ItemIsUserCheckable) : (curFlags &= ~Qt::ItemIsUserCheckable);
}

bool QtPropertyData::IsCheckable() const
{
	return (curFlags & Qt::ItemIsUserCheckable);
}

void QtPropertyData::SetChecked(bool checked)
{
	// TODO:
	// ...
}

bool QtPropertyData::IsChecked() const
{
	// TODO:
	// ...

	return false;
}

void QtPropertyData::SetEditable(bool editable)
{
	(editable) ? (curFlags |= Qt::ItemIsEditable) : (curFlags &= ~Qt::ItemIsEditable);
}

bool QtPropertyData::IsEditable() const
{
	return (curFlags & Qt::ItemIsEditable);
}

void QtPropertyData::SetEnabled(bool enabled)
{
	(enabled) ? (curFlags |= Qt::ItemIsEnabled) : (curFlags &= ~Qt::ItemIsEnabled);
}

bool QtPropertyData::IsEnabled() const
{
	return (curFlags & Qt::ItemIsEnabled);
}

QtPropertyModel* QtPropertyData::GetModel() const
{
	return model;
}

QWidget* QtPropertyData::CreateEditor(QWidget *parent, const QStyleOptionViewItem& option) const
{ 
	return CreateEditorInternal(parent, option);
}

bool QtPropertyData::EditorDone(QWidget *editor)
{
    return EditorDoneInternal(editor);
}

bool QtPropertyData::SetEditorData(QWidget *editor)
{
    return SetEditorDataInternal(editor);
}

void QtPropertyData::EmitDataChanged(ValueChangeReason reason)
{
	if(NULL != model)
	{
		model->DataChanged(this, reason);
	}
}

void QtPropertyData::UpdateUp()
{
	if(NULL != parent)
	{
		parent->UpdateValue();
		parent->UpdateUp();
	}
}

void QtPropertyData::UpdateDown()
{
	for(int i = 0; i < childrenData.size(); ++i)
	{
		QtPropertyData *child = childrenData.at(i);
		if(NULL != child)
		{
			child->UpdateValue();
			child->UpdateDown();
		}
	}
}

QtPropertyData* QtPropertyData::Parent() const
{
	return parent;
}

void QtPropertyData::ChildAdd(const QString &key, QtPropertyData *data)
{
	if(NULL != data && !key.isEmpty())
	{
		childrenData.append(data);
		childrenNames.append(key);
		data->parent = this;
		data->model = model;
	}
}

void QtPropertyData::ChildAdd(const QString &key, const QVariant &value)
{
	ChildAdd(key, new QtPropertyData(value));
}

int QtPropertyData::ChildCount() const
{
	return childrenData.size();
}

QtPropertyData* QtPropertyData::ChildGet(int i) const
{
	QtPropertyData *ret = NULL;

	if(i >= 0 && i < childrenData.size())
	{
		ret = childrenData.at(i);
	}

	return ret;
}

QtPropertyData* QtPropertyData::ChildGet(const QString &key) const
{
	QtPropertyData *data = NULL;

	int index = childrenNames.indexOf(key);
	if(-1 != index)
	{
		data = childrenData.at(index);
	}

	return data;
}

int QtPropertyData::ChildIndex(QtPropertyData *data) const
{
	return childrenData.indexOf(data);
}

void QtPropertyData::ChildRemove(QtPropertyData *data)
{
	int index = childrenData.indexOf(data);
	ChildRemove(index);
}

void QtPropertyData::ChildRemove(const QString &key)
{
	int index = childrenNames.indexOf(key);
	ChildRemove(index);
}

void QtPropertyData::ChildRemove(int index)
{
	if(index >= 0 && index < childrenData.size())
	{
		QtPropertyData *data = childrenData.at(index);
		childrenData.removeAt(index);
		childrenNames.removeAt(index);

		delete data;
		data = NULL;
	}
}

int QtPropertyData::GetOWCount() const
{
	return optionalWidgets.size();
}

const QtPropertyOW* QtPropertyData::GetOW(int index)
{
	const QtPropertyOW *ret = NULL;

	if(index >= 0 && index < optionalWidgets.size())
	{
		ret = &optionalWidgets.at(index);
	}

	return ret;
}

void QtPropertyData::AddOW(const QtPropertyOW &ow)
{
	optionalWidgets.append(ow);

	if(NULL != ow.widget)
	{
		ow.widget->setParent(optionalWidgetViewport);
		ow.widget->hide();
	}
}

void QtPropertyData::RemOW(int index)
{
	if(index >= 0 && index < optionalWidgets.size())
	{
		if(NULL != optionalWidgets.at(index).widget)
		{
			delete optionalWidgets.at(index).widget;
		}

		optionalWidgets.remove(index);
	}
}

void QtPropertyData::RemOW(QWidget *widget)
{
	for(int i = 0; i < optionalWidgets.size(); ++i)
	{
		if(optionalWidgets[i].widget == widget)
		{
			RemOW(i);
			break;
		}
	}
}

QWidget* QtPropertyData::GetOWViewport() const
{
	return optionalWidgetViewport;
}

void QtPropertyData::SetOWViewport(QWidget *viewport)
{
	optionalWidgetViewport = viewport;

	for(int i = 0; i < optionalWidgets.size(); ++i)
	{
		if(NULL != optionalWidgets.at(i).widget)
		{
			optionalWidgets.at(i).widget->setParent(viewport);
		}
	}

	
	for (int i = 0; i < childrenData.size(); i++)
	{
		childrenData.at(i)->SetOWViewport(viewport);
	}
}

void* QtPropertyData::CreateLastCommand() const
{
	// can be re-implemented by sub-class

	return NULL;
}

QVariant QtPropertyData::GetValueInternal() const
{
	// should be re-implemented by sub-class

	return curValue;
}

bool QtPropertyData::UpdateValueInternal()
{
	return false;
}

QVariant QtPropertyData::GetValueAlias() const
{
	// should be re-implemented by sub-class

	return QVariant();
}

void QtPropertyData::SetValueInternal(const QVariant &value)
{
	// should be re-implemented by sub-class

	curValue = value;
}

QWidget* QtPropertyData::CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option) const
{
	// should be re-implemented by sub-class

	return NULL;
}

bool QtPropertyData::EditorDoneInternal(QWidget *editor)
{
	// should be re-implemented by sub-class
	return false;
}

bool QtPropertyData::SetEditorDataInternal(QWidget *editor)
{
	// should be re-implemented by sub-class
	return false;
}
