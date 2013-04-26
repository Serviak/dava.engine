//
//  UISpinnerMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 3/11/13.
//
//

#include "UISpinnerMetadata.h"
#include "StringUtils.h"

namespace DAVA {

UISpinnerMetadata::UISpinnerMetadata(QObject* parent) :
UIControlMetadata(parent)
{
}

void UISpinnerMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	UIControlMetadata::InitializeControl(controlName, position);

	int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UISpinner* spinner = dynamic_cast<UISpinner*>(this->treeNodeParams[i].GetUIControl());
		if (spinner && spinner->GetButtonNext() && spinner->GetButtonPrevious())
		{
			spinner->GetButtonNext()->SetStateText(0, L"Next");
			spinner->GetButtonPrevious()->SetStateText(0, L"Prev");
			
			// Define some initial positions for UISpinner buttons.
			PositionSpinnerButtons(spinner);
		}
    }
}

void UISpinnerMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
	UIControlMetadata::UpdateExtraData(extraData, updateStyle);
}

QString UISpinnerMetadata::GetPrevButtonText()
{
	if (!VerifyActiveParamID() || !GetPrevButton())
    {
        return QString();
    }

	UIStaticText* textControl = GetPrevButton()->GetStateTextControl(this->uiControlStates[GetActiveStateIndex()]);
	if (textControl)
	{
		return WideString2QStrint(textControl->GetText());
	}
	
	return QString();
}

void UISpinnerMetadata::SetPrevButtonText(const QString& value)
{
	if (!VerifyActiveParamID() || !GetPrevButton())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetPrevButton()->SetStateText(this->uiControlStates[i], QStrint2WideString(value));
	}
}
	
QString UISpinnerMetadata::GetNextButtonText()
{
	if (!VerifyActiveParamID() || !GetNextButton())
    {
        return QString();
    }
	
	UIStaticText* textControl = GetNextButton()->GetStateTextControl(this->uiControlStates[GetActiveStateIndex()]);
	if (textControl)
	{
		return WideString2QStrint(textControl->GetText());
	}
	
	return QString();
}

void UISpinnerMetadata::SetNextButtonText(const QString& value)
{
	if (!VerifyActiveParamID() || !GetNextButton())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetNextButton()->SetStateText(this->uiControlStates[i], QStrint2WideString(value));
	}
}

UISpinner* UISpinnerMetadata::GetActiveUISpinner()
{
    return dynamic_cast<UISpinner*>(GetActiveUIControl());
}

UIButton* UISpinnerMetadata::GetPrevButton()
{
	UISpinner* activeSpinner = GetActiveUISpinner();
	if (!activeSpinner)
	{
		return NULL;
	}
	
	return activeSpinner->GetButtonPrevious();
}
	
UIButton* UISpinnerMetadata::GetNextButton()
{
	UISpinner* activeSpinner = GetActiveUISpinner();
	if (!activeSpinner)
	{
		return NULL;
	}
	
	return activeSpinner->GetButtonNext();
}

void UISpinnerMetadata::PositionSpinnerButtons(UISpinner* spinner)
{
	if (!spinner || !spinner->GetButtonPrevious() || !spinner->GetButtonNext())
	{
		return;
	}

	static const float32 RELATIVE_SPINNER_BUTTONS_WIDTH = 0.25f;
	
	// Position Prev and Next buttons on the right side of the control.
	Rect controlRect = spinner->GetRect();
	Rect newPrevButtonRect;
	newPrevButtonRect.x = (controlRect.dx * (1.0f - RELATIVE_SPINNER_BUTTONS_WIDTH));
	newPrevButtonRect.y = 0;
	newPrevButtonRect.dx = controlRect.dx * RELATIVE_SPINNER_BUTTONS_WIDTH;
	newPrevButtonRect.dy = controlRect.dy / 2;
	spinner->GetButtonPrevious()->SetRect(newPrevButtonRect);

	// The rect for "Next" button is the same, but shifted down.
	Rect newNextButtonRect = newPrevButtonRect;
	newNextButtonRect.y = controlRect.dy / 2;
	spinner->GetButtonNext()->SetRect(newNextButtonRect);
}

};