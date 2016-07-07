#ifndef __META_OBJ_MODIFY_COMMAND_H__
#define __META_OBJ_MODIFY_COMMAND_H__

#include "Commands2/Base/RECommand.h"

class MetaObjModifyCommand : public RECommand
{
public:
    MetaObjModifyCommand(const DAVA::MetaInfo* info, void* object, const DAVA::VariantType& value);
    ~MetaObjModifyCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    };

    const DAVA::MetaInfo* info;
    void* object;

    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};

#endif // __META_OBJ_MODIFY_COMMAND_H__
