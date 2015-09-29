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

#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

DAVA_TESTCLASS(DateTimeTest)
{
    DEDUCE_COVERED_CLASS_FROM_TESTCLASS()

    DAVA_TEST(TestFunction)
    {
        TEST_VERIFY(FormatDateTime(DateTime(1970, 0, 1, 0, 0, 0, 0)) == "1970-00-01 00:00:00+0");
        
        TEST_VERIFY(FormatDateTime(DateTime(2001, 1, 1, 0, 0, 0, 3 * 3600)) == "2001-01-01 00:00:00+10800");
        TEST_VERIFY(FormatDateTime(DateTime(2002, 2, 3, 5, 20, 10, -3 * 3600)) == "2002-02-03 05:20:10-10800");
        TEST_VERIFY(FormatDateTime(DateTime(2020, 11, 31, 5, 22, 10, -2 * 3600)) == "2020-11-31 05:22:10-7200");
        TEST_VERIFY(FormatDateTime(DateTime(2001, 1, 1, 14, 0, 0, 5 * 3600)) == "2001-01-01 14:00:00+18000");

        // Devices where tests are run should be in timezone UTC+3 to successfully pass DateTimeTest
        TEST_VERIFY_WITH_MESSAGE(DateTime::Now().GetTimeZoneOffset() == 3 * 3600, "Devices where tests are run should be in timezone UTC+3 to successfully pass DateTimeTest");
        TEST_VERIFY(FormatDateTime(DateTime(2001, 1, 1, 0, 0, 0, 3 * 3600).ConvertToLocalTimeZone()) == "2001-01-01 00:00:00+10800");
        TEST_VERIFY(FormatDateTime(DateTime(2001, 1, 1, 0, 0, 0, 3 * 3600).ConvertToTimeZone(0)) == "2001-00-31 21:00:00+0");
        TEST_VERIFY(FormatDateTime(DateTime(2001, 1, 1, 0, 0, 0, 3 * 3600).ConvertToTimeZone(-3 * 3600)) == "2001-00-31 18:00:00-10800");

        {   // Test ParseISO8601Date
            DateTime date = DateTime::Now();
            TEST_VERIFY(date.ParseISO8601Date("1970-01-01T05:00:00-03:00"));
            TEST_VERIFY(FormatDateTime(date) == "1970-00-01 05:00:00-10800");
        }
        {   // Test ParseRFC822Date
            DateTime date = DateTime::Now();
            TEST_VERIFY(date.ParseRFC822Date("Wed, 27 Sep 2006 21:36:45 +0100"));
            TEST_VERIFY(FormatDateTime(date) == "2006-08-27 21:36:45+3600");
        }

        {   // Test GmTime
            DateTime dt(2015, 10, 15, 13, 0, 0, 3 * 3600);
            TEST_VERIFY(FormatDateTime(DateTime::GmTime(dt.GetTimestamp())) == "2015-10-15 10:00:00+0");
        }
        {   // Test LocalTime
            DateTime dt(2015, 10, 15, 13, 0, 0, 0);
            TEST_VERIFY(FormatDateTime(DateTime::LocalTime(dt.GetTimestamp())) == "2015-10-15 16:00:00+10800");
        }
        {
            LocalizationSystem* ls = LocalizationSystem::Instance();
            String previousLocale = ls->GetCurrentLocale();

            ls->SetDirectory("~res:/Strings/");
            ls->Init();
            ls->SetCurrentLocale("ru");

            DAVA::String country_code = ls->GetCountryCode();

            TEST_VERIFY(country_code == "ru_RU");

            auto wideCompare = [](const WideString& left, const wchar_t* right) -> bool
            {
            	//return std::equal(left.begin(), left.end(), right);
            	for(uint32 i = 0; i < left.size(); ++i)
            	{
            		wchar_t l = left[i];
            		wchar_t r = right[i];
            		if (l != r)
            		{
            			Logger::Info("i == %d, l == %d, r == %d", i, static_cast<int>(l), static_cast<int>(r));
            			return false;
            		}
            		return true;
            	}
            };

            DateTime date;
            date = DateTime(1984, 8, 8, 16, 30, 22, 0); // 08.09.1984
            WideString x_date_ru = date.GetLocalizedDate(); // date representation
            Logger::Info("x_date_ru == \"%s\"", UTF8Utils::EncodeToUTF8(x_date_ru).c_str());
            TEST_VERIFY(wideCompare(x_date_ru, L"08.09.1984") || wideCompare(x_date_ru, L"08.09.84")); // may differ on win32/win10/android/ios
            WideString x_time_ru = date.GetLocalizedTime(); // time representation
            Logger::Info("x_time_ru == \"%s\"", UTF8Utils::EncodeToUTF8(x_time_ru).c_str());
            TEST_VERIFY(wideCompare(x_time_ru, L"16:30:22"));

            ls->SetCurrentLocale("en");
            ls->Init();
            country_code = ls->GetCountryCode();

            TEST_VERIFY(country_code == "en_US");

            WideString x_date_en = date.GetLocalizedDate(); // date representation
            Logger::Info("x_date_en == \"%s\"", UTF8Utils::EncodeToUTF8(x_date_en).c_str());
            TEST_VERIFY(wideCompare(x_date_en, L"9/8/1984") || wideCompare(x_date_en, L"9/8/84"));
            WideString x_time_en = date.GetLocalizedTime(); // time representation
            Logger::Info("x_time_en == \"%s\"", UTF8Utils::EncodeToUTF8(x_time_en).c_str());
            TEST_VERIFY(wideCompare(x_time_en, L"4:30:22 PM"));

            //TEST_VERIFY(x_date_en != x_date_ru);
            //TEST_VERIFY(x_time_en != x_time_ru);

            ls->SetCurrentLocale(previousLocale);
        }
    }

    void PrintDateTimeContent(const DateTime& inputTime)
    {
        int32 y = inputTime.GetYear();
        int32 month = inputTime.GetMonth();
        int32 day = inputTime.GetDay();
        int32 hour = inputTime.GetHour();
        int32 minute = inputTime.GetMinute();
        int32 sec = inputTime.GetSecond();
        int32 tz = inputTime.GetTimeZoneOffset() / 60;
        Logger::Debug("\tContent of current date by components:");
        Logger::Debug("\tYear:%d Month:%d DayOfMonth(counting from 1):%d Hour:%d Minute:%d Second:%d timZoneOffset(in minutes): %d", y, month, day, hour, minute, sec, tz);
    }

    String FormatDateTime(const DateTime& dt)
    {
        int32 year = dt.GetYear();
        int32 month = dt.GetMonth();
        int32 day = dt.GetDay();
        int32 hour = dt.GetHour();
        int32 minute = dt.GetMinute();
        int32 sec = dt.GetSecond();
        int32 tz = dt.GetTimeZoneOffset();
        String result = Format("%04d-%02d-%02d %02d:%02d:%02d%+d", year, month, day, hour, minute, sec, tz);
        return result;
    }
};
