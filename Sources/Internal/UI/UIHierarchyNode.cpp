/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "UIHierarchyNode.h"

namespace DAVA
{
UIHierarchyNode::UIHierarchyNode(void *_userNode)
{
    userNode = _userNode;
    isOpen = false;
    openedChildrenCount = 0;
    parent = NULL;
    nodeLevel = 0;
}

UIHierarchyNode::~UIHierarchyNode()
{
    DeleteChildren();
}

void UIHierarchyNode::AddChild(UIHierarchyNode *child)
{
    if (child->parent) 
    {
        child->parent->RemoveChild(child);
    }
    children.push_back(child);
    child->parent = this;
}

void UIHierarchyNode::RemoveChild(UIHierarchyNode *child)
{
    for (List<UIHierarchyNode *>::iterator it = children.begin(); it != children.end(); it++) 
    {
        if ((*it) == child) 
        {
            children.erase(it);
            child->parent = NULL;
            return;
        }
    }
}

void UIHierarchyNode::DeleteChildren()
{
    for (List<UIHierarchyNode *>::iterator it = children.begin(); it != children.end(); it++) 
    {
        (*it)->DeleteChildren();
        delete (*it);
    }
    children.clear();
}

UIHierarchyNode *UIHierarchyNode::CheckChildrenForUserNode(void *userNodePtr)
{
    for (List<UIHierarchyNode *>::iterator it = children.begin(); it != children.end(); it++) 
    {
        if ((*it)->userNode == userNodePtr) 
        {
            return (*it);
        }
    }
    return NULL;
}
    
void * UIHierarchyNode::GetUserNode()
{
    return userNode;
}



};