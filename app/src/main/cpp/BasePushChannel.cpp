//
// Created by LUYI on 2019/10/28.
//

#include "BasePushChannel.h"
#include "macro.h"

void *task_start_live(void *args) {
    BasePushChannel *videoPushChannel = static_cast<BasePushChannel *>(args);
    videoPushChannel->_task_start();
    return 0;
}

void BasePushChannel::startLive(char* url){
    if(this->isPlaying){
        return;
    }
    this->isPlaying = 1;
    this->url = url;
    pthread_create(&pid_start,0, task_start_live, this);
}

void BasePushChannel::_task_start(){
    RTMP *rtmp = 0;
    do{
        rtmp = RTMP_Alloc();
        if(!rtmp){
            LOGE("Rtmp init failed!")
            break;
        }

        RTMP_Init(rtmp);
        rtmp->Link.timeout = 5; //seconds
        int ret = RTMP_SetupURL(rtmp, url);
        if(!ret){
            LOGE("Rtmp SetupURL failed!")
            break;
        }

        LOGE("Rtmp SetupURL OK! %s",url)

        RTMP_EnableWrite(rtmp);

        LOGE("RTMP_EnableWrite OK!")
        ret =  RTMP_Connect(rtmp,0);
        if(!ret){
            LOGE("Rtmp RTMP_Connect failed! %d", ret)
            break;
        }

        LOGE("RTMP_Connect OK!")
        ret= RTMP_ConnectStream(rtmp, 0);
        if(!ret){
            LOGE("Rtmp RTMP_ConnectStream failed! %d", ret)
            break;
        }
        LOGE("RTMP_ConnectStream OK!")


        //ready to push stream to server
        isConnected = 1;
        //Record the start time of pushing
        start_time = RTMP_GetTime();

        rtmpPackets.setWork(1);

        RTMPPacket *packet = 0;
        while(isConnected){
            rtmpPackets.pop(packet);
            LOGE("Pop up a rtmp packet")
            if(!isConnected){
                break;
            }
            if(!packet){
                continue;
            }
            packet->m_nInfoField2 = rtmp->m_stream_id;
            ret = RTMP_SendPacket(rtmp, packet, 1);
            if(!ret){
                LOGE("SEND PACKET ERROR")
            }
        }

        releasePacket(&packet);

    }while (0);
    isPlaying = 0;
    isConnected = 0;
/*    rtmpPackets.setWork(0);
    rtmpPackets.clear();*/
    if(rtmp){
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete(url);
}

void BasePushChannel::stopLive() {
    isPlaying = 0;
    isConnected = 0;
    rtmpPackets.setWork(0);
    pthread_join(pid_start, 0);
}

void BasePushChannel::pushPacket(RTMPPacket *pPacket) {
    if (pPacket) {
        pPacket->m_nTimeStamp = RTMP_GetTime() - start_time;
        rtmpPackets.push(pPacket);
    }
}

