#ifndef __GenericTreeNodeS_H__
#define __GenericTreeNodeS_H__

#include <QVariant>

#include "Base/BaseTypes.h"

class GenericTreeNode
{
public:
    GenericTreeNode();
    virtual ~GenericTreeNode() = default;

    virtual GenericTreeNode* Parent() const;
    virtual int Index() const;
    virtual int ChildrenCount() const;  // Returns int due to using in AllocationTreeModel::rowCount which returns int
    virtual GenericTreeNode* Child(int index) const;
    virtual void AppendChild(GenericTreeNode* child);
    virtual QVariant Data(int row, int clm) const;

protected:
    void SetParent(GenericTreeNode* aParent);
    int ChildIndex(const GenericTreeNode* child) const;

protected:
    GenericTreeNode* parent;
    DAVA::Vector<std::unique_ptr<GenericTreeNode>> children;
};

//////////////////////////////////////////////////////////////////////////
inline GenericTreeNode::GenericTreeNode()
    : parent(nullptr)
{
}

inline GenericTreeNode* GenericTreeNode::Parent() const
{
    return parent;
}

inline int GenericTreeNode::Index() const
{
    if (parent != nullptr)
    {
        return parent->ChildIndex(this);
    }
    return 0;
}

inline int GenericTreeNode::ChildrenCount() const
{
    return static_cast<int>(children.size());
}

inline GenericTreeNode* GenericTreeNode::Child(int index) const
{
    Q_ASSERT(0 <= index && index < ChildrenCount());
    return children[index].get();
}

inline void GenericTreeNode::AppendChild(GenericTreeNode* child)
{
    Q_ASSERT(child != nullptr);
    Q_ASSERT(children.cend() == std::find_if(children.cbegin(), children.cend(), [child](const std::unique_ptr<GenericTreeNode>& o) -> bool { return o.get() == child; }));
    child->SetParent(this);
    children.push_back(std::unique_ptr<GenericTreeNode>(child));
}

inline QVariant GenericTreeNode::Data(int row, int clm) const
{
    return QVariant();
}

inline void GenericTreeNode::SetParent(GenericTreeNode* aParent)
{
    parent = aParent;
}

inline int GenericTreeNode::ChildIndex(const GenericTreeNode* child) const
{
    auto i = std::find_if(children.cbegin(), children.cend(), [child](const std::unique_ptr<GenericTreeNode>& o) -> bool { return o.get() == child; });
    Q_ASSERT(i != children.cend());
    return static_cast<int>(std::distance(children.cbegin(), i));
}

#endif  // __GenericTreeNodeS_H__
