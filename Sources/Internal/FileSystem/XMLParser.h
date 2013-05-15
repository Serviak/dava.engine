/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#ifndef __DAVAENGINE_XML_PARSER__
#define __DAVAENGINE_XML_PARSER__


#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

#include "FileSystem/FileSystem.h"

typedef unsigned char xmlChar;

namespace DAVA 
{
	
class XMLParserDelegate 
{
public:
	virtual void OnElementStarted(const String &elementName, const String &namespaceURI
										  , const String &qualifedName, const Map<String, String> &attributes) = 0;
	virtual void OnElementEnded(const String &elementName, const String &namespaceURI
										, const String &qualifedName) = 0;
	
	virtual void OnFoundCharacters(const String & chars) = 0;


	/**
	 \brief Returns attribute value if this value is presents in the attributesMap.
	 \param[in] attributes map you want to search for.
	 \param[in] key you fant to found in the map.
	 \param[out] writes to this string value for the key if attribute is present.
	 \returns true if attribute for key is present.
	 */
	inline bool GetAttribute(const Map<String, String> &attributesMap, const String &key, String &attributeValue);
};

	
class XMLParser : public BaseObject 
{
public:
	
	XMLParser();
	
	static bool ParseFile(const FilePath &fileName, XMLParserDelegate *delegate);
	static bool ParseBytes(const unsigned char *bytes, int length, XMLParserDelegate *delegate);
	
private:
	static void StartDocument(void *user_data);
	static void EndDocument(void *user_data);
	static void Characters(void *user_data, const xmlChar *ch, int len);

	static void StartElement(void *user_data, const xmlChar *name, const xmlChar **attrs);
	static void EndElement(void *user_data, const xmlChar *name);
	
	
protected:
	
	virtual ~XMLParser();
};
	
	
bool XMLParserDelegate::GetAttribute(const Map<String, String> &attributesMap, const String &key, String &attributeValue)
{
	Map<String, String>::const_iterator it;
	it = attributesMap.find(key);
	if (it != attributesMap.end())
	{
		attributeValue = it->second;
		return true;
	}
	
	return false;
}
	
};

#endif //#ifndef __DAVAENGINE_XML_PARSER__