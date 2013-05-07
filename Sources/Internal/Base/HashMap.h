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
        * Created by Sergey Zdanevich 
=====================================================================================*/

#ifndef __DAVAENGINE_HASH_MAP__
#define __DAVAENGINE_HASH_MAP__

#include "Base/BaseTypes.h"
#include "Base/Hash.h"
#include "Base/TemplateHelpers.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

template <typename K, typename V>
class HashMap
{
protected:
    struct HashMapItem;

public:
	struct HashMapIterator;
	typedef HashMapIterator Iterator;

	HashMap(size_t _hashSize = 128, V _defaultV = V());
	HashMap(const HashMap<K, V> &hm);
	~HashMap();

	size_t Size() const;

	bool IsEmpty() const;
	bool IsKey(const K &key) const;

	void Insert(const K &key, const V &value);
	void Remove(const K &key);
	void Clear();

	V GetValue(const K &key) const;
	V operator[](const K &key) const;
	HashMap<K, V>& operator=(const HashMap<K, V> &hm);

	void Resize(size_t newSize);

	Iterator Begin() const;
	Iterator End() const;
    
public:
	struct HashMapIterator
	{
		friend class HashMap<K, V>;
        
		HashMapIterator(const HashMapIterator &i);
		HashMapIterator(const HashMap *map);
        
		bool operator==(const HashMapIterator &i);
		bool operator!=(const HashMapIterator &i);
		HashMapIterator& operator++();
		HashMapIterator operator++(int count);
        
		const K & GetKey() const;
		const V & GetValue() const;
        
	protected:
		size_t szTable;
		size_t current_index;
        
		HashMapItem **table;
		HashMapItem *current_item;
        
		HashMapIterator& GoEnd();
	};

protected:
	size_t sz;
	size_t szTable;

	HashMapItem **table;
	Hash<K> hashFn;

	V defaultV;

	inline size_t GetIndex(const K &key) const;
	inline const HashMapItem* GetItem(const K &key) const;
	inline void InsertItem(HashMapItem* item);

protected:
	struct HashMapItemBase
	{ };

	struct HashMapItem : public HashMapItemBase
	{
		K key;
		V value;
		HashMapItem *next;

		HashMapItem(const K & k, const V & v)
            :   key(k), value(v), next(NULL)
		{
		}
	};
};

// 
// HashMap implementation
// begin -->

template <typename K, typename V>
HashMap<K, V>::HashMap(size_t _hashSize, V _defaultV)
	: sz(0)
	, szTable(_hashSize)
	, defaultV(_defaultV)
{
	table = new HashMapItem*[szTable];
	for(size_t i = 0; i < szTable; ++i)
	{
		table[i] = NULL;
	}
}

template <typename K, typename V>
HashMap<K, V>::HashMap(const HashMap<K, V> &hm)
	: sz(0)
	, szTable(0)
	, table(NULL)
{
	operator=(hm);
}

template <typename K, typename V>
HashMap<K, V>::~HashMap()
{
	Clear();
	delete[] table;
}

template <typename K, typename V>
size_t HashMap<K, V>::Size() const
{
	return sz;
}

template <typename K, typename V>
bool HashMap<K, V>::IsEmpty() const
{
	return (0 == sz);
}

template <typename K, typename V>
void HashMap<K, V>::Insert(const K &key, const V &value)
{
	HashMapItem* item = new HashMapItem(key, value);
	InsertItem(item);
}

template <typename K, typename V>
void HashMap<K, V>::Remove(const K &key)
{
	size_t index = GetIndex(key);
	HashMapItem* item = table[index];
	HashMapItem* prev = NULL;

	while(NULL != item)
	{
		if(hashFn.Compare(item->key, key))
		{
			if(NULL != prev)
			{
				prev->next = item->next;
			}
			else
			{
				table[index] = item->next;
			}

			sz--;
			delete item;

			break;
		}

		prev = item;
		item = item->next;
	}
}

template <typename K, typename V>
bool HashMap<K, V>::IsKey(const K &key) const
{
	return (NULL != GetItem(key));
}

template <typename K, typename V>
V HashMap<K, V>::GetValue(const K &key) const
{
	const HashMapItem* item = GetItem(key);
	if(NULL != item)
	{
		return item->value;
	}

	return defaultV;
}

template <typename K, typename V>
V HashMap<K, V>::operator[](const K &key) const
{
	return GetValue(key);
}

template <typename K, typename V>
HashMap<K, V>& HashMap<K, V>::operator=(const HashMap<K, V> &hm)
{
	if(NULL != table)
	{
		Clear();
		delete[] table;
	}

	szTable = hm.szTable;
	defaultV = hm.defaultV;

	table = new HashMapItem*[szTable];
	for(size_t i = 0; i < szTable; ++i)
	{
		table[i] = NULL;
	}

	HashMap<K, V>::Iterator i = hm.Begin();
	for(; i != hm.End(); ++i)
	{
		Insert(i.GetKey(), i.GetValue());
	}

	return *this;
}

