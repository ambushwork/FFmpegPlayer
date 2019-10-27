//
// Created by LUYI on 2019/10/28.
//

#ifndef FFMPEGPLAYER_BASEPUSHCHANNEL_H
#define FFMPEGPLAYER_BASEPUSHCHANNEL_H

#include "librtmp/rtmp.h"
#include "safe_queue.h"
#include "macro.h"
class BasePushChannel {
public:
    BasePushChannel(){
        rtmpPackets.setReleaseCallback(releasePacket);
        pthread_mutex_init(&mutex, 0);
    }
    ~BasePushChannel(){
        pthread_mutex_destroy(&mutex);
    }
    bool isConnected = 0;
    uint32_t start_time=0;
    pthread_t pid_start;
    SafeQueue<RTMPPacket *> rtmpPackets;
    pthread_mutex_t mutex;
    char* url;
    static void releasePacket(RTMPPacket **packet){
        if(packet){
            RTMPPacket_Free(*packet);
            delete  packet;
            packet =0;
        }
    }

    bool isPlaying = 0;

    void startLive(char* url);

    void stopLive();

    void _task_start();

    void pushPacket(RTMPPacket *pPacket);
};


#endif //FFMPEGPLAYER_BASEPUSHCHANNEL_H
