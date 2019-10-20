//
// Created by LUYI on 2019/10/16.
//

#ifndef FFMPEGPLAYER_VIDEOPUSHCHANNEL_H
#define FFMPEGPLAYER_VIDEOPUSHCHANNEL_H


#include "librtmp/rtmp.h"
#include "safe_queue.h"

class VideoPushChannel {
public:
    VideoPushChannel(){
        rtmpPackets.setReleaseCallback(releasePacket);
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

    bool isPlaying = 0;
    bool isConnected = 0;
    uint32_t start_time=0;
    void startLive(char* url);
    void *task_start(void *args);

    void stopLive();
};


#endif //FFMPEGPLAYER_VIDEOPUSHCHANNEL_H
