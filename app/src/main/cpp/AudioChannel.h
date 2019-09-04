//
// Created by LUYI on 2019/8/11.
//


#ifndef FFMPEGPLAYER_AUDIOCHANNEL_H
#define FFMPEGPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"
extern "C"{
#include <libswresample/swresample.h>
};
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id, AVCodecContext* formatContext);

    virtual ~AudioChannel();

    void start();

    void stop();

    void audio_decode();

    void audio_play();

    int getPCM();

    uint8_t *out_buffers = 0;
    int out_channels;
    int out_sampleSize;
    int out_sampleRate;
    int out_buffers_size;

private:
    pthread_t pid_start;
    pthread_t pid_audio_decode;
    pthread_t pid_audio_start;
    SLObjectItf engineObject = 0;
    SLEngineItf  engineInterface = 0;
    SLPlayItf bqPlayerPlay =0;
    SLObjectItf  outputMixObject = 0;
    SLObjectItf  bqPlayerObject=  0;
    SwrContext *swrContext = 0;

    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = 0;


};
#endif //FFMPEGPLAYER_AUDIOCHANNEL_H