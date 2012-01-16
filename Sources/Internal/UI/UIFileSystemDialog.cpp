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
        * Created by Alexey 'Hottych' Prosin
=====================================================================================*/
#include "UI/UIFileSystemDialog.h"
#include "UI/UIList.h"
#include "UI/UITextField.h"
#include "FileSystem/FileList.h"
#include "Utils/Utils.h"
#include "Core/Core.h"
#include "Platform/SystemTimer.h"
#include <algorithm>

namespace DAVA
{

UIFileSystemDialog::UIFileSystemDialog(const String &_fontPath)
: UIControl(Rect(GetScreenWidth()/2, GetScreenHeight()/2, GetScreenWidth()*2/3, GetScreenHeight()*4/5))
{
    fontPath = _fontPath;
    
    background->SetDrawType(UIControlBackground::DRAW_FILL);
    background->SetColor(Color(0.5, 0.5, 0.5, 0.75));
    pivotPoint = size / 2;
    
    operationType = OPERATION_LOAD;
    delegate = NULL;
    extensionFilter.push_back(".*");
    
    SetCurrentDir(FileSystem::Instance()->GetCurrentWorkingDirectory());
    
    cellH = (int32)GetScreenHeight()/20;
    cellH = Max(cellH, 32);
    int32 border = (int32)GetScreenHeight()/64;
    fileListView = new UIList(Rect((float32)border, (float32)(border + cellH), (float32)(size.x - border*2), (float32)(size.y - cellH*3 - border*3)), UIList::ORIENTATION_VERTICAL);
    fileListView->SetDelegate(this);
    fileListView->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    fileListView->GetBackground()->SetColor(Color(0.25, 0.25, 0.25, 0.25));
    AddControl(fileListView);
    
    lastSelectionTime = 0;
    
    Font *f = FTFont::Create(fontPath);
    f->SetSize((float32)cellH * 2 / 3);
    f->SetColor(Color(1.f, 1.f, 1.f, 1.f));
    
    title = new UIStaticText(Rect((float32)border, (float32)border/2, (float32)size.x - border*2, (float32)cellH));
    title->SetFont(f);
    title->SetFittingOption(TextBlock::FITTING_REDUCE);
    title->SetText(L"Select file:");
    AddControl(title);

    workingPath = new UIStaticText(Rect((float32)border, (float32)border/2 + fileListView->size.y + fileListView->relativePosition.y, (float32)size.x - border*2, (float32)cellH));
    workingPath->SetFont(f);
    workingPath->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);
    workingPath->SetFittingOption(TextBlock::FITTING_REDUCE);
    workingPath->SetText(L"c:");
    AddControl(workingPath);
    
    
    int32 buttonW = cellH * 3;
    positiveButton = new UIButton(Rect((float32)size.x - border - buttonW, (float32)workingPath->relativePosition.y + border/2 + cellH, (float32)buttonW, (float32)cellH));
    positiveButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    positiveButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.5f, 0.6f, 0.5f, 0.5f));
    positiveButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    positiveButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.75f, 0.85f, 0.75f, 0.5f));
    positiveButton->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    positiveButton->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    positiveButton->SetStateFont(UIControl::STATE_NORMAL, f);
    positiveButton->SetStateText(UIControl::STATE_NORMAL, L"OK");
	positiveButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIFileSystemDialog::ButtonPressed));
    AddControl(positiveButton);

    negativeButton = new UIButton(Rect((float32)positiveButton->relativePosition.x - buttonW - border, (float32)positiveButton->relativePosition.y, (float32)buttonW, (float32)cellH));
    negativeButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    negativeButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.6f, 0.5f, 0.5f, 0.5f));
    negativeButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    negativeButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.85f, 0.75f, 0.75f, 0.5f));
    negativeButton->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    negativeButton->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    negativeButton->SetStateFont(UIControl::STATE_NORMAL, f);
    negativeButton->SetStateText(UIControl::STATE_NORMAL, L"Cancel");
	negativeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIFileSystemDialog::ButtonPressed));
    AddControl(negativeButton);
    

    textField = new UITextField(Rect((float32)border, (float32)positiveButton->relativePosition.y, (float32)negativeButton->relativePosition.x - border*2, (float32)cellH));
    textField->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    textField->GetBackground()->SetColor(Color(0.25f, 0.25f, 0.25f, 0.25f));
    textField->SetFont(f);
    textField->SetDelegate(this);
    
    SafeRelease(f);
    
    files = NULL;
}

void UIFileSystemDialog::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
    if (obj == negativeButton) 
    {
        Retain();
        GetParent()->RemoveControl(this);
        if (delegate)
        {
            delegate->OnFileSytemDialogCanceled(this);
        }
        Release();
    }
    else if (obj == positiveButton)
    {
        if (operationType == OPERATION_LOAD)
        {
            OnIndexSelected(lastSelectedIndex);
        }
        else if(operationType == OPERATION_CHOOSE_DIR)
        {
            if (lastSelectedIndex >= 0)
            {
                if (delegate)
                {
                    delegate->OnFileSelected(this, files->GetPathname(fileUnits[lastSelectedIndex].indexInFileList));
                }
            }
            else 
            {
                if (delegate)
                {
                    delegate->OnFileSelected(this, currentDir);
                }
            }

            GetParent()->RemoveControl(this);
        }
        else if(operationType == OPERATION_SAVE)
        {
            SaveFinishing();
        }
            

    }

}

