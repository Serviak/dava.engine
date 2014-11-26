/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_IMAGEPACKER_H__
#define __DAVAENGINE_IMAGEPACKER_H__

#include "Base/BaseTypes.h"
#include "Math/Math2D.h"

namespace DAVA
{
    

//! helper class to simplify packing of many small 2D images to one big 2D image
class ImagePacker
{
public:
	//! \brief constructor
	//! \param[in] size of this imagePacker
	ImagePacker(const Rect2i & _rect, bool _useTwoSideMargin, int32 _texturesMargin);
	
	//! \brief destructor
	virtual ~ImagePacker();

	//! \brief release all data allocated by image packer and reset it internal state
	void Release(); 
	
	//! \brief Add image to image packer & image packer must allocate position for this image
	//! \param[in] imageSize image size of image we want to pack
	//! \return rectangle with image position in current image topology
	bool AddImage(const Size2i & imageSize, void * searchPtr);
	Rect2i * SearchRectForPtr(void * searchPtr);
	Rect2i * SearchRectForPtr(void * searchPtr, uint32& rmargin, uint32& bmargin);
	
	Rect2i & GetRect() { return rect; };

public:
	bool useTwoSideMargin;
	int32 texturesMargin;

private:
	// Implementation details
	Rect2i rect;
	
	struct PackNode
	{
		PackNode(ImagePacker * _packer) :
			packer(_packer)
		{
			isLeaf = true;
			child[0] = 0;
			child[1] = 0;
			isImageSet = false;
			searchPtr = 0;
			touchesRightBorder = true;
			touchesBottomBorder = true;
			rightMargin = 1;
			bottomMargin = 1;
		}
	
		bool			isImageSet;
		Rect2i			rect;
		bool			isLeaf;
		PackNode *		child[2];
		ImagePacker *	packer;
		void *			searchPtr;
		bool			touchesRightBorder;
		bool			touchesBottomBorder;
		uint32			rightMargin;
		uint32			bottomMargin;

		PackNode *	Insert(const Size2i & imageSize);
		PackNode *	SearchRectForPtr(void * searchPtr);
		void		Release();
	};
	
	
	PackNode * root;
};

};

#endif // __DAVAENGINE_IMAGEPACKER_H__
