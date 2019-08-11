//
// Created by LUYI on 2019/8/11.
//

#include <pthread.h>

#include "FFmpegCore.h"
#include "macro.h"

extern "C" {
#include <libavformat/avformat.h>
}

FFmpegCore::FFmpegCore(JavaCallHelper *javaCallHelper, char* path) {
    this->javaCallHelper = javaCallHelper;
    //Because the path will be released outside the method, so we need to copy the content.
    //C string end with '\0', such as "path\0", so we need to plus 1.
    this->dataSource = new char[strlen(path)+1];
    strcpy(this->dataSource, path);
}

FFmpegCore::~FFmpegCore() {
    DELETE(dataSource);
    DELETE(javaCallHelper)
}

void *task_prepare(void* args){
    FFmpegCore *fFmpegCore = static_cast<FFmpegCore *>(args);
    fFmpegCore -> _prepare();
    return 0;
}

void FFmpegCore::_prepare(){
    AVFormatContext* formatContext = avformat_alloc_context();
    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary, "timeout", "1000000", 0);
    int ret = avformat_open_input(&formatContext,dataSource, 0, &dictionary);
    av_dict_free(&dictionary);
    if(ret){
        //fail
        //LOGE("Open media failed! $s",av_err2str(ret));

    }
};

void FFmpegCore::prepare() {
    //create new thread to load file or stream
    pthread_create(&pid_prepare, 0,task_prepare ,this);
}
