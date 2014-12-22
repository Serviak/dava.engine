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


#ifndef __DAVA_REF_PTR_H__
#define __DAVA_REF_PTR_H__

#include "Base/BaseObject.h"

namespace DAVA
{

/// reference pointer wrapper for BaseObject refcounted classes.
template <class T> class RefPtr
{
public:
	RefPtr()
	{
		_ptr = 0;
	}
	
	explicit RefPtr(T * p)
	{
		_ptr = p;
	}
	
	
	/// reinitializes pointer without incrementing reference
	void Set(T * p)
	{
		T*tmp_ptr = _ptr;
		_ptr = p;
		SafeRelease(tmp_ptr);
	}
	
	~RefPtr()
	{
		SafeRelease(_ptr);
	}
		
	RefPtr(const RefPtr<T> & rp)
	{
		_ptr = rp._ptr;
		
		SafeRetain(_ptr);
	}
	
	template <class Other> RefPtr(RefPtr < Other > & rp)
	{
		_ptr = rp.Get();
		
		SafeRetain(_ptr);
	}
	
	T * Get() const
	{
		return _ptr;
	}
	
	bool Valid() const
	{
		return _ptr != 0;
	}
	
	RefPtr & operator = (const RefPtr & rp)
	{
		assign(rp);
		return *this;
	}
	
	template <class Other> RefPtr & operator = (const RefPtr<Other> & rp)
	{
		assign(rp);
		return *this;
	}
	
	RefPtr& operator = (T * ptr)
	{
		if (_ptr == ptr)
			return *this;
		
		T* tmp_ptr = _ptr;
		_ptr = ptr;
		SafeRetain(_ptr);
		SafeRelease(tmp_ptr);
		return *this;
	}
	
	T& operator *() const { return *_ptr; }
	T* operator->() const { return _ptr; }
    operator T*  () const { return _ptr; }
	
	bool operator == (const RefPtr & rp) const { return _ptr == rp._ptr; }
	bool operator == (const T* ptr) const { return _ptr == ptr; }
    bool operator == (int nullPtr) const { DVASSERT(nullPtr == 0); return _ptr == 0; } // Enable "if( sp == 0 || sp == NULL) {...}"
    friend bool operator == (const T* ptr, const RefPtr<T> &rp);
    friend bool operator == (int nullPtr, const RefPtr<T> &rp);// Enable "if( 0 == sp || NULL == sp) {...}"
	
	bool operator != (const RefPtr& rp) const { return _ptr != rp._ptr; }
	bool operator != (const T* ptr) const { return _ptr != ptr; }
    bool operator != (int nullPtr) const { DVASSERT(nullPtr == 0); return _ptr != 0; } // Enable "if( sp != 0 || sp != NULL) {...}"
    friend bool operator != (const T* ptr, const RefPtr<T> &rp);
    friend bool operator != (int nullPtr, const RefPtr<T> &rp); // Enable "if( 0 != sp || NULL != sp) {...}"

    operator bool() const // Enables "if (sp) {...}" 
    {
        return (_ptr != 0);
    }
	bool operator!() const // Enables "if (!sp) {...}" 
	{ 
		return _ptr == 0;
	}
private:
	class Tester
	{
		void operator delete(void*);
	};
public:
	operator Tester*() const
	{
		if (!_ptr) return 0;
		static Tester test;
		return &test;
	}


private:
	T * _ptr;
	
	
	template <class Other> void assign(const RefPtr<Other> & rp)
	{
		if (_ptr == rp.Get())
			return;
		
		T* tmp_ptr = _ptr;
		_ptr = rp.Get();
		SafeRetain(_ptr);
		SafeRelease(tmp_ptr);
	}
};
template <class T>
inline bool operator == (const T *ptr, const RefPtr<T> &rp) { return ptr == rp._ptr; }

template <class T>
inline bool operator == (int nullPtr, const RefPtr<T> &rp) { DVASSERT(nullPtr == 0); return 0 == rp._ptr; }

template <class T>
inline bool operator != (const T *ptr, const RefPtr<T> &rp) { return ptr != rp._ptr; }

template <class T>
inline bool operator != (int nullPtr, const RefPtr<T> &rp) { DVASSERT(nullPtr == 0); return 0 != rp._ptr; }

} // ns

#endif // __DAVA_REF_PTR_H__
