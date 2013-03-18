#include "WebViewControlWin32.h"
#include "CorePlatformWin32.h"
using namespace DAVA;

#include <exdisp.h>

WebBrowserContainer::WebBrowserContainer() :
	hwnd(0),
	webBrowser(NULL)
{
}

WebBrowserContainer::~WebBrowserContainer()
{
	if (webBrowser)
	{
		webBrowser->Release();
		webBrowser = NULL;
	}
}

bool WebBrowserContainer::Initialize(HWND parentWindow)
{
	this->hwnd = parentWindow;

	IOleObject* oleObject = NULL;
	HRESULT hRes = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_INPROC, IID_IOleObject, (void**)&oleObject);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::Inititalize(), CoCreateInstance(CLSID_WebBrowser) failed!, error code %i", hRes);
		return false;
	}

	hRes = oleObject->SetClientSite(this);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::Inititalize(), IOleObject::SetClientSite() failed!, error code %i", hRes);
		oleObject->Release();
		return false;
	}

	// Activating the container.
	RECT rect = {0};
	GetClientRect(hwnd, &rect);
	hRes = oleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, this->hwnd, &rect);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::InititalizeBrowserContainer(), IOleObject::DoVerb() failed!, error code %i", hRes);
		oleObject->Release();
		return false;
	}

	// Prepare the browser itself.
	hRes = oleObject->QueryInterface(IID_IWebBrowser2, (void**)&this->webBrowser);
	if (FAILED(hRes))
	{
		Logger::Error("WebViewControl::InititalizeBrowserContainer(), IOleObject::QueryInterface(IID_IWebBrowser2) failed!, error code %i", hRes);
		oleObject->Release();
		return false;
	}

	// Initialization is OK.
	oleObject->Release();
	return true;
}

HRESULT __stdcall WebBrowserContainer::QueryInterface(REFIID riid, void** ppvObject)
{
	if( !ppvObject )
	{
		return E_POINTER;
	}

	if( riid==IID_IUnknown || riid==IID_IOleWindow || riid==IID_IOleInPlaceSite )
	{
		return *ppvObject = (void*)static_cast<IOleInPlaceSite*>(this), S_OK;
	}

	if( riid==IID_IOleClientSite )
	{
		return *ppvObject = (void*)static_cast<IOleClientSite*>(this), S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

HRESULT __stdcall WebBrowserContainer::GetWindow(HWND *phwnd)
{
	if (!phwnd)
	{
		return E_INVALIDARG;
	}

	*phwnd = this->hwnd;
	return S_OK;
}

HRESULT __stdcall WebBrowserContainer::GetWindowContext( IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc,
		LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	if( !(ppFrame && ppDoc && lprcPosRect && lprcClipRect && lpFrameInfo) )
	{
		return E_INVALIDARG;
	}

	*ppFrame = NULL;
	*ppDoc = NULL;

	GetClientRect( this->hwnd, lprcPosRect );
	GetClientRect( this->hwnd, lprcClipRect );

	lpFrameInfo->fMDIApp = false;
	lpFrameInfo->hwndFrame = this->hwnd;
	lpFrameInfo->haccel = 0;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

bool WebBrowserContainer::OpenUrl(const WCHAR* urlToOpen)
{
	if (!this->webBrowser)
	{
		return false;
	}

	BSTR url = SysAllocString(urlToOpen);
	VARIANT empty = {0};
	VariantInit(&empty);

	this->webBrowser->Navigate(url, &empty, &empty, &empty, &empty);
	SysFreeString(url);

	return true;
}

void WebBrowserContainer::UpdateRect()
{
	IOleInPlaceObject* oleInPlaceObject = NULL;
	HRESULT hRes = this->webBrowser->QueryInterface(IID_IOleInPlaceObject, (void**)&oleInPlaceObject);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::SetSize(), IOleObject::QueryInterface(IID_IOleInPlaceObject) failed!, error code %i", hRes);
		return;
	}

	// Update the browser window according to the holder window.
	RECT rect = {0};
	GetClientRect(this->hwnd, &rect);

	hRes = oleInPlaceObject->SetObjectRects(&rect, &rect);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::SetSize(), IOleObject::SetObjectRects() failed!, error code %i", hRes);
		return;
	}

	oleInPlaceObject->Release();
}

WebViewControl::WebViewControl()
{
	browserWindow = 0;
	browserContainer = NULL;
}

WebViewControl::~WebViewControl()
{
	if (browserWindow != 0)
	{
		::DestroyWindow(browserWindow);
	}

	SafeDelete(browserContainer);
}

void WebViewControl::Initialize(const Rect& rect)
{
	CoreWin32Platform *core = dynamic_cast<CoreWin32Platform *>(CoreWin32Platform::Instance());
	if (core == NULL)
	{
		return;
	}

	// Create the browser holder window.
	browserWindow = ::CreateWindowEx(0, L"Static", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		0, 0, 0, 0, core->hWindow, NULL, core->hInstance, NULL);
	SetRect(rect);

	// Initialize the browser itself.
	InititalizeBrowserContainer();
}

bool WebViewControl::InititalizeBrowserContainer()
{
	HRESULT hRes = ::CoInitialize(NULL);
	if (FAILED(hRes))
	{
		Logger::Error("WebViewControl::InititalizeBrowserContainer(), CoInitialize() failed!");
		return false;
	}

	this->browserContainer= new WebBrowserContainer();
	return browserContainer->Initialize(this->browserWindow);
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
	if (this->browserContainer)
	{
		this->browserContainer->OpenUrl(StringToWString(urlToOpen.c_str()).c_str());
	}
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
	if (this->browserWindow != 0)
	{
		::ShowWindow(this->browserWindow, isVisible);
	}
}

void WebViewControl::SetRect(const Rect& rect)
{
	if (this->browserWindow == 0)
	{
		return;
	}

	RECT browserRect = {0};
	::GetWindowRect(this->browserWindow, &browserRect);

	browserRect.left = (LONG)(rect.x * DAVA::Core::GetVirtualToPhysicalFactor());
	browserRect.top  = (LONG)(rect.y * DAVA::Core::GetVirtualToPhysicalFactor());
	browserRect.left  += (LONG)Core::Instance()->GetPhysicalDrawOffset().x;
	browserRect.top += (LONG)Core::Instance()->GetPhysicalDrawOffset().y;

	browserRect.right = (LONG)(browserRect.left + rect.dx * Core::GetVirtualToPhysicalFactor());
	browserRect.bottom = (LONG)(browserRect.top + rect.dy * Core::GetVirtualToPhysicalFactor());

	::SetWindowPos(browserWindow, NULL, browserRect.left, browserRect.top,
		browserRect.right - browserRect.left, browserRect.bottom - browserRect.top, SWP_NOZORDER );

	if (this->browserContainer)
	{
		this->browserContainer->UpdateRect();
	}
}
