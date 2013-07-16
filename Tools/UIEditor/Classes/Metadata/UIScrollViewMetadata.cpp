//
//  UIScrollViewMetadata.cpp
//  UIEditor
//
//  Created by Denis Bespalov on 4/23/13.
//
//

#include "UIScrollViewMetadata.h"

namespace DAVA {

UIScrollViewMetadata::UIScrollViewMetadata(QObject* parent) :
	UIControlMetadata(parent)
{
}

UIScrollView* UIScrollViewMetadata::GetActiveUIScrollView() const
{
	return dynamic_cast<UIScrollView*>(GetActiveUIControl());
}

void UIScrollViewMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	UIControlMetadata::InitializeControl(controlName, position);
	
	int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
		// Initialize UIList
        UIScrollView* scrollView = dynamic_cast<UIScrollView*>(this->treeNodeParams[i].GetUIControl());
		if (scrollView)
		{
			scrollView->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
		}
    }	
}

void UIScrollViewMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
	UIControlMetadata::UpdateExtraData(extraData, updateStyle);
}

float UIScrollViewMetadata::GetHorizontalScrollPosition() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
	Vector2 padding = GetActiveUIScrollView()->GetPadding();
	
	return (-1) * qRound(padding.x);
}

void UIScrollViewMetadata::SetHorizontalScrollPosition(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 padding = GetActiveUIScrollView()->GetPadding();
	padding.x = (-1) * value;	

	GetActiveUIScrollView()->SetPadding(padding);
}

float UIScrollViewMetadata::GetVerticalScrollPosition() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	Vector2 padding = GetActiveUIScrollView()->GetPadding();
	
	return (-1) * qRound(padding.y);
}

void UIScrollViewMetadata::SetVerticalScrollPosition(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	Vector2 padding = GetActiveUIScrollView()->GetPadding();
	padding.y = (-1) * value;

	GetActiveUIScrollView()->SetPadding(padding);
}

float UIScrollViewMetadata::GetContentSizeX() const
{
	if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	Vector2 contentSize = GetActiveUIScrollView()->GetContentSize();
		
	return contentSize.x;
}

void UIScrollViewMetadata::SetContentSizeX(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 contentSize = GetActiveUIScrollView()->GetContentSize();
	contentSize.x = value;
}

float UIScrollViewMetadata::GetContentSizeY() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	Vector2 contentSize = GetActiveUIScrollView()->GetContentSize();
		
	return contentSize.y;
}

void UIScrollViewMetadata::SetContentSizeY(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 contentSize = GetActiveUIScrollView()->GetContentSize();
	contentSize.y = value;
}
}