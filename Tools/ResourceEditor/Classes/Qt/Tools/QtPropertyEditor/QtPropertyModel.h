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


#ifndef __QT_PROPERTY_MODEL_H__
#define __QT_PROPERTY_MODEL_H__

#include "Base/Introspection.h"
#include "QtPropertyData.h"

#include <QAbstractItemModel>
#include <QEvent>

class QtPropertyModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	QtPropertyModel(QWidget *_viewport, QObject* parent = 0);
	~QtPropertyModel();

	QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex & index) const;
	int rowCount(const QModelIndex & parent = QModelIndex()) const;
	int columnCount(const QModelIndex & parent = QModelIndex()) const;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
	Qt::ItemFlags flags(const QModelIndex & index) const;
    bool event(QEvent * e);

	QtPropertyData* rootItem() const;

	QtPropertyData* itemFromIndex(const QModelIndex & index) const;
	QModelIndex indexFromItem(QtPropertyData *data) const;

    void AppendProperties(DAVA::Vector<TPropertyPtr> && properties, const QModelIndex& parent = QModelIndex());
    QModelIndex AppendProperty(TPropertyPtr && data, const QModelIndex &parent = QModelIndex());
    void MergeProperty(TPropertyPtr && data, const QModelIndex &parent = QModelIndex());
    QModelIndex InsertProperty(TPropertyPtr && data, int row, const QModelIndex &parent = QModelIndex());

	bool GetEditTracking();
	void SetEditTracking(bool enabled);

	void RemoveProperty(const QModelIndex &index);
	void RemovePropertyAll();

	void UpdateStructure(const QModelIndex &parent = QModelIndex());
    void FinishTreeCreation();

signals:
	void PropertyEdited(const QModelIndex &index);

protected:
    enum
    {
        DataRefreshRequared = QEvent::User + 1
    };

    friend class QtPropertyData;

	QtPropertyData *root;
	bool trackEdit;
    bool needRefresh;

    class InsertionGuard
    {
    public:
        InsertionGuard(QtPropertyModel* model_, QtPropertyData * parent, int first, int last)
            : model(model_)
        {
            if (model != nullptr)
                model->DataAboutToBeAdded(parent, first, last);
        }

        ~InsertionGuard()
        {
            if (model != nullptr)
                model->DataAdded();
        }

    private:
        QtPropertyModel* model;
    };

    class DeletionGuard
    {
    public:
        DeletionGuard(QtPropertyModel * model_, QtPropertyData * parent, int first, int last)
            : model(model_)
        {
            if (model != nullptr)
                model->DataAboutToBeRemoved(parent, first, last);
        }

        ~DeletionGuard()
        {
            if (model != nullptr)
                model->DataRemoved();
        }

    private:
        QtPropertyModel * model;
    };

	QtPropertyData *itemFromIndexInternal(const QModelIndex & index) const;

	void DataChanged(QtPropertyData *data, int reason);
	void DataAboutToBeAdded(QtPropertyData *parent, int first, int last);
	void DataAdded();
	void DataAboutToBeRemoved(QtPropertyData *parent, int first, int last);
	void DataRemoved();

	void UpdateStructureInternal(const QModelIndex &index);
};

#endif // __QT_PROPERTY_MODEL_H__
