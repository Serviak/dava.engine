#pragma once

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "UI/IMovieViewControl.h"

namespace DAVA
{
class PrivateMovieViewWinUAP;
class MovieViewControl : public IMovieViewControl
{
public:
    MovieViewControl();
    virtual ~MovieViewControl();

    // Initialize the control.
    void Initialize(const Rect& rect) override;

    // Position/visibility.
    void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible) override;

    // Open the Movie.
    void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params) override;

    // Start/stop the video playback.
    void Play() override;
    void Stop() override;

    // Pause/resume the playback.
    void Pause() override;
    void Resume() override;

    // Whether the movie is being played?
    bool IsPlaying() const override;

    void Update() override;

private:
    std::shared_ptr<PrivateMovieViewWinUAP> privateImpl;
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
