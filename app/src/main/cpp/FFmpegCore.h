//
// Created by LUYI on 2019/8/11.
//

#ifndef FFMPEGPLAYER_FFMPEGCORE_H
#define FFMPEGPLAYER_FFMPEGCORE_H


#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include <cstring>
#include <pthread.h>
#include "macro.h"


extern "C" {
#include <libavformat/avformat.h>
};




class FFmpegCore{
    friend void *task_stop(void* args);
public:
    FFmpegCore(JavaCallHelper *javaCallHelper, char* path);

    ~FFmpegCore();

    void prepare();

    void _prepare();

    void start();

    void _start();

    void setRenderCallback(RenderCallback renderCallback);

    void stop();

private:
    JavaCallHelper *javaCallHelper = 0;
    AudioChannel *audioChannel =0;
    VideoChannel *videoChannel = 0;
    char *dataSource;
    pthread_t pid_prepare;
    pthread_t pid_start;
    pthread_t pid_stop;
    bool isPlaying;
    AVFormatContext* formatContext;
    RenderCallback renderCallback;
};

#endif //FFMPEGPLAYER_FFMPEGCORE_H
