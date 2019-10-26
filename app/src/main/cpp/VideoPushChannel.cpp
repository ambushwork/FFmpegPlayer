//
// Created by LUYI on 2019/10/16.
//


#include <pthread.h>
#include "librtmp/rtmp.h"
#include "VideoPushChannel.h"
#include <x264.h>
#include "macro.h"

void *task_start_live(void *args) {
    VideoPushChannel *videoPushChannel = static_cast<VideoPushChannel *>(args);
    videoPushChannel->_task_start();
    return 0;
}

void VideoPushChannel::startLive(char* url){
    if(this->isPlaying){
        return;
    }
    this->isPlaying = 1;
    this->url = url;
    pthread_create(&pid_start,0, task_start_live, this);
}

void VideoPushChannel::_task_start(){
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

        //releasePacket(&packet);

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



void VideoPushChannel::stopLive() {
    isPlaying = 0;
    isConnected = 0;
    rtmpPackets.setWork(0);
    pthread_join(pid_start, 0);
}

//init x264 encoder
void VideoPushChannel::initVideoEncoder(int w, int h, int fps, int bitrate) {

    pthread_mutex_lock(&mutex);
    mWidth = w;
    mHeight = h;
    mFps = fps;
    mBitrate = bitrate;

    y_len = w * h;
    uv_len = y_len / 4;

 /*   if(videoEncoder){
        *//*x264_encoder_close(videoEncoder);
        videoEncoder = 0;*//*
    }

    if(pic_in){
        x264_picture_clean(pic_in);
        DELETE(pic_in)
    }*/

    x264_param_t param;
    x264_param_default_preset(&param, "ultrafast", "zerolatency");

    //codec -rule : related with video quality
    param.i_level_idc = 32;

    //input format
    param.i_csp = X264_CSP_I420;

    param.i_width = w;
    param.i_height = h;

    //no b frame - B frame will affect efficiency of encoder
    param.i_bframe = 0;

    //CQP(stable quality)  CRF(stable bitrate) ABR(avarage bitrate)
    param.rc.i_rc_method = X264_RC_CRF;

    param.rc.i_bitrate = bitrate/1000;

    //max bitrate
    param.rc.i_vbv_max_bitrate = bitrate / 1000 *1.2;
    param.rc.i_vbv_buffer_size = bitrate / 1000;
    //bitrate control by fps instead of timebase and timestamp
    param.b_vfr_input = 0;

    param.i_fps_num = fps;

    param.i_fps_den = 1;
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;

    //key frame interval
    param.i_keyint_max = fps * 2;

    param.b_repeat_headers = 1;

    param.i_threads = 1;

    x264_param_apply_profile(&param, "baseline");


    pic_in = new x264_picture_t;
    x264_picture_alloc(pic_in, param.i_csp, param.i_width, param.i_height);


    videoEncoder = x264_encoder_open(&param);
    if(videoEncoder){
        LOGE("x264 encoder open!")
    }
    pthread_mutex_unlock(&mutex);
}

void VideoPushChannel::encodeData(int8_t *data) {
    pthread_mutex_lock(&mutex);
    //nv21 -> I420
    //y data
    memcpy(pic_in->img.plane[0], data, y_len);
    for(int i =0; i <uv_len; ++i){
        //u data
        *(pic_in->img.plane[1] +i) = *(data + y_len + i*2 +1);
        //v data
        *(pic_in->img.plane[2] +i) = *(data + y_len + i*2);
    }
    x264_nal_t *nal = 0;
    int pi_nal;
    x264_picture_t pic_out;

    int ret = x264_encoder_encode(videoEncoder, &nal, &pi_nal, pic_in, &pic_out);

    if(ret < 0){
        LOGE("x264 encoder fail")
        pthread_mutex_unlock(&mutex);
        return;
    }

    //sps pps

    int sps_len, pps_len;
    uint8_t sps[100];
    uint8_t pps[100];
    pic_in->i_pts +=1;
    for(int i =0; i < pi_nal; ++i){
        if(nal[i].i_type == NAL_SPS){
            sps_len = nal[i].i_payload - 4;
            memcpy(sps, nal[i].p_payload + 4, sps_len);
        }  else if(nal[i].i_type == NAL_PPS){
            pps_len = nal[i].i_payload - 4;
            memcpy(pps, nal[i].p_payload + 4, pps_len);
            sendSpsPps(sps,pps, sps_len,pps_len);
        } else {
            //frame type
            sendFrame(nal[i].i_type,nal[i].i_payload,nal[i].p_payload);
        }
    }

    pthread_mutex_unlock(&mutex);
}

void VideoPushChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {
    RTMPPacket *packet = new RTMPPacket;
    int body_size = 5 + 8 + sps_len + 3 + pps_len;
    RTMPPacket_Alloc(packet, body_size);

    int i = 0;
    packet -> m_body[i++] = 0x17;

    packet -> m_body[i++] = 0x00;
    packet -> m_body[i++] = 0x00;
    packet -> m_body[i++] = 0x00;
    packet -> m_body[i++] = 0x00;

    packet -> m_body[i++] = 0x01;

    packet -> m_body[i++] = sps[1];
    packet -> m_body[i++] = sps[2];
    packet -> m_body[i++] = sps[3];

    packet -> m_body[i++] = 0xFF;
    packet -> m_body[i++] = 0xE1;

    packet -> m_body[i++] = (sps_len >> 8);
    packet -> m_body[i++] = (sps_len & 0xFF);

    memcpy(&packet->m_body[i], sps, sps_len);
    i+= sps_len;


    packet -> m_body[i++] = 0x01;

    packet -> m_body[i++] = (pps_len >> 8);
    packet -> m_body[i++] = (pps_len & 0xFF);
    memcpy(&packet->m_body[i], pps, pps_len);
    i+= pps_len;


    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 10;
    packet->m_nTimeStamp = 0; //sps pps packet doesn't have timestamp
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;

    //put data into queue
    if (packet) {
        LOGE("Push SpsPPs")
        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
        rtmpPackets.push(packet);
    }


}

void VideoPushChannel::sendFrame(int type, int payload, uint8_t *p_payload) {
    //remove start code
    if(p_payload[2] = 0x00){
        p_payload +=4;
        payload -= 4;
    } else if(p_payload[2] = 0x01){
        p_payload +=3;
        payload -= 3;
    }

    RTMPPacket *packet = new RTMPPacket;
    int body_size = 5 + 4 + payload;
    RTMPPacket_Alloc(packet, body_size);

    packet -> m_body[0] = 0x27;
    if(type == NAL_SLICE_IDR){
        packet ->m_body[0] = 0x17;
    }

    packet -> m_body[1] = 0x01;
    packet -> m_body[2] = 0x00;
    packet -> m_body[3] = 0x00;
    packet -> m_body[4] = 0x00;

    packet -> m_body[5] = (payload >> 24)& 0xFF;

    packet -> m_body[6] = (payload >> 16)& 0xFF;
    packet -> m_body[7] = (payload >> 8)& 0xFF;
    packet -> m_body[8] = (payload & 0xFF);

    memcpy(&packet->m_body[9], p_payload, payload);


    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 10;
    //packet->m_nTimeStamp = -1;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;


    packet->m_nTimeStamp = RTMP_GetTime() - start_time;
    //put data into queue
    LOGE("Push frame")
    rtmpPackets.push(packet);

}

