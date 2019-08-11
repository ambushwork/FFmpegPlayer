//
// Created by LUYI on 2019/8/11.
//

#ifndef FFMPEGPLAYER_FFMPEGCORE_H
#define FFMPEGPLAYER_FFMPEGCORE_H


#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include <cstring>

class FFmpegCore{
public:
    FFmpegCore(JavaCallHelper *javaCallHelper, char* path);

    ~FFmpegCore();

    void prepare();

    void _prepare();

private:
    JavaCallHelper *javaCallHelper = 0;
    AudioChannel *audioChannel =0;
    VideoChannel *videoChannel = 0;
    char *dataSource;
    pthread_t pid_prepare;
};

#endif //FFMPEGPLAYER_FFMPEGCORE_H
