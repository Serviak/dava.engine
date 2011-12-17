/*
 *  TestScreen.cpp
 *  TemplateProjectMacOS
 *
 *  Created by Vitaliy  Borodovsky on 3/21/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "UIFileTree.h"
#include "UIFileTreeCell.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "Utils/Utils.h"
#include "Base/ObjectFactory.h"

namespace DAVA 
{
	
REGISTER_CLASS(UIFileTree);
	

UIFileTree::UIFileTree(const Rect &rect, bool rectInAbsoluteCoordinates)
	: UIList(rect, UIList::ORIENTATION_VERTICAL, rectInAbsoluteCoordinates)
{
	treeHead = 0;
	delegate = 0;
	isFolderNavigationEnabled = false;
    isRootFolderChangeEnabled = true;
    isRootFolderExpandingDisabled = false;
}

UIFileTree::~UIFileTree()
{
	SafeRelease(treeHead);
}

void UIFileTree::SetPath(const String & fullpath, const String & extensionsString)
{
	originalExtensionsString = extensionsString;
	Split(extensionsString, ";", extensions);
	
	SafeRelease(treeHead);
	
	String path, dirname;
	FileSystem::SplitPath(fullpath, path, dirname);

	treeHead = new UITreeItemInfo(this);
	treeHead->Set(0, dirname, fullpath, true);
	treeHead->isExpanded = true;
    
	RecursiveTreeWalk(fullpath, treeHead);
	
	UIList::SetDelegate(this);
	Refresh();
}

	
int32 UIFileTree::ElementsCount(UIList *forList)
{
	return treeHead->GetTotalCount();
}

UIListCell *UIFileTree::CellAtIndex(UIList *forList, int32 index)
{
    int32 width = forList->GetRect().dx;
    
	UIFileTreeCell *c = (UIFileTreeCell *)forList->GetReusableCell("FileTreeCell"); //try to get cell from the reusable cells store
	if(!c)
	{ //if cell of requested type isn't find in the store create new cell
//		c = new UIFileTreeCell(Rect(0, 0, 200, 20), "FileTreeCell");
		c = new UIFileTreeCell(Rect(0, 0, width, 20), "FileTreeCell");
	}
	//fill cell whith data
	//c->serverName = GameServer::Instance()->totalServers[index].name + LocalizedString("'s game");

	UITreeItemInfo * entry = treeHead->EntryByIndex(index);

//	String empty;
//	for (int k = 0; k < entry->GetLevel(); ++k)
//	{
//		empty += ' ';
//		empty += ' ';
//	}
	float32 shiftX = entry->GetLevel() * 10.0f;
//	c->SetRect(Rect(shiftX, 0, 200 - shiftX, 16));
	c->SetRect(Rect(shiftX, 0, width - shiftX, 16));
	c->SetStateText(UIControl::STATE_NORMAL, StringToWString(entry->GetName()));
    c->GetStateTextControl(UIControl::STATE_NORMAL)->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    c->SetStateText(UIControl::STATE_SELECTED, StringToWString(entry->GetName()));
	c->GetStateTextControl(UIControl::STATE_SELECTED)->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);

    
	c->SetItemInfo(entry);
	
	/*
		WTF ??? I can't call RemoveAllEvents here.
	 */
	c->RemoveEvent(UIControl::EVENT_TOUCH_DOWN, Message(this, &UIFileTree::OnDirectoryChange));
	
	if (entry->IsDirectory())
		c->AddEvent(UIControl::EVENT_TOUCH_DOWN, Message(this, &UIFileTree::OnDirectoryChange));
	
	//c->connection = GameServer::Instance()->totalServers[index].connection;
	//c->serverIndex = GameServer::Instance()->totalServers[index].index;

	return c;//returns cell
	//your application don't need to manage cells. UIList do all cells management.
	//you can create cells of your own types derived from the UIListCell
}
	
void UIFileTree::OnDirectoryChange(BaseObject * obj, void * userData, void * callerData)
{
	UIFileTreeCell * cell = dynamic_cast<UIFileTreeCell*> (obj);
	UIEvent * event = reinterpret_cast<UIEvent*> (callerData);	
	if (cell && event)
	{
		Logger::Debug("Click count: %d", event->tapCount);
		if (isRootFolderChangeEnabled && (event->tapCount == 2))
		{
			UITreeItemInfo * info = cell->GetItemInfo();
			String pathname = info->GetPathname();
			Logger::Debug("Switch to path: %s", pathname.c_str());
			SetPath(FileSystem::RealPath(pathname), originalExtensionsString);
			treeHead->ToggleExpanded();
		}
	}
}


int32 UIFileTree::CellWidth(UIList *forList, int32 index)
{
	return 20; //rect.dx;
}
	
