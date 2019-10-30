//
// Created by LUYI on 2019/10/27.
//

#ifndef FFMPEGPLAYER_AUDIOPUSHCHANNEL_H
#define FFMPEGPLAYER_AUDIOPUSHCHANNEL_H


#include <sys/types.h>
#include "libfaac/include/faac.h"
#include "librtmp/rtmp.h"
#include "safe_queue.h"
#include "BasePushChannel.h"
#include <jni.h>

class AudioPushChannel{
public:
    AudioPushChannel(BasePushChannel *basePushChannel){
        this->basePushChannel = basePushChannel;
    }
    void initAudioEncoder(int sample_rate, int channel_numbers);
    void encodeData(int8_t *data);
    jint getInputSamples();
    RTMPPacket* getAudioTag();
    ~AudioPushChannel();
    BasePushChannel *basePushChannel;

private:
    int mChannels;

    faacEncHandle audioCodec;
    u_long inputSamples;
    u_long maxOutputBytes;
    u_char *buffer = 0;
};


#endif //FFMPEGPLAYER_AUDIOPUSHCHANNEL_H
