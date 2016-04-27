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

#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
// http://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion

class Float16Compressor
{
    union Bits
    {
        float32 f;
        int32 si;
        uint32 ui;
    };

    static int32 const shift = 13;
    static int32 const shiftSign = 16;

    static int32 const infN = 0x7F800000; // flt32 infinity
    static int32 const maxN = 0x477FE000; // max flt16 normal as a flt32
    static int32 const minN = 0x38800000; // min flt16 normal as a flt32
    static int32 const signN = 0x80000000; // flt32 sign bit

    static int32 const infC = infN >> shift;
    static int32 const nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
    static int32 const maxC = maxN >> shift;
    static int32 const minC = minN >> shift;
    static int32 const signC = signN >> shiftSign; // flt16 sign bit

    static int32 const mulN = 0x52000000; // (1 << 23) / minN
    static int32 const mulC = 0x33800000; // minN / (1 << (23 - shift))

    static int32 const subC = 0x003FF; // max flt32 subnormal down shifted
    static int32 const norC = 0x00400; // min flt32 normal down shifted

    static int32 const maxD = infC - maxC - 1;
    static int32 const minD = minC - subC - 1;

public:
    static uint16 Compress(float32 value)
    {
        Bits v, s;
        v.f = value;
        uint32_t sign = v.si & signN;
        v.si ^= sign;
        sign >>= shiftSign; // logical shift
        s.si = mulN;
        s.si = static_cast<int32>(s.f * v.f); // correct subnormals
        v.si ^= (s.si ^ v.si) & -(minN > v.si);
        v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
        v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
        v.ui >>= shift; // logical shift
        v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
        v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
        return v.ui | sign;
    }

    static float32 Decompress(uint16 value)
    {
        Bits v;
        v.ui = value;
        int32 sign = v.si & signC;
        v.si ^= sign;
        sign <<= shiftSign;
        v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
        v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
        Bits s;
        s.si = mulC;
        s.f *= v.si;
        int32 mask = -(norC > v.si);
        v.si <<= shift;
        v.si ^= (s.si ^ v.si) & mask;
        v.si |= sign;
        return v.f;
    }
};
}
