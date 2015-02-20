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



#include "AndroidLayer.h"
#include "UI/UITextFieldAndroid.h"
#include "Base/BaseTypes.h"
#include "Utils/UTF8Utils.h"
#include "Render/Image/ImageConvert.h"

extern "C"
{
	void Java_com_dava_framework_JNITextField_TextFieldShouldReturn(JNIEnv* env, jobject classthis, uint32_t id)
	{
		DAVA::UITextFieldAndroid::TextFieldShouldReturn(id);
	}

	jbyteArray Java_com_dava_framework_JNITextField_TextFieldKeyPressed(JNIEnv* env, jobject classthis, uint32_t id, int replacementLocation, int replacementLength, jbyteArray replacementString)
	{
		DAVA::WideString string;

		jbyte* bufferPtr = env->GetByteArrayElements(replacementString, NULL);
		jsize lengthOfArray = env->GetArrayLength(replacementString);

		DAVA::UTF8Utils::EncodeToWideString((uint8_t*)bufferPtr, lengthOfArray, string);

		env->ReleaseByteArrayElements(replacementString, bufferPtr, 0);

		bool res = DAVA::UITextFieldAndroid::TextFieldKeyPressed(id, replacementLocation, replacementLength, string);
		DAVA::String returnStr = res ? DAVA::UTF8Utils::EncodeToUTF8(string) : "";

		jbyteArray r = env->NewByteArray(returnStr.length());
		if (r == NULL)
			return NULL;
		env->SetByteArrayRegion(r, 0, returnStr.length(), (const jbyte*)returnStr.c_str());
		return r;
	}

	void Java_com_dava_framework_JNITextField_TextFieldOnTextChanged(JNIEnv* env, jobject classthis, uint32_t id, jbyteArray newText, jbyteArray oldText)
	{
		DAVA::WideString newString, oldString;

		jbyte* bufferPtr = env->GetByteArrayElements(newText, NULL);
		jsize lengthOfArray = env->GetArrayLength(newText);
		DAVA::UTF8Utils::EncodeToWideString((uint8_t*)bufferPtr, lengthOfArray, newString);
		env->ReleaseByteArrayElements(newText, bufferPtr, 0);

		bufferPtr = env->GetByteArrayElements(oldText, NULL);
		lengthOfArray = env->GetArrayLength(oldText);
		DAVA::UTF8Utils::EncodeToWideString((uint8_t*)bufferPtr, lengthOfArray, oldString);
		env->ReleaseByteArrayElements(oldText, bufferPtr, 0);

		DAVA::UITextFieldAndroid::TextFieldOnTextChanged(id, newString, oldString);
	}

	void Java_com_dava_framework_JNITextField_TextFieldKeyboardShown(JNIEnv* env, jobject classthis, uint32_t id, int x, int y, int dx, int dy)
	{
	    // Recalculate to virtual coordinates.
	    DAVA::Vector2 keyboardOrigin(x, y);
	    keyboardOrigin = DAVA::VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(keyboardOrigin);

	    DAVA::Vector2 keyboardSize(dx, dy);
	    keyboardSize = DAVA::VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(keyboardSize);

	    DAVA::UITextFieldAndroid::TextFieldKeyboardShown(id, DAVA::Rect(keyboardOrigin, keyboardSize));
	}

	void Java_com_dava_framework_JNITextField_TextFieldKeyboardHidden(JNIEnv* env, jobject classthis, uint32_t id)
	{
	    DAVA::UITextFieldAndroid::TextFieldKeyboardHidden(id);
	}

    void Java_com_dava_framework_JNITextField_TextFieldFocusChanged(JNIEnv* env, jobject classthis, uint32_t id, bool hasFocus)
    {
        DAVA::UITextFieldAndroid::TextFieldFocusChanged(id, hasFocus);
    }

    void Java_com_dava_framework_JNITextField_TextFieldTextureUpdate(JNIEnv* env,
            jobject classthis, uint32_t id, jintArray pixels, int width, int height)
    {
        DVASSERT(width);
        DVASSERT(height);

        jboolean isCopy = JNI_FALSE;
        jint* rawData = env->GetIntArrayElements(pixels, &isCopy);

        DVASSERT(rawData);
        DVASSERT(env->GetArrayLength(pixels) == width * height); // ARGB

        DAVA::uint32 pitch = width * 4; // 4 byte per pixel

        // convert on the same memory
        DAVA::ImageConvert::ConvertImageDirect(DAVA::FORMAT_BGRA8888,
                DAVA::FORMAT_RGBA8888, rawData, width, height, pitch, rawData,
                width, height, pitch);

        DAVA::UITextFieldAndroid::TextFieldUpdateTexture(id, rawData, width,
                height);

        env->ReleaseIntArrayElements(pixels, rawData, 0);
    }

};
