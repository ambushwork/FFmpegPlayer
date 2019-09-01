//
// Created by LUYI on 2019/8/11.
//

#include "FFmpegCore.h"

FFmpegCore::FFmpegCore(JavaCallHelper *javaCallHelper, char* path) {
    this->javaCallHelper = javaCallHelper;
    //Because the path will be released outside the method, so we need to copy the content.
    //C string end with '\0', such as "path\0", so we need to plus 1.
    this->dataSource = new char[strlen(path)+1];
    strcpy(this->dataSource, path);
}

FFmpegCore::~FFmpegCore() {
    DELETE(dataSource);
    DELETE(javaCallHelper);
}

void *task_prepare(void* args){
    FFmpegCore *fFmpegCore = static_cast<FFmpegCore *>(args);
    fFmpegCore -> _prepare();
    return 0;
}

void *task_start(void* args){
    FFmpegCore *fFmpegCore = static_cast<FFmpegCore *>(args);
    fFmpegCore -> _start();
    return 0;
}

void FFmpegCore::_start(){
    while(isPlaying){
        // memory leak point
        if(videoChannel -> packets.size() > 100){
            av_usleep(10 * 1000);
            continue;
        }
        AVPacket *avPacket =av_packet_alloc();
        int ret = av_read_frame(formatContext, avPacket);
        if(!ret){
            //Thread safe queue
            //LOGE("av_read_frame OK")
            if(videoChannel && avPacket-> stream_index == videoChannel->id){
                //LOGE("video channel push packets OK")
                videoChannel->packets.push(avPacket);
            } else if(audioChannel && avPacket->stream_index == audioChannel -> id){
                audioChannel->packets.push(avPacket);
            }
        } else if( ret == AVERROR_EOF){
            //LOGE("av_read_frame FINISH")
        } else {
            //LOGE("av_read_frame FAIL")
        }

    }

    isPlaying = 0;

    videoChannel -> stop();
    audioChannel -> stop();

}

void FFmpegCore::_prepare(){
    formatContext = avformat_alloc_context();
    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary, "timeout", "1000000", 0);
    int ret = avformat_open_input(&formatContext,dataSource, 0, &dictionary);
    av_dict_free(&dictionary);
    if(ret){
        //fail
        //LOGE("Open media failed! $s",av_err2str(ret));

    }
    ret = avformat_find_stream_info(formatContext, 0);
    if(ret < 0){
        //todo
        return;
    }
    int video_stream_index= -1;
    int audio_stream_index= -1;
    int fps = -1;
    for(int i = 0; i < formatContext -> nb_streams; ++i){
        //Get codec parameter then get codec type
        if(formatContext -> streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            AVRational frame_rate =  formatContext->streams[i]->avg_frame_rate;
            fps = av_q2d(frame_rate);

        } else if(formatContext -> streams[i] ->codecpar ->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_index = i;
        }
    }

    AVCodecParameters *codecpar = formatContext -> streams[video_stream_index]->codecpar;

    //Decoder h264
    AVCodec *dec = avcodec_find_decoder(codecpar ->codec_id);

    //Context of decoder
    AVCodecContext *codecContext = avcodec_alloc_context3(dec);
    //Copy codec parameters to context
    ret = avcodec_parameters_to_context(codecContext, codecpar);
    audioChannel = new AudioChannel(audio_stream_index,codecContext);
    videoChannel = new VideoChannel(video_stream_index,codecContext, fps);
    videoChannel->setRenderCallback(renderCallback);

    if(ret < 0){
        //todo
        return;
    }

    ret = avcodec_open2(codecContext, dec, 0);

    if(ret < 0){
        //todo
        return;
    }



    javaCallHelper->onPrepared(THREAD_CHILD);


};

void FFmpegCore::prepare() {
    //create new thread to load file or stream
    pthread_create(&pid_prepare, 0,task_prepare ,this);
}

void FFmpegCore::start() {
    isPlaying = 1;
    if(videoChannel){
        LOGE("FFmpegCore -> start")
        videoChannel->start();
    }
    pthread_create(&pid_start, 0,task_start ,this);
}

void FFmpegCore::setRenderCallback(RenderCallback renderCallback){
    this->renderCallback = renderCallback;
}
