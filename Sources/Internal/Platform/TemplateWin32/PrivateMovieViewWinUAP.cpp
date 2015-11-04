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

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/Logger.h"
#include "Utils/Utils.h"

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Platform/TemplateWin32/PrivateMovieViewWinUAP.h"

using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace concurrency;

namespace DAVA
{

PrivateMovieViewWinUAP::PrivateMovieViewWinUAP()
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
{}

PrivateMovieViewWinUAP::~PrivateMovieViewWinUAP()
{
    if (nativeMovieView != nullptr)
    {
        // Compiler complains of capturing nativeWebView data member in lambda
        MediaElement^ p = nativeMovieView;
        core->RunOnUIThread([p]() { // We don't need blocking call here
            static_cast<CorePlatformWinUAP*>(Core::Instance())->XamlApplication()->RemoveUIElement(p);
        });
        nativeMovieView = nullptr;
    }
}

void PrivateMovieViewWinUAP::OwnerAtPremortem()
{
    // For now do nothing here
}

void PrivateMovieViewWinUAP::Initialize(const Rect& rect)
{
    core->RunOnUIThreadBlocked([this, &rect]() {
        nativeMovieView = ref new MediaElement();
        nativeMovieView->AllowDrop = false;
        nativeMovieView->CanDrag = false;
        nativeMovieView->AutoPlay = false;
        nativeMovieView->Volume = 1.0;

        core->XamlApplication()->AddUIElement(nativeMovieView);

        InstallEventHandlers();
        PositionMovieView(rect);
    });
}

void PrivateMovieViewWinUAP::SetRect(const Rect& rect)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, rect]() {
        PositionMovieView(rect);
    });
}

void PrivateMovieViewWinUAP::SetVisible(bool isVisible)
{
    if (visible != isVisible)
    {
        visible = isVisible;
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self]() {
            nativeMovieView->Visibility = visible ? Visibility::Visible : Visibility::Collapsed;
        });
    }
}

void PrivateMovieViewWinUAP::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    movieLoaded = false;
    playRequest = false;
    moviePlaying = false;

    Uri^ uri = UriFromPath(moviePath);
    if (uri != nullptr)
    {
        IRandomAccessStream^ stream = CreateStreamFromUri(uri);
        if (stream != nullptr)
        {
            Stretch scaling = Stretch::None;
            switch (params.scalingMode)
            {
            case scalingModeNone:
                scaling = Stretch::None;
                break;
            case scalingModeAspectFit:
                scaling = Stretch::Uniform;
                break;
            case scalingModeAspectFill:
                scaling = Stretch::UniformToFill;
                break;
            case scalingModeFill:
                scaling = Stretch::Fill;
                break;
            default:
                scaling = Stretch::None;
                break;
            }

            auto self{shared_from_this()};
            core->RunOnUIThread([this, self, stream, scaling]()
            {
                nativeMovieView->Stretch = scaling;
                nativeMovieView->SetSource(stream, L"");
            });
        }
    }
}

void PrivateMovieViewWinUAP::Play()
{
    if (movieLoaded)
    {
        if (!moviePlaying)
        {
            moviePlaying = true;
            auto self{shared_from_this()};
            core->RunOnUIThread([this, self]()
                                {
                nativeMovieView->Play();
                                });
        }
    }
    else
    {
        playRequest = true;
    }
}

void PrivateMovieViewWinUAP::Stop()
{
    // Game plays intro movie in the following sequence:
    //  1. movie->Play();
    //  2. while (movie->IsPlaying()) {}
    //  3. movie->Stop();
    // After Stop() method has been called native control shows first movie frame
    // so UIMovieView emulates Stop() through Pause()
    Pause();

    // DO NOT DELETE COMMENTED CODE
    //playRequest = false;
    //moviePlaying = false;
    //if (movieLoaded)
    //{
    //    auto self{shared_from_this()};
    //    core->RunOnUIThread([this, self]() {
    //        nativeMovieView->Stop();
    //    });
    //}
}

void PrivateMovieViewWinUAP::Pause()
{
    playRequest = false;
    moviePlaying = false;
    if (movieLoaded)
    {
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self]() {
            nativeMovieView->Pause();
        });
    }
}

void PrivateMovieViewWinUAP::Resume()
{
    Play();
}

void PrivateMovieViewWinUAP::InstallEventHandlers()
{
    std::weak_ptr<PrivateMovieViewWinUAP> self_weak(shared_from_this());
    // Install event handlers through lambdas as it seems only ref class's member functions can be event handlers directly
    auto mediaOpened = ref new RoutedEventHandler([this, self_weak](Platform::Object^, RoutedEventArgs^) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnMediaOpened();
        }
    });
    auto mediaEnded = ref new RoutedEventHandler([this, self_weak](Platform::Object^, RoutedEventArgs^) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnMediaEnded();
        }
    });
    auto mediaFailed = ref new ExceptionRoutedEventHandler([this, self_weak](Platform::Object^, ExceptionRoutedEventArgs^ args) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnMediaFailed(args);
        }
    });
    nativeMovieView->MediaOpened += mediaOpened;
    nativeMovieView->MediaEnded += mediaEnded;
    nativeMovieView->MediaFailed += mediaFailed;
}

