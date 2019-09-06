//
// Created by LUYI on 2019/8/11.
//

#ifndef FFMPEGPLAYER_BASECHANNEL_H
#define FFMPEGPLAYER_BASECHANNEL_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
};

#include "safe_queue.h"

class BaseChannel{
public:
    BaseChannel(int id,  AVCodecContext* formatContext,AVRational time_base):id(id),
    avCodecContext(formatContext),
    time_base(time_base){
        packets.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    }

    virtual ~BaseChannel(){
        packets.clear();
        frames.clear();
    }

    static void releaseAVPacket(AVPacket **packet){
        if(packet){
            av_packet_free(packet);
            *packet = 0;
        }
    }

    static void releaseAVFrame(AVFrame **frame){
        if(frame){
            av_frame_free(frame);
            *frame = 0;
        }
    }

    virtual void start() = 0;
    virtual void stop() = 0;

    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    int id;
    bool isPlaying = 0;
    AVCodecContext* avCodecContext = 0;
    AVRational time_base;
};


#endif //FFMPEGPLAYER_BASECHANNEL_H