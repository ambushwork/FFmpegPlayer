//
// Created by LUYI on 2019/8/11.
//

#include "JavaCallHelper.h"
#include "macro.h"

JavaCallHelper::JavaCallHelper(JavaVM *javaVM_, JNIEnv *env_, jobject instance_) {
    this->javaVM = javaVM_;
    this->jniEnv = env_;
    //can not directly assign, because instance has been used across threads.
    //needs to create global reference
    this->instance = instance_;
    this->instance = env_->NewGlobalRef(instance_);
    jclass  clazz = env_->GetObjectClass(instance_);
    //Get method id by class instance, class name and class signature
    //to get class signature, go into the class directory and use "javap -s <classname>"
    jmd_prepared  = env_->GetMethodID(clazz,"onPrepare","()V");
}

JavaCallHelper::~JavaCallHelper() {
    javaVM = 0;
    jniEnv->DeleteGlobalRef(instance);
    instance = 0;
}

void JavaCallHelper::onPrepared(int threadMode) {
    if(threadMode == THREAD_MAIN){
        jniEnv->CallVoidMethod(instance,jmd_prepared);
    } else {
        JNIEnv *env_child;
        javaVM->AttachCurrentThread( &env_child, 0);
        env_child->CallVoidMethod(instance,jmd_prepared);
        javaVM->DetachCurrentThread();
    }
}
