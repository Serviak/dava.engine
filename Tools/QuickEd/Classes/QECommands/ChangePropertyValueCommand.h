#ifndef __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__
#define __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__

#include "Document/CommandsBase/Command.h"
#include "FileSystem/VariantType.h"
#include "EditorSystems/EditorSystemsManager.h"

class PackageNode;
class ControlNode;
class AbstractProperty;

class ChangePropertyValueCommand : public QECommand
{
public:
    ChangePropertyValueCommand(PackageNode* _root, const DAVA::Vector<ChangePropertyAction>& propertyActions, size_t hash = 0);

    ChangePropertyValueCommand(PackageNode* _root, ControlNode* _node, AbstractProperty* _property, const DAVA::VariantType& newValue, size_t hash = 0);
    ChangePropertyValueCommand(PackageNode* _root, ControlNode* _node, AbstractProperty* _property);
    virtual ~ChangePropertyValueCommand();

    void Redo() override;
    void Undo() override;

private:
    struct PropertyActionWithOldValue
    {
        PropertyActionWithOldValue(ControlNode* node_, AbstractProperty* property_, const DAVA::VariantType& oldValue_, const DAVA::VariantType& newValue_)
            : node(node_)
            , property(property_)
            , oldValue(oldValue_)
            , newValue(newValue_)
        {
        }
        ControlNode* node = nullptr;
        AbstractProperty* property = nullptr;
        DAVA::VariantType oldValue;
        DAVA::VariantType newValue;
    };
    void Init();
    DAVA::VariantType GetValueFromProperty(AbstractProperty* property);
    PackageNode* root = nullptr;

    DAVA::Vector<PropertyActionWithOldValue> changedProperties;

    size_t hash = 0;
};

#endif // __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__
