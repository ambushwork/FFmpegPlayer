//
// Created by LUYI on 2019/10/16.
//

#include "VideoPushChannel.h"
#include "../../../../../../Android/SDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/pthread.h"
#include "macro.h"

VideoPushChannel::VideoPushChannel() {

}

void VideoPushChannel::startLive(char* url){
    pthread_create(&pid_start,0, task_start, url);
}

void *VideoPushChannel::task_start(void *args) {
    char *url = static_cast<char *>(args);
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

        RTMP_EnableWrite(rtmp);

        ret =  RTMP_Connect(rtmp,0);
        if(!ret){
            LOGE("Rtmp RTMP_Connect failed!")
            break;
        }
        ret= RTMP_ConnectStream(rtmp, 0);
        if(!ret){
            LOGE("Rtmp RTMP_ConnectStream failed!")
            break;
        }


        //ready to push stream to server
        isConnected = 1;
        //Record the start time of pushing
        start_time = RTMP_GetTime();

        rtmpPackets.setWork(1);

        RTMPPacket *packet = 0;
        while(isConnected){
            rtmpPackets.pop(packet);
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
    if(rtmp){
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete(url);
    return 0;
}

void VideoPushChannel::stopLive() {
    isPlaying = 0;
    isConnected = 0;
    rtmpPackets.setWork(0);
    pthread_join(pid_start, 0);
}
