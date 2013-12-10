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



#include "MaterialItem.h"
#include "MaterialModel.h"

MaterialItem::MaterialItem(DAVA::NMaterial * _material, MaterialModel * _model)
    : QStandardItem()
    , material(_material)
    , model(_model)
{
    DVASSERT(model);
    DVASSERT(material);
    
    setText(material->GetMaterialName().c_str());
    setData(QVariant::fromValue<DAVA::NMaterial *>(material));
    
    if(material->IsSwitchable())
    {
        setIcon(QIcon(QString::fromUtf8(":/QtLibraryIcons/lodmaterial.png")));
    }
    else
    {
        setIcon(QIcon(QString::fromUtf8(":/QtIcons/materialeditor.png")));
    }
    setEditable(false);
}

MaterialItem::~MaterialItem()
{
}

QVariant MaterialItem::data(int role) const
{
//    if(role == Qt::BackgroundColorRole)
//    {
//        if(model && model->IsMaterialSelected(material))
//        {
//            return QColor(235, 215, 210);
//        }
//    }
    
    return QStandardItem::data(role);
}