void PrivateMovieViewWinUAP::PositionMovieView(const Rect& rectInVirtualCoordinates)
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();

    // 1. map virtual to physical
    Rect controlRect = coordSystem->ConvertVirtualToPhysical(rectInVirtualCoordinates);
    controlRect += coordSystem->GetPhysicalDrawOffset();

    // 2. map physical to window
    const float32 scaleFactor = core->GetScreenScaleFactor();
    controlRect.x /= scaleFactor;
    controlRect.y /= scaleFactor;
    controlRect.dx /= scaleFactor;
    controlRect.dy /= scaleFactor;

    // 3. set control's position and size
    nativeMovieView->MinHeight = 0.0;       // Force minimum control sizes to zero to
    nativeMovieView->MinWidth = 0.0;        // allow setting any control sizes
    nativeMovieView->Width = controlRect.dx;
    nativeMovieView->Height = controlRect.dy;
    core->XamlApplication()->PositionUIElement(nativeMovieView, controlRect.x, controlRect.y);
}

IRandomAccessStream^ PrivateMovieViewWinUAP::CreateStreamFromUri(Windows::Foundation::Uri^ uri) const
{
    auto self{shared_from_this()};
    task<StorageFile^> getFileTask(StorageFile::GetFileFromApplicationUriAsync(uri));
    task<IRandomAccessStream^> getInputStreamTask = getFileTask.then([self](StorageFile^ storageFile)
    {
        return storageFile->OpenAsync(FileAccessMode::Read);
    }).then([self](IRandomAccessStream^ stream)
    {
        return stream;
    });

    try {
        return getInputStreamTask.get();
    }
    catch (Platform::COMException^ e) {
        // Ignore errors when file is not found or access is denied
        HRESULT hr = e->HResult;
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr || HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
        {
            Logger::Error("[MovieView] failed to load file='%s': %s", WStringToString(uri->Path->Data()).c_str(),
                          HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr ? "file not found" : "access denied");
            return nullptr;
        }
        throw;  // Rethrow other exceptions
    }
}

Uri^ PrivateMovieViewWinUAP::UriFromPath(const FilePath& path) const
{
    String pathTail;
    Platform::String^ prefix = nullptr;
    String absPath = path.GetAbsolutePathname();
    FileSystem* fs = FileSystem::Instance();
    if (CheckIfPathReachableFrom(absPath, fs->GetCurrentExecutableDirectory().GetAbsolutePathname(), pathTail))
    {
        prefix = ref new Platform::String(L"ms-appx:///");
    }
    else if (CheckIfPathReachableFrom(absPath, fs->GetUserDocumentsPath().GetAbsolutePathname(), pathTail))
    {
        prefix = ref new Platform::String(L"ms-appdata:///local/");
    }
    else
    {
        DVASSERT(0 && "For now MovieView supports file loading only from GetCurrentExecutableDirectory() and GetUserDocumentsPath() folders");
        return nullptr;
    }

    Platform::String^ tail = ref new Platform::String(StringToWString(pathTail).c_str());
    return ref new Uri(prefix + tail);
}

bool PrivateMovieViewWinUAP::CheckIfPathReachableFrom(const String& pathToCheck, const String& pathToReach, String& pathTail) const
{
    if (pathToCheck.length() >= pathToReach.length())
    {
        if (0 == pathToCheck.compare(0, pathToReach.length(), pathToReach))
        {
            pathTail = pathToCheck.substr(pathToReach.length());
            if (!pathTail.empty() && '/' == pathTail.back())
            {
                pathTail.pop_back();
            }
            return true;
        }
    }
    return false;
}

void PrivateMovieViewWinUAP::OnMediaOpened()
{
    movieLoaded = true;
    if (playRequest)
    {
        playRequest = false;
        moviePlaying = true;
        nativeMovieView->Play();
    }
}

void PrivateMovieViewWinUAP::OnMediaEnded()
{
    playRequest = false;
    moviePlaying = false;
}

void PrivateMovieViewWinUAP::OnMediaFailed(ExceptionRoutedEventArgs^ args)
{
    playRequest = false;
    moviePlaying = false;
    String errMessage = WStringToString(args->ErrorMessage->Data());
    Logger::Error("[MovieView] failed to decode media file: %s", errMessage.c_str());
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
