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



#include "FileSystem/Logger.h"
#include "Utils/UTF8Utils.h"
#include "Utils/Utils.h"
#include "ExternC/AndroidLayer.h"
#include "Platform/TemplateAndroid/WebViewControlAndroid.h"

#include "Render/Texture.h"
#include "Render/2D/Sprite.h"
#include "UI/UIWebView.h"


namespace DAVA
{

int32 WebViewControl::webViewIdCount = 0;
int32 WebViewControl::requestId = 0;

jclass JniWebView::gJavaClass = NULL;
const char* JniWebView::gJavaClassName = NULL;
JniWebView::CONTROLS_MAP JniWebView::controls;

jclass JniWebView::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniWebView::GetJavaClassName() const
{
	return gJavaClassName;
}

void JniWebView::Initialize(WebViewControl* control, int id, const Rect& controlRect)
{
	controls[id] = control;
	Rect rect = V2P(controlRect);

	jmethodID mid = GetMethodID("Initialize", "(IFFFF)V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniWebView::Deinitialize(int id)
{
	controls.erase(id);
	jmethodID mid = GetMethodID("Deinitialize", "(I)V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id);
}

void JniWebView::OpenURL(int id, const String& urlToOpen)
{
	jmethodID mid = GetMethodID("OpenURL", "(ILjava/lang/String;)V");
	if (mid)
	{
		jstring jUrlToOpen = GetEnvironment()->NewStringUTF(urlToOpen.c_str());
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, jUrlToOpen);
		GetEnvironment()->DeleteLocalRef(jUrlToOpen);
	}
}

void JniWebView::LoadHtmlString(int id, const String& htmlString)
{
	jmethodID mid = GetMethodID("LoadHtmlString", "(ILjava/lang/String;)V");
	if (mid)
	{
		jstring jHtmlStringToOpen = GetEnvironment()->NewStringUTF(htmlString.c_str());
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, jHtmlStringToOpen);
		GetEnvironment()->DeleteLocalRef(jHtmlStringToOpen);
	}
}

void JniWebView::ExecuteJScript(int id, int requestId, const String& scriptString)
{
	jmethodID mid = GetMethodID("ExecuteJScript", "(IILjava/lang/String;)V");
	if (mid)
	{
		jstring jScriptToExecute = GetEnvironment()->NewStringUTF(scriptString.c_str());
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, requestId, jScriptToExecute);
		GetEnvironment()->DeleteLocalRef(jScriptToExecute);
	}
}

void JniWebView::DeleteCookies(const String& targetURL)
{
	jmethodID mid = GetMethodID("DeleteCookies", "(Ljava/lang/String;)V");
	if (mid)
	{
		jstring jTargetURL = GetEnvironment()->NewStringUTF(targetURL.c_str());
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, jTargetURL);
		GetEnvironment()->DeleteLocalRef(jTargetURL);
	}
}

String JniWebView::GetCookie(const String& targetUrl, const String& cookieName)
{
	jmethodID mid = GetMethodID("GetCookie", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
	String returnStr = "";

	if (mid)
	{
		jstring jTargetURL = GetEnvironment()->NewStringUTF(targetUrl.c_str());
		jstring jName = GetEnvironment()->NewStringUTF(cookieName.c_str());
		jobject item = GetEnvironment()->CallStaticObjectMethod(GetJavaClass(), mid, jTargetURL, jName);

		CreateStringFromJni(GetEnvironment(), jstring(item), returnStr);

		GetEnvironment()->DeleteLocalRef(jTargetURL);
		GetEnvironment()->DeleteLocalRef(jName);
	}

	return returnStr;
}

Map<String, String> JniWebView::GetCookies(const String& targetUrl)
{
	jmethodID mid = GetMethodID("GetCookies", "(Ljava/lang/String;)[Ljava/lang/Object;");
	Map<String, String> cookiesMap;

	if (mid)
	{
		jstring jTargetURL = GetEnvironment()->NewStringUTF(targetUrl.c_str());

		jobjectArray jArray = (jobjectArray) GetEnvironment()->CallStaticObjectMethod(GetJavaClass(), mid, jTargetURL);
		if (jArray)
		{
			jsize size = GetEnvironment()->GetArrayLength(jArray);
			for (jsize i = 0; i < size; ++i)
			{
				jobject item = GetEnvironment()->GetObjectArrayElement(jArray, i);
				String cookiesString = "";
				CreateStringFromJni(GetEnvironment(), jstring(item), cookiesString);

				Vector<String> cookieEntry;
				Split(cookiesString, "=", cookieEntry);

				DVASSERT(1 < cookieEntry.size());
				cookiesMap[cookieEntry[0]] = cookieEntry[1];
			}
		}

		GetEnvironment()->DeleteLocalRef(jTargetURL);
	}

	return cookiesMap;
}

void JniWebView::OpenFromBuffer(int id, const String& string, const String& baseUrl)
{
	jmethodID mid = GetMethodID("OpenFromBuffer", "(ILjava/lang/String;Ljava/lang/String;)V");
	if (mid)
	{
		jstring jString = GetEnvironment()->NewStringUTF(string.c_str());
		jstring jBaseUrl = GetEnvironment()->NewStringUTF(baseUrl.c_str());
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, jString, jBaseUrl);
		GetEnvironment()->DeleteLocalRef(jString);
		GetEnvironment()->DeleteLocalRef(jBaseUrl);
	}
}

void JniWebView::SetRect(int id, const Rect& controlRect)
{
	Rect rect = V2P(controlRect);
	jmethodID mid = GetMethodID("SetRect", "(IFFFF)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, rect.x, rect.y, rect.dx, rect.dy);
	}
}

