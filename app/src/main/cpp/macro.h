//
// Created by LUYI on 2019/8/11.
//

#ifndef FFMPEGPLAYER_MACRO_H
#define FFMPEGPLAYER_MACRO_H

#include <android/log.h>

#define THREAD_MAIN 1
#define THREAD_CHILD 2

#define DELETE(object) if(object){delete object; object = 0;}

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "FFMPEG",__VA_ARGS__);

#endif //FFMPEGPLAYER_MACRO_H
