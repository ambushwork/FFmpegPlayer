//
// Created by LUYI on 2019/8/11.
//

#include "VideoChannel.h"

void dropAVPacket(queue<AVPacket *> &q){
    while(!q.empty()){
        AVPacket *avPacket = q.front();
        //I frame, B frame, P frame
        //Can not drop I frame, I frame is key frame
        if(avPacket->flags != AV_PKT_FLAG_KEY){
            BaseChannel::releaseAVPacket(&avPacket);
            q.pop();
        } else {
            break;
        }
    }
}


void dropAVFrame(queue<AVFrame *> &q){
    if(!q.empty()){
        AVFrame *avFrame = q.front();
        BaseChannel::releaseAVFrame(&avFrame);
        q.pop();
    }
}


VideoChannel::VideoChannel(int id,AVCodecContext* avCodecContext,int fps,AVRational time_base) :BaseChannel(id, avCodecContext, time_base){
    this->fps = fps;
    packets.setSyncHandler(dropAVPacket);
    frames.setSyncHandler(dropAVFrame);
}

VideoChannel::~VideoChannel() {

}

void *task_video_decode(void *args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel -> video_decode();
    return 0;
}

void *task_video_play(void *args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel -> video_play();
    return 0;
}

void VideoChannel::start(){
    isPlaying = 1;
    packets.setWork(1);
    frames.setWork(1);
    pthread_create(&pid_video_decode, 0, task_video_decode, this);

    pthread_create(&pid_video_start, 0, task_video_play, this);
};



void VideoChannel::stop(){

}

void VideoChannel::video_decode() {
    AVPacket *packet = 0;
    while(isPlaying){
        //LOGE("video_decode loop")
        int ret = packets.pop(packet);
        if(!ret){
            //fail
            //LOGE("video_decode packet data fail")
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        //LOGE("video_decode avcodec_send_packet OK")
        if(ret){
            //LOGE("avcodec_send_packet FAIL %s", av_err2str(ret));
            //Fail to send packet
            break;
        }
        releaseAVPacket(&packet);
        AVFrame *frame = av_frame_alloc();
        ret =  avcodec_receive_frame(avCodecContext, frame);
        if(ret == AVERROR(EAGAIN)){
            //Restart
            //LOGE("avcodec_receive_frame RESTART")
            continue;
        } else if(ret){
            //LOGE("avcodec_receive_frame FAIL")
            break;
        }
        // memory leak point
        while(isPlaying && frames.size() > 100){
            av_usleep(10 *1000);
        }
        //LOGE("video_decode push frames OK")
        frames.push(frame);
    }
    releaseAVPacket(&packet);

}

void VideoChannel::video_play() {
    AVFrame *frame = 0;
    //yuv -> rgba
    SwsContext * swsContext = sws_getContext(avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
                                             avCodecContext->width, avCodecContext->height,
                                             AV_PIX_FMT_RGBA, SWS_FAST_BILINEAR, 0, 0, 0);

    uint8_t *dst_data[4];
    //The header address of every line
    int dst_linesize[4];
    av_image_alloc(dst_data, dst_linesize, avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA, 1);

    //according to fps to control the delay of every frame
    double delay_time_per_frame = 1.0/fps;
    while (isPlaying){
        //LOGE("video_play loop")
        int ret = frames.pop(frame);
        if(!ret){
            //LOGE("Fail to get frame data")
            continue;
        }
        //LOGE("video_play sws_scale")
        sws_scale(swsContext, frame->data,
                frame->linesize, 0 , avCodecContext->height,dst_data, dst_linesize);
        //every frame has its own extra delay (time of drawing the frame)
        double extra_delay = frame->repeat_pict / (2 * fps);
        double real_delay = extra_delay + delay_time_per_frame;
        //av_usleep(real_delay * 1000000);

        double video_time = frame->best_effort_timestamp * av_q2d(time_base);
        if(!audioChannel){
            av_usleep(real_delay * 1000000);
        } else {
            double audio_time = audioChannel->audio_time;
            //Get the video audio time diff
            double time_diff = video_time - audio_time;
            if(time_diff > 0){
                //video faster than audio, wait for audio
                if(time_diff > 1){
                    av_usleep(real_delay *2 * 1000000);
                } else {
                    av_usleep((real_delay + time_diff) * 1000000);
                }
            } else {
                //video slower than audio, catch audio (try to drop some packet)
                if(fabs(time_diff) >= (double) 0.05){
                    //before decode
                    //packets.sync();
                    //after decode
                    frames.sync();
                    continue;
                }
            }
        }


        renderCallback(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
        releaseAVFrame(&frame);
    }

    releaseAVFrame(&frame);
    isPlaying = 0;
    av_freep(&dst_data[0]);
    sws_freeContext(swsContext);
}

void VideoChannel::setRenderCallback(RenderCallback callback) {
        this->renderCallback = callback;
}


void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
}