int32 UIFileTree::CellHeight(UIList *forList, int32 index)
{
    if(delegate)
    {
        return delegate->CellHeight(forList, index);
    }
    return 16;
}

    
void UIFileTree::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    int32 index = selectedCell->GetIndex();
    
	UITreeItemInfo * entry = treeHead->EntryByIndex(index);
    
    if(!(isRootFolderExpandingDisabled && 0 == index))
    {
        entry->ToggleExpanded();
    }
	
	if (delegate)
		delegate->OnCellSelected(this, dynamic_cast<UIFileTreeCell*>(selectedCell));
	
	Refresh();
};

	
void UIFileTree::RecursiveTreeWalk(const String & path, UITreeItemInfo * current)
{
	FileList * fileList = new FileList(path);
	
	// Find flags and setup them
	for (int fi = 0; fi < fileList->GetCount(); ++fi)
	{
		bool addElement = true;
		if (!fileList->IsDirectory(fi))
		{
			size_t extsSize = extensions.size();
			if (extsSize > 0)
			{
				addElement = false;
				String ext = FileSystem::GetExtension(fileList->GetFilename(fi));
				for (size_t ei = 0; ei < extsSize; ++ei)
                {
                    if(0 == CompareExtensions(extensions[ei], ext))
					{
						addElement = true;
						break;
					}
                }
			}
		}
		if (!isFolderNavigationEnabled)
			if (fileList->IsNavigationDirectory(fi))
				addElement = false;

		if (fileList->GetFilename(fi) == ".")
			addElement = false;

		if (addElement)
		{
			UITreeItemInfo *child = new UITreeItemInfo(this);
			child->Set(current->GetLevel() + 1, fileList->GetFilename(fi), fileList->GetPathname(fi), fileList->IsDirectory(fi));
			current->AddChild(child);

//			if (fileList->IsDirectory(fi) )
//			{
//				if (!fileList->IsNavigationDirectory(fi))
//				{
//					RecursiveTreeWalk(path + String("/") + fileList->GetFilename(fi), child);
//				}
//			}
		}
	}
	SafeRelease(fileList);
}

	
void UITreeItemInfo::ToggleExpanded()
{
	if ((GetName() == "..") || (GetName() == "."))return;
	isExpanded = !isExpanded;
	if (isExpanded)
	{
		RemoveChildren();
		ownerTree->RecursiveTreeWalk(this->GetPathname(), this);
	}
}

void UITreeItemInfo::RemoveChildren()
{
	for (int32 k = 0; k < (int32) children.size(); ++k) 
		SafeRelease(children[k]);
	children.clear();	
}
	
int32 UITreeItemInfo::GetTotalCount()
{
	int32 cnt = 0;
	if (isExpanded)
	{
        if(ownerTree)
        {
            UITreeItemInfo *inf = new UITreeItemInfo(ownerTree);
            inf->level = level;
            inf->name = GetName();
            inf->pathname = GetPathname();
            inf->isDirectory = isDirectory;
            inf->isExpanded = isExpanded;
            
            ownerTree->RecursiveTreeWalk(this->GetPathname(), inf);
            
            //remove unused
            for (Vector<UITreeItemInfo *>::iterator it = children.begin(); 
                 it < children.end(); )
            {
                UITreeItemInfo *item = (UITreeItemInfo *)(*it);
                bool wasFound = false;
                for (int32 iNew = 0; iNew < (int32)inf->children.size(); ++iNew)
                {
                    if(item->name == inf->children[iNew]->name)
                    {
                        wasFound = true;
                        break;
                    }
                }
                
                if(!wasFound)
                {
                    SafeRelease(item);
                    children.erase(it);
                    it = children.begin();
                }
                else
                {
                    ++it;
                }
            }
            //add new
            for (int32 iNew = 0; iNew < (int32)inf->children.size(); ++iNew)
            {
                bool wasFound = false;
                for (int32 iOld = 0; iOld < (int32)children.size(); ++iOld)
                {
                    if(children[iOld]->name == inf->children[iNew]->name)
                    {
                        wasFound = true;
                        break;
                    }
                }
                
                if(!wasFound)
                {
                    children.push_back(SafeRetain(inf->children[iNew]));
                }
            }
            
            SafeRelease(inf);
        }        
        
		for (int32 c = 0; c < (int32)children.size(); ++c)
			cnt += children[c]->GetTotalCount();
	}
	return cnt + 1;
}
	
UITreeItemInfo * UITreeItemInfo::EntryByIndex(int32 index)
{
	int32 cnt = 0;
	if (index == 0)return this;
	index--;
	if (isExpanded)
	{
		for (int32 c = 0; c < (int32)children.size(); ++c)
		{
			int32 childCnt = children[c]->GetTotalCount();
			if ((index >= cnt) && (index < cnt + childCnt))
				return children[c]->EntryByIndex(index - cnt);
			cnt += childCnt;
		}
	}
	return 0;
}
	
void UIFileTree::SetDelegate(UIFileTreeDelegate * _delegate)
{
	delegate = _delegate;
}
	
void UIFileTree::SetFolderNavigation(bool isEnabled)
{
	isFolderNavigationEnabled = isEnabled;
}
				
int32 UIFileTree::CompareExtensions(const String &ext1, const String &ext2)
{
    String newExt1 = "";
    newExt1.resize(ext1.length());
    std::transform(ext1.begin(), ext1.end(), newExt1.begin(), ::tolower);

    String newExt2 = "";
    newExt2.resize(ext2.length());
    std::transform(ext2.begin(), ext2.end(), newExt2.begin(), ::tolower);

    if(newExt1 == newExt2)
    {
        return 0;   
    }
    else if(newExt1 < newExt2)
    {
        return -1;
    }
    
    return 1;
}

void UIFileTree::Refresh()
{
    UIList::Refresh();
}
    
void UIFileTree::EnableRootFolderChange(bool isEnabled)
{
    isRootFolderChangeEnabled = isEnabled;
}
    
void UIFileTree::DisableRootFolderExpanding(bool isDisabled)
{
    isRootFolderExpandingDisabled = isDisabled;
}
    
    
};

