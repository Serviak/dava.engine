/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Scene3D/DataNode.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA 
{

REGISTER_CLASS(DataNode);
    
DataNode::DataNode(Scene * _scene)
:   scene(_scene)
,   index(-1)
{
    
}

DataNode::~DataNode()
{
    
}
void DataNode::SetName(const String & _name)
{
    name = _name;
}
    
const String & DataNode::GetName()
{
    return name;
}

void DataNode::AddNode(DataNode * node)
{
    if (node)
    {
        node->Retain();
        node->index = children.size();
        children.push_back(node);
        //node->SetParent(this);
    }
}

DataNode *	DataNode::FindByName(const String & searchName)
{
    if (name == searchName)
        return this;
    
    uint32 size = (uint32)children.size();
    for (uint32 c = 0; c < size; ++c)
    {
        DataNode * res = children[c]->FindByName(searchName);
        if (res != 0)return res;
    }
    return 0;
}

void DataNode::RemoveNode(DataNode * node)
{
    if (!node) 
    {
        return;
    }
//    if (inUpdate) 
//    {
//        removedCache.push_back(node);
//        return;
//    }
    const std::vector<DataNode*>::iterator & childrenEnd = children.end();
    for (std::vector<DataNode*>::iterator t = children.begin(); t != childrenEnd; ++t)
    {
        if (*t == node)
        {
            children.erase(t);
            if (node)
            {
                //node->SetParent(0);
                node->index = -1;
                node->Release();
            }
            break;
        }
    }
    
}

DataNode * DataNode::GetChild(int32 index)
{
    return children[index];
}

int32 DataNode::GetChildrenCount()
{
    return (int32)children.size();
}

void DataNode::RemoveAllChildren()
{
    for (std::vector<DataNode*>::iterator t = children.begin(); t != children.end(); ++t)
    {
        DataNode *node = *t;
        //node->SetParent(0);
        node->index = -1;
        node->Release();
    }
    children.clear();
}

int32  DataNode::GetNodeIndex()
{
    return index;
}
    
void DataNode::Load(KeyedArchive * archive)
{
    archive->SetInt32("#index", index);
}

void DataNode::Save(KeyedArchive * archive)
{
    index = archive->GetInt32("#index", -1);
}



}