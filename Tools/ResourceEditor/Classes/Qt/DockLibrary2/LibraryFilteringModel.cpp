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



#include "LibraryFilteringModel.h"

#include <QFileSystemModel>

LibraryFilteringModel::LibraryFilteringModel(QObject *parent /* = NULL */)
    : QSortFilterProxyModel(parent)
    , model(NULL)
{
}

void LibraryFilteringModel::SetModel(QAbstractItemModel *newModel)
{
    model = newModel;
    setSourceModel(model);
}


void LibraryFilteringModel::SetSourceRoot( const QModelIndex &root )
{
	sourceRoot = root;
}


bool LibraryFilteringModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if(model == NULL) return true;

	// First we see if we're the source root node
	QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
	if (!sourceIndex.isValid() || !sourceParent.isValid())
		return true; // viewer will handle filtering

	// Make sure the root is always accepted, or we become rootless
	// See http://stackoverflow.com/questions/3212392/qtreeview-qfilesystemmodel-setrootpath-and-qsortfilterproxymodel-with-regexp-fo
	if (sourceRoot.isValid() && sourceIndex == sourceRoot) 
		return true; // true root, always accept

	// filter only items at parent
	if(sourceParent == sourceRoot)
	{
		QString data = sourceModel()->data(sourceIndex).toString();
		return (data.contains(filterRegExp()));
	}

	return true; // nothing matched
}


