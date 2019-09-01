//
// Created by LUYI on 2019/8/11.
//

#ifndef FFMPEGPLAYER_VIDEOCHANNEL_H
#define FFMPEGPLAYER_VIDEOCHANNEL_H
#include "BaseChannel.h"
#include "macro.h"
extern "C"{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
}
typedef void (*RenderCallback) (uint8_t *, int, int, int);
class VideoChannel : public BaseChannel{
public:
    VideoChannel(int id, AVCodecContext* avCodecContext, int fps);

    virtual ~VideoChannel();

    void start();

    void stop();

    void video_decode();

    void video_play();

    void setRenderCallback(RenderCallback callback);

private:
    pthread_t pid_video_decode;
    pthread_t pid_video_start;
    RenderCallback  renderCallback;
    int fps;
};

#endif //FFMPEGPLAYER_VIDEOCHANNEL_H