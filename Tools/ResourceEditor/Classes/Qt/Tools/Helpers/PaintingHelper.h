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


#ifndef PALETTEHELPER_H
#define PALETTEHELPER_H

#include <QObject>
#include <QColor>
#include <QImage>

#include "../GuiTypes.h"

class PaintingHelper
{
public:
    static QImage DrawHSVImage(const QSize& size);
    static QImage DrawGradient(const QSize& size, const QColor& c1, const QColor& c2, Qt::Orientation orientation);
    static QBrush DrawGridBrush(const QSize& size);
    static QImage DrawArrowIcon(const QSize& size, EDGE dimension, const QColor& bgColor = Qt::lightGray, const QColor& brdColor = Qt::gray);

    static QPoint GetHSVColorPoint(const QColor& c, const QSize& size);
    static int HueRC(const QPoint& pt, const QSize& size);
    static int SatRC(const QPoint& pt, const QSize& size);
    static int ValRC(const QPoint& pt, const QSize& size);

    static QColor MinColorComponent(const QColor& color, char component);
    static QColor MaxColorComponent(const QColor& color, char component);
};

#endif // PALETTEHELPER_H
