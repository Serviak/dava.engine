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

#include "Document.h"
#include "DocumentGroup.h"
#include <QUndoGroup>
#include "Debug/DVAssert.h"

DocumentGroup::DocumentGroup(QObject* parent)
    : QObject(parent)
    , active(nullptr)
    , undoGroup(new QUndoGroup(this))
{
}

DocumentGroup::~DocumentGroup()
{
}

void DocumentGroup::InsertDocument(int index, Document* document)
{
    DVASSERT(nullptr != document);
    undoGroup->addStack(document->GetUndoStack());
    if (documentList.contains(document))
    {
        DVASSERT(false && "document already exists in document group");
        return;
    }
    documentList.insert(index, document);
}

void DocumentGroup::RemoveDocument(Document* document)
{
    DVASSERT(nullptr != document);
    undoGroup->removeStack(document->GetUndoStack());

    if (0 == documentList.removeAll(document))
    {
        return;
    }
    if (document == active)
    {
        SetActiveDocument(nullptr);
    }
}

QList<Document*> DocumentGroup::GetDocuments() const
{
    return documentList;
}

void DocumentGroup::SetActiveDocument(Document* document)
{
    if (active == document)
    {
        return;
    }

    active = document;

    if (nullptr == active)
    {
        undoGroup->setActiveStack(nullptr);
    }
    else
    {
        undoGroup->setActiveStack(active->GetUndoStack());
    }
    emit ActiveDocumentChanged(document);
}


Document* DocumentGroup::GetActiveDocument() const
{
    return active;
}

const QUndoGroup* DocumentGroup::GetUndoGroup() const
{
    return undoGroup;
}