template <typename K, typename V>
void HashMap<K, V>::Clear()
{
	const HashMapItem* item = NULL;
	const HashMapItem* next = NULL;

	for(size_t i = 0; i < szTable; ++i)
	{
		item = table[i];
		table[i] = NULL;
		while(NULL != item)
		{
			next = item->next;
			delete item;
			item = next;
		}
	}
}

template <typename K, typename V>
void HashMap<K, V>::Resize(size_t newSize)
{
	HashMapItem* item;
	HashMapItem* next = NULL;

	// not 0 and power of 2
	DVASSERT(0 != newSize && 0 == (newSize & (newSize - 1)));

	HashMapItem **oldTable = table;
	size_t szOld = szTable;

	szTable = newSize;
	table = new HashMapItem*[szTable];

	for(size_t i = 0; i < szTable; ++i)
	{
		table[i] = NULL;
	}

	for(size_t i = 0; i < szOld; ++i)
	{
		item = oldTable[i];
		while(NULL != item)
		{
			next = item->next;

			item->next = NULL;
			InsertItem(item);

			item = next;
		}
	}

	delete[] oldTable;
}

template <typename K, typename V>
typename HashMap<K, V>::Iterator HashMap<K, V>::Begin() const
{
	return Iterator(this);
}

template <typename K, typename V>
typename HashMap<K, V>::Iterator HashMap<K, V>::End() const
{
	Iterator i(this);
	return i.GoEnd();
}

template <typename K, typename V>
inline size_t HashMap<K, V>::GetIndex(const K &key) const
{
	// fast hashFn(key) % szTable
	return hashFn(key) & (szTable - 1);
}

template <typename K, typename V>
inline const typename HashMap<K, V>::HashMapItem* HashMap<K, V>::GetItem(const K &key) const
{
	size_t index = GetIndex(key);

	HashMapItem* i = table[index];

	while(NULL != i)
	{
		if(hashFn.Compare(i->key, key))
		{
			break;
		}

		i = i->next;
	}

	return i;
}

template <typename K, typename V>
inline void HashMap<K, V>::InsertItem(typename HashMap<K, V>::HashMapItem* item)
{
	size_t index = GetIndex(item->key);

	item->next = table[index];
	table[index] = item;

	sz++;
}

// 
// HashMap implementation
// end <--


// 
// HashMapIterator implementation
// begin -->

template <typename K, typename V>
HashMap<K, V>::HashMapIterator::HashMapIterator(const typename HashMap<K, V>::HashMapIterator &i)
	: szTable(i.szTable)
	, table(i.table)
	, current_index(i.current_index)
	, current_item(i.current_item)
{ }

template <typename K, typename V>
HashMap<K, V>::HashMapIterator::HashMapIterator(const HashMap<K, V> *map)
	: szTable(map->szTable)
	, table(map->table)
	, current_index(0)
	, current_item(NULL)
{
	if(NULL != table && szTable > 0)
	{
		for (uint32 k = 0; k < szTable; ++k)
			if (table[k] != 0)
			{
				current_item = table[k];
				current_index = k;
				break;
			}

			if(NULL == current_item)
			{
				GoEnd();
			}
	}
    // DVASSERT(current_index < szTable);
}

template <typename K, typename V>
bool HashMap<K, V>::HashMapIterator::operator==(const typename HashMap<K, V>::HashMapIterator &i)
{
	return (szTable == i.szTable &&
		table == i.table &&
		current_index == i.current_index &&
		current_item == i.current_item);
}

template <typename K, typename V>
bool HashMap<K, V>::HashMapIterator::operator!=(const typename HashMap<K, V>::HashMapIterator &i)
{
	return !operator==(i);
}

template <typename K, typename V>
typename HashMap<K, V>::HashMapIterator& HashMap<K, V>::HashMapIterator::operator++()
{
	// operator ++iterator

	if(NULL != current_item)
	{
		if(NULL != current_item->next)
		{
			current_item = current_item->next;
		}
		else
		{
			current_index++;

			current_item = 0;
			while(current_index < szTable && current_item == 0)
			{
				current_item = table[current_index];
                if(!current_item) current_index++;
			}

			if (current_item == 0)
			{
				GoEnd();
			}
		}
	}

	return *this;
}

template <typename K, typename V>
typename HashMap<K, V>::HashMapIterator HashMap<K, V>::HashMapIterator::operator++(int count)
{
	// operator iterator++

	HashMapIterator tmp = *this;

	while(0 < count--)
	{
		++tmp;
	}

	return tmp;
}

template <typename K, typename V>
const K & HashMap<K, V>::HashMapIterator::GetKey() const
{
	return current_item->key;
}

template <typename K, typename V>
const V & HashMap<K, V>::HashMapIterator::GetValue()const
{
	return current_item->value;
}

template <typename K, typename V>
typename HashMap<K, V>::HashMapIterator& HashMap<K, V>::HashMapIterator::GoEnd()
{
	current_item = NULL;
	current_index = 0;

	return *this;
}

// 
// HashMapIterator implementation
// end <--

};

#endif
