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


#include "Utils.h"
#include "FileSystem/FileSystem.h"
#include "Render/TextureDescriptor.h"
#include <QPainter>

// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension)
{
    // Just wrap around the particular DAVA engine functions.

    DAVA::String truncatedName = fileName.toStdString();

    size_t truncatedStringLen = truncatedName.length() - extension.length();
    bool endsWithExtension = false;
    if (fileName.length() >= extension.length())
    {
        endsWithExtension = (truncatedName.compare(truncatedStringLen, extension.length(), extension.toStdString()) == 0);
    }

    if (endsWithExtension)
    {
        truncatedName.resize(truncatedStringLen);
    }

    return QString::fromStdString(truncatedName);
}

bool FindAndReplace(DAVA::String& str, const DAVA::String& from, const DAVA::String& to)
{
    size_t startPos = str.find(from);
    if (startPos == DAVA::String::npos)
        return false;
    str.replace(startPos, from.length(), to);
    return true;
}

QPixmap CreateIconFromColor(const QColor& color)
{
    QPixmap pix(16, 16);
    QPainter p(&pix);
    p.setPen(QColor(0, 0, 0, 0));

    if (color.alpha() < 255)
    {
        p.setBrush(QColor(250, 250, 250));
        p.drawRect(QRect(0, 0, 15, 15));
        p.setPen(QColor(200, 200, 200));
        p.setBrush(QColor(150, 150, 150));
        p.drawRect(QRect(0, 0, 7, 7));
        p.drawRect(QRect(8, 8, 15, 15));
    }

    p.setBrush(QBrush(color));
    p.drawRect(QRect(0, 0, 15, 15));
    return pix;
}
