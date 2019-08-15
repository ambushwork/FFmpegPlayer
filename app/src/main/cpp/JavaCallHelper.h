//
// Created by LUYI on 2019/8/11.
//

#include <jni.h>

#ifndef FFMPEGPLAYER_JAVACALLHELPER_H
#define FFMPEGPLAYER_JAVACALLHELPER_H

#endif //FFMPEGPLAYER_JAVACALLHELPER_H

class JavaCallHelper{
public:
    JavaCallHelper(JavaVM *javaVM_,JNIEnv *env_, jobject instance_);
    ~JavaCallHelper();
private:
    JavaVM *javaVM;
    JNIEnv *jniEnv;
    jobject  instance;
    jmethodID jmd_prepared;

public:
    void onPrepared(int i);
};