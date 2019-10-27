//
// Created by LUYI on 2019/10/16.
//

#ifndef FFMPEGPLAYER_VIDEOPUSHCHANNEL_H
#define FFMPEGPLAYER_VIDEOPUSHCHANNEL_H

#include <x264.h>
#include "librtmp/rtmp.h"
#include "safe_queue.h"
#include "macro.h"
#include "BasePushChannel.h"

class VideoPushChannel{
public:
    VideoPushChannel(BasePushChannel *basePushChannel){
        this->basePushChannel = basePushChannel;

    }

    ~VideoPushChannel(){

        if(videoEncoder){
            x264_encoder_close(videoEncoder);
            videoEncoder = 0;
        }
        if(pic_in){
            x264_picture_clean(pic_in);
            DELETE(pic_in);
        }
    }



    void encodeData(int8_t *data);

    void sendSpsPps(uint8_t *sps, uint8_t *pps, int len, int pps_len);

    void sendFrame(int type, int payload, uint8_t *p_payload);
    uint32_t start_time=0;
    BasePushChannel *basePushChannel;
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int y_len;
    int uv_len;

    void initVideoEncoder(int i, int i1, int i2, int i3);

private:
    x264_picture_t *pic_in;
    x264_t *videoEncoder;

};

#endif //FFMPEGPLAYER_VIDEOPUSHCHANNEL_H