void UIFileSystemDialog::SaveFinishing()
{
    if (!textField->GetText().empty())
    {
        if (delegate)
        {
            if (textField->GetText().find(L".") != textField->GetText().npos)
            {
                delegate->OnFileSelected(this, currentDir + "/" + WStringToString(textField->GetText()));
            }
            else 
            {
                delegate->OnFileSelected(this, currentDir + "/" + WStringToString(textField->GetText()) + extensionFilter[0]);
            }
            
        }
        GetParent()->RemoveControl(this);
    }
}


void UIFileSystemDialog::Show(UIControl *parentControl)
{
    parentControl->AddControl(this);
    RemoveControl(textField);
    switch (operationType) 
    {
        case OPERATION_LOAD:
            positiveButton->SetStateText(UIControl::STATE_NORMAL, L"Load");
            break;
        case OPERATION_SAVE:
            positiveButton->SetStateText(UIControl::STATE_NORMAL, L"Save");
            AddControl(textField);
            break;
        case OPERATION_CHOOSE_DIR:
            positiveButton->SetStateText(UIControl::STATE_NORMAL, L"Choose");
            break;
    }

    RefreshList();
}


void UIFileSystemDialog::SetCurrentDir(const String &newDirPath)
{

    //int32 ppos = newDirPath.rfind(".");
    //int32 spos = newDirPath.rfind("/");
    //if (ppos != newDirPath.npos && ppos > spos)
    if (!FileSystem::Instance()->IsDirectory(newDirPath))
    {
        FileSystem::Instance()->SplitPath(newDirPath, currentDir, selectedFile);
    }
    else 
    {
        currentDir = newDirPath;
        selectedFile = "";
    }
    
    if (currentDir[currentDir.length()-1] == '/') 
    {
        currentDir.substr(0, currentDir.length()-1);
    }

    Logger::Info("Setting path: %s", currentDir.c_str());
    Logger::Info("Setting file: %s", selectedFile.c_str());
    if (GetParent()) 
    {
        RefreshList();
    }
}

const String &UIFileSystemDialog::GetCurrentDir()
{
    return currentDir;
}
    
void UIFileSystemDialog::SetExtensionFilter(const String & extensionFilter)
{
    Vector<String>  newExtensionFilter;
    Split(extensionFilter, ";", newExtensionFilter);
    SetExtensionFilter(newExtensionFilter);
}

void UIFileSystemDialog::SetExtensionFilter(const Vector<String> &newExtensionFilter)
{
    DVASSERT(!GetParent());
    extensionFilter.clear();
    extensionFilter = newExtensionFilter;
    
	int32 size = extensionFilter.size();
    for (int32 k = 0; k < size; ++k)
        std::transform(extensionFilter[k].begin(), extensionFilter[k].end(), extensionFilter[k].begin(), ::tolower);
}

const Vector<String> & UIFileSystemDialog::GetExtensionFilter()
{
    return extensionFilter;
}

void UIFileSystemDialog::OnIndexSelected(int32 index)
{
    if (fileUnits[index].type == FUNIT_DIR_OUTSIDE) 
    {
        int32 p = currentDir.rfind("/");
        SetCurrentDir(String(currentDir, 0, p));
    }
    else if (fileUnits[index].type == FUNIT_DIR_INSIDE)
    {
        SetCurrentDir(files->GetPathname(fileUnits[index].indexInFileList));
    }
    else if (fileUnits[index].type == FUNIT_FILE)
    {
        if (operationType == OPERATION_LOAD) 
        {
            if (delegate)
            {
                delegate->OnFileSelected(this, files->GetPathname(fileUnits[index].indexInFileList));
            }
            GetParent()->RemoveControl(this);
        }
        else
        {
            SaveFinishing();
        }
        
    }
}

