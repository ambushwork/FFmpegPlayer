//
// Created by LUYI on 2019/10/16.
//

#ifndef FFMPEGPLAYER_VIDEOPUSHCHANNEL_H
#define FFMPEGPLAYER_VIDEOPUSHCHANNEL_H

#include <x264.h>
#include "librtmp/rtmp.h"
#include "safe_queue.h"

class VideoPushChannel {
public:
    VideoPushChannel(){
        rtmpPackets.setReleaseCallback(releasePacket);
        pthread_mutex_init(&mutex, 0);
    }

    ~VideoPushChannel(){
        pthread_mutex_destroy(&mutex);
    }
    SafeQueue<RTMPPacket *> rtmpPackets;
    pthread_t pid_start;

    static void releasePacket(RTMPPacket **packet){
        if(packet){
            RTMPPacket_Free(*packet);
            delete  packet;
            packet =0;
        }
    }

    void encodeData(int8_t *data);

    void sendSpsPps(uint8_t *sps, uint8_t *pps, int len, int pps_len);

    void sendFrame(int type, int payload, uint8_t *p_payload);

    void _task_start();

    bool isPlaying = 0;
    bool isConnected = 0;
    uint32_t start_time=0;

    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int y_len;
    int uv_len;
    char* url;
    void startLive(char* url);

    void stopLive();

    void initVideoEncoder(int i, int i1, int i2, int i3);

private:
    x264_picture_t *pic_in;
    x264_t *videoEncoder;
    pthread_mutex_t mutex;


};

#endif //FFMPEGPLAYER_VIDEOPUSHCHANNEL_H
