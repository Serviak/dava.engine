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


#include "Base/Result.h"

using namespace DAVA;

Result::Result(ResultType type_, const DAVA::String &message_, const DAVA::VariantType &data_)
    : type(type_)
    , message(message_)
    , data(data_)
{
}

ResultList::ResultList()
    : allOk(true)
{
    
}

ResultList::ResultList(const Result& result)
    : allOk(result)
{
    AddResult(result);
}

ResultList::ResultList(const ResultList& resultList)
    : allOk(resultList.allOk)
{
    results = resultList.GetResults();
}

ResultList::ResultList(const ResultList&& resultList)
    : allOk(resultList.allOk)
{
    results = std::move(resultList.results);
}

ResultList& ResultList::operator=(ResultList& resultList)
{
    results = resultList.results;
    allOk = resultList.allOk;
    return *this;
}

ResultList& ResultList::operator=(ResultList&& resultList)
{
    results = std::move(resultList.results);
    allOk = resultList.allOk;
    return *this;
}

ResultList& ResultList::AddResult(const Result &result)
{
    allOk &= result;
    results.push_back(result);
    return *this;
}

ResultList& ResultList::AddResult(const Result &&result)
{
    allOk &= result;
    results.emplace_back(result);
    return *this;
}

ResultList& ResultList::AddResult(const Result::ResultType type, const String &message, const VariantType &data)
{
    return AddResult(Result(type, message, data));
}

ResultList& ResultList::AddResultList(const ResultList &resultList)
{
    allOk &= resultList.allOk;
    results.insert(results.end(), resultList.results.begin(), resultList.results.end());
    return *this;
}

ResultList& ResultList::AddResultList(const ResultList &&resultList)
{
    allOk &= resultList.allOk;
    if (results.empty())
    {
        results = move(resultList.results);
    }
    else
    {
        move(begin(resultList.results), end(resultList.results), back_inserter(results));
    }
    return *this;
}
