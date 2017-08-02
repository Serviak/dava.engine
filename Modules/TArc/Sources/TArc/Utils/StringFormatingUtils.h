#pragma once

#include <Base/BaseTypes.h>

#include <QString>

namespace DAVA
{
namespace TArc
{
void ReduceZeros(String& value);
void ReduceZeros(QString& value);
void FloatToString(float32 value, int32 precision, String& result);
void FloatToString(float32 value, int32 precision, QString& result);
void FloatToString(float64 value, int32 precision, QString& result);
} // namespace TArc
} // namespace DAVA