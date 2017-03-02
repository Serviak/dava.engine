#pragma once

#include "TArc/Core/ContextAccessor.h"
#include "TArc/Core/FieldBinder.h"
#include "TArc/DataProcessing/Common.h"
#include "TArc/Utils/QtConnections.h"

#include <QWidget>

class QTreeView;

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyModel;
class ExtensionChain;
class ContextAccessor;
class UI;

class PropertiesView : public QWidget
{
    Q_OBJECT
public:
    /**
        Create PropertiesView widget with ReflectedModel. As data source for ReflectedMode use value of "objectsField"
        Value of "objectsField" could be casted to Vector<Reflection>
    */
    PropertiesView(UI* ui, ContextAccessor* accessor, const FieldDescriptor& objectsField, const String& settingsNodeName);
    ~PropertiesView();

    void RegisterExtension(const std::shared_ptr<ExtensionChain>& extension);
    void UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension);

private:
    void SetupUI();
    void OnObjectsChanged(const Any& objects);
    void OnColumnResized(int columnIndex, int oldSize, int newSize);

    void OnExpanded(const QModelIndex& index);
    void OnCollapsed(const QModelIndex& index);

private:
    FieldBinder binder;
    ContextAccessor* accessor = nullptr;
    QTreeView* view = nullptr;
    std::unique_ptr<ReflectedPropertyModel> model;
    const String settingsNodeName;
    QtConnections connections;
};
}
}