void UIFileSystemDialog::RefreshList()
{
    workingPath->SetText(StringToWString(currentDir));
    if (operationType != OPERATION_CHOOSE_DIR) 
    {
        positiveButton->SetDisabled(true);
    }
    else 
    {
        positiveButton->SetDisabled(false);
    }

    SafeRelease(files);
    lastSelected = NULL;
    lastSelectedIndex = -1;
    Logger::Debug("Cur Dir: %s", currentDir.c_str());
    if (currentDir.size() == 0)
        currentDir = "/";
    
    files = new FileList(currentDir);
    files->Sort();
    
    fileUnits.clear();
    int32 cnt = files->GetCount();
    int32 outCnt = 0;
    int32 p = -1;
    
    while (true) 
    {
        p = currentDir.rfind("/", p);
        if(p != currentDir.npos)
        {
            p--;
            outCnt++;
            if (p <= 0) 
            {
                break;
            }
        }
        else 
        {
            break;
        }
    }

    if(outCnt >= 1)
    {
        DialogFileUnit fud;
        fud.name = "..";
        fud.type = FUNIT_DIR_OUTSIDE;
        fud.indexInFileList = -1;
        fileUnits.push_back(fud);
    }
        
    for (int i = 0; i < cnt; i++)
    {
        if (!files->IsNavigationDirectory(i))
        {
            DialogFileUnit fu;
            fu.name = files->GetFilename(i);
            fu.indexInFileList = i;
            fu.type = FUNIT_FILE;
            if (files->IsDirectory(i)) 
            {
                fu.type = FUNIT_DIR_INSIDE;
            }
            else 
            {
                if (operationType == OPERATION_CHOOSE_DIR) 
                {
                    continue;
                }
                if (fu.name == selectedFile) 
                {
                    lastSelectedIndex = fileUnits.size();
                    positiveButton->SetDisabled(false);
                    textField->SetText(StringToWString(files->GetFilename(fu.indexInFileList)));
                }
                String ext = FileSystem::GetExtension(fu.name);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                bool isPresent = false;
				int32 size = extensionFilter.size();
                for (int32 n = 0; n < size; n++) 
                {
                    if (extensionFilter[n] == ".*" || ext == extensionFilter[n])
                    {
                        isPresent = true;
                        break;
                    }
                }
                if (!isPresent) 
                {
                    continue;
                }
            }

            
            fileUnits.push_back(fu);
        }
    }
    fileListView->ResetScrollPosition();
    fileListView->Refresh();
}


void UIFileSystemDialog::TextFieldShouldReturn(UITextField * textField)
{
    SaveFinishing();
}

bool UIFileSystemDialog::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    if (textField->GetText().size() + replacementLength > 0) 
    {
        positiveButton->SetDisabled(false);
    }
    else 
    {
        positiveButton->SetDisabled(true);
    }

    return true;
}


int32 UIFileSystemDialog::ElementsCount(UIList *forList)
{
    return fileUnits.size();
}

UIListCell *UIFileSystemDialog::CellAtIndex(UIList *forList, int32 index)
{
    UIListCell *c = forList->GetReusableCell("File cell"); //try to get cell from the reusable cells store
    if(!c)
    { //if cell of requested type isn't find in the store create new cell
        c = new UIListCell(Rect(0, 0, (float32)forList->size.x, (float32)cellH), "File cell");
        UIStaticText *text = new UIStaticText(Rect(0, 0, (float32)forList->size.x, (float32)cellH));
        c->AddControl(text);
        text->SetName("CellText");
        text->SetFittingOption(TextBlock::FITTING_REDUCE);
        text->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);
        Font *f = FTFont::Create(fontPath);
        f->SetSize((float32)cellH * 2 / 3);
        f->SetColor(Color(1,1,1,1));
        text->SetFont(f);
        SafeRelease(f);
        c->GetBackground()->SetColor(Color(0.75, 0.75, 0.75, 0.5));
    }
    UIStaticText *t = (UIStaticText *)c->FindByName("CellText");
    if (fileUnits[index].type == FUNIT_FILE) 
    {
        t->SetText(StringToWString(fileUnits[index].name));
    }
    else 
    {
        t->SetText(StringToWString("[" + fileUnits[index].name + "]"));
    }
    
    if (index != lastSelectedIndex) 
    {
        c->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    }
    else
    {
        c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
        lastSelected = c;
    }

    return c;//returns cell
}

int32 UIFileSystemDialog::CellHeight(UIList *forList, int32 index)
{
    return cellH;
}

void UIFileSystemDialog::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    if (lastSelected) 
    {
        lastSelected->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    }
    uint64 curTime = SystemTimer::Instance()->AbsoluteMS();
    if (curTime - lastSelectionTime < 330 && lastSelected == selectedCell)
    {
        lastSelected = selectedCell;
        lastSelectedIndex = lastSelected->GetIndex();
        lastSelected->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
        lastSelectionTime = curTime;
        OnIndexSelected(lastSelectedIndex);
    }
    else 
    {
        lastSelected = selectedCell;
        lastSelectedIndex = lastSelected->GetIndex();
        lastSelected->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
        lastSelectionTime = curTime;
        if (operationType == OPERATION_LOAD) 
        {
            if (fileUnits[selectedCell->GetIndex()].type == FUNIT_FILE)
            {
                positiveButton->SetDisabled(false);
            }
            else 
            {
                positiveButton->SetDisabled(true);
            }
        }
        else if (operationType == OPERATION_SAVE)
        {
            if (fileUnits[selectedCell->GetIndex()].type == FUNIT_FILE)
            {
                textField->SetText(StringToWString(files->GetFilename(fileUnits[lastSelectedIndex].indexInFileList)));
                positiveButton->SetDisabled(false);
            }
        }

        

    }


}
    
};