void JniWebView::SetVisible(int id, bool isVisible)
{
	jmethodID mid = GetMethodID("SetVisible", "(IZ)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, isVisible);
	}
}

void JniWebView::SetBackgroundTransparency(int id, bool enabled)
{
	jmethodID mid = GetMethodID("SetBackgroundTransparency", "(IZ)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, enabled);
	}
}

void JniWebView::SetRenderToTexture(int id, bool renderToTexture)
{
    jmethodID mid = GetMethodID("setRenderToTexture", "(IZ)V");
    if (mid)
    {
        GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id,
                renderToTexture);
    }
}

bool JniWebView::IsRenderToTexture(int id)
{
    jmethodID mid = GetMethodID("isRenderToTexture", "(I)Z");
    if (mid)
    {
        jboolean result = GetEnvironment()->CallStaticBooleanMethod(
                GetJavaClass(), mid, id);
        return result == 0 ? false : true;
    }
    return false; // if we come here java exception will throw
}

IUIWebViewDelegate::eAction JniWebView::URLChanged(int id, const String& newURL)
{
	CONTROLS_MAP::iterator iter = controls.find(id);
	if (iter == controls.end())
	{
		Logger::Error("Error web view id=%d", id);
		return IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
	}

	WebViewControl* control = iter->second;
	IUIWebViewDelegate *delegate = control->delegate;

	if (delegate)
	{
	    return delegate->URLChanged(&control->webView, newURL, true);
	}
	return IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
}

void JniWebView::PageLoaded(int id, int* rawPixels, int width, int height)
{
	CONTROLS_MAP::iterator iter = controls.find(id);
	if (iter == controls.end())
	{
		Logger::Error("Error web view id=%d", id);
		return;
	}

	WebViewControl* control = iter->second;
	IUIWebViewDelegate *delegate = control->delegate;
	if (delegate)
	{
		delegate->PageLoaded(&control->webView);
	}

	UIWebView& webView = control->GetUIWebView();
	if (rawPixels)
	{
		{
		    Texture* tex = Texture::CreateFromData(FORMAT_RGBA8888,
		            reinterpret_cast<uint8*>(rawPixels), width, height, false);
            Rect rect = webView.GetRect();
            {
                Sprite* spr = Sprite::CreateFromTexture(tex, 0, 0, rect.dx,
                        rect.dy);
                webView.GetBackground()->SetSprite(spr, 0);
                SafeRelease(spr);
            }
            SafeRelease(tex);
		}
	} else
	{
		// reset sprite to prevent render old sprite under native webveiw
		webView.SetSprite(nullptr, 0);
	}
}

void JniWebView::OnExecuteJScript(int id, int requestId, const String& result)
{
	CONTROLS_MAP::iterator iter = controls.find(id);
	if (iter == controls.end())
	{
		Logger::Error("Error web view id=%d", id);
		return;
	}

	WebViewControl* control = iter->second;
	IUIWebViewDelegate *delegate = control->delegate;
	if (delegate)
	{
		delegate->OnExecuteJScript(&control->webView, requestId, result);
	}
}

WebViewControl::WebViewControl(UIWebView& uiWebView):
	webViewId(-1),
	delegate(nullptr),
	webView(uiWebView)
{
	webView = uiWebView;

	webViewId = webViewIdCount++;
}

WebViewControl::~WebViewControl()
{
	JniWebView jniWebView;
	jniWebView.Deinitialize(webViewId);
}

void WebViewControl::Initialize(const Rect& rect)
{
	JniWebView jniWebView;
	jniWebView.Initialize(this, webViewId, rect);
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
	JniWebView jniWebView;
	jniWebView.OpenURL(webViewId, urlToOpen);
}

void WebViewControl::LoadHtmlString(const WideString& urlToOpen)
{
	JniWebView jniWebView;
	jniWebView.LoadHtmlString(webViewId, UTF8Utils::EncodeToUTF8(urlToOpen));
}

void WebViewControl::DeleteCookies(const String& targetUrl)
{
	JniWebView jniWebView;
	jniWebView.DeleteCookies(targetUrl);
}

String WebViewControl::GetCookie(const String& url, const String& name) const
{
	JniWebView jniWebView;
	return jniWebView.GetCookie(url, name);
}
// Get the list of cookies for specific domain
Map<String, String> WebViewControl::GetCookies(const String& url) const
{
	JniWebView jniWebView;
	return jniWebView.GetCookies(url);
}

int32 WebViewControl::ExecuteJScript(const String& scriptString)
{
	requestId++;
	JniWebView jniWebView;
	jniWebView.ExecuteJScript(webViewId, requestId, scriptString);
	return requestId;
}

void WebViewControl::OpenFromBuffer(const String& data, const FilePath& urlToOpen)
{
	JniWebView jniWebView;
	jniWebView.OpenFromBuffer(webViewId, data, urlToOpen.AsURL());
}

void WebViewControl::SetRect(const Rect& rect)
{
	JniWebView jniWebView;
	jniWebView.SetRect(webViewId, rect);
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
	JniWebView jniWebView;
	jniWebView.SetVisible(webViewId, isVisible);
}

void WebViewControl::SetDelegate(DAVA::IUIWebViewDelegate *delegate, UIWebView*)
{
    this->delegate = delegate;
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
	JniWebView jniWebView;
	jniWebView.SetBackgroundTransparency(webViewId, enabled);
}

void WebViewControl::SetRenderToTexture(bool value)
{
    JniWebView jniWebView;
    jniWebView.SetRenderToTexture(webViewId, value);
}
bool WebViewControl::IsRenderToTexture() const
{
    JniWebView jniWebView;
    return jniWebView.IsRenderToTexture(webViewId);
}

}//namespace DAVA
