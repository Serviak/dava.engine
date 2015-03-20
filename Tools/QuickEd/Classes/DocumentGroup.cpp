#include "DocumentGroup.h"
#include <QObject>
#include "Model/PackageHierarchy/ControlNode.h"
#include "Document.h"
#include <QUndoGroup>
#include <QAbstractItemModel>

class DocumentGroupPrivate
{
    Q_DECLARE_PUBLIC(DocumentGroup)
    DocumentGroup* const q_ptr;

public:
    DocumentGroupPrivate(DocumentGroup &object);
    Document *active;
    QList<Document*> documentList;
    QUndoGroup undoGroup;
};

DocumentGroupPrivate::DocumentGroupPrivate(DocumentGroup &object)
    : q_ptr(&object)
    , active(nullptr)
    , undoGroup(&object)
{

}

DocumentGroup::DocumentGroup(QObject *parent) 
    : QObject(parent)
    , d_ptr(new DocumentGroupPrivate(*this))
{
}

DocumentGroup::~DocumentGroup()
{
}

void DocumentGroup::AddDocument(Document* document)
{
    Q_D(DocumentGroup);
    Q_ASSERT(document);
    d->undoGroup.addStack(document->GetUndoStack());
    if (d->documentList.contains(document))
    {
        return;
    }
    d->documentList.append(document);
}

void DocumentGroup::RemoveDocument(Document* document)
{
    Q_D(DocumentGroup);
    Q_ASSERT(document);
    d->undoGroup.removeStack(document->GetUndoStack());

    if (0 == d->documentList.removeAll(document))
    {
        return;
    }
    if (document == d->active)
    {
        SetActiveDocument(nullptr);
    }
}

QList<Document*> DocumentGroup::GetDocuments() const
{
    Q_D(const DocumentGroup);
    return d->documentList;
}

void DocumentGroup::SetActiveDocument(Document* document)
{
    Q_D(DocumentGroup);
    if (d->active == document)
    {
        return;
    }
    if (nullptr != d->active) 
    {
        disconnect(d->active, &Document::LibraryDataChanged, this, &DocumentGroup::LibraryDataChanged);
        disconnect(d->active, &Document::PropertiesDataChanged, this, &DocumentGroup::PropertiesDataChanged);
        disconnect(d->active, &Document::Packag)

        disconnect(d->active, &Document::controlSelectedInEditor, this, &DocumentGroup::controlSelectedInEditor);
        disconnect(d->active, &Document::allControlsDeselectedInEditor, this, &DocumentGroup::allControlsDeselectedInEditor);

        disconnect(this, &DocumentGroup::OnSelectionControlChanged, d->active, &Document::OnSelectionControlChanged);
        disconnect(this, &DocumentGroup::OnSelectionRootControlChanged, d->active, &Document::OnSelectionRootControlChanged);

    }
    
    d->active = document;

    if (nullptr == d->active)
    {
        emit LibraryContextChanged(nullptr);
        emit PropertiesContextChanged(nullptr);
        //
        //!!check that is actual
        emit allControlsDeselectedInEditor();
        d->undoGroup.setActiveStack(nullptr);
    }
    else
    {
        emit LibraryContextChanged(d->active->GetLibraryContext());
        emit PropertiesContextChanged(d->active->GetPropertiesContext());
        //

        connect(d->active, &Document::LibraryDataChanged, this, &DocumentGroup::LibraryDataChanged);
        connect(d->active, &Document::PropertiesDataChanged, this, &DocumentGroup::PropertiesDataChanged);

        connect(d->active, &Document::controlSelectedInEditor, this, &DocumentGroup::controlSelectedInEditor);
        connect(d->active, &Document::allControlsDeselectedInEditor, this, &DocumentGroup::allControlsDeselectedInEditor);

        connect(this, &DocumentGroup::OnSelectionControlChanged, d->active, &Document::OnSelectionControlChanged);
        connect(this, &DocumentGroup::OnSelectionRootControlChanged, d->active, &Document::OnSelectionRootControlChanged);

        d->undoGroup.setActiveStack(d->active->GetUndoStack());
    }
    emit ActiveDocumentChanged(document);
}

Document *DocumentGroup::GetActiveDocument() const
{
    Q_D(const DocumentGroup);
    return d->active;
}

const QUndoGroup& DocumentGroup::GetUndoGroup() const
{
    Q_D(const DocumentGroup);
    return d->undoGroup;
}
