//
// Created by LUYI on 2019/8/11.
//


#ifndef FFMPEGPLAYER_AUDIOCHANNEL_H
#define FFMPEGPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id, AVCodecContext* formatContext);

    virtual ~AudioChannel();

    void start();

    void stop();

private:
    pthread_t pid_start;
};
#endif //FFMPEGPLAYER_AUDIOCHANNEL_H