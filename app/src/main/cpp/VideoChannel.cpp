//
// Created by LUYI on 2019/8/11.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel(int id,AVCodecContext* avCodecContext) :BaseChannel(id, avCodecContext){}

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
