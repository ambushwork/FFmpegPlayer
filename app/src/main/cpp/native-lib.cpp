#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <zconf.h>
#include <android/log.h>
#include "FFmpegCore.h"
#include "librtmp/rtmp.h"
#include "VideoPushChannel.h"
#include "librtmp/rtmp.h"
#include "x264.h"
#include "AudioPushChannel.h"


#define MAX_AUDIO_FRME_SIZE 48000 *4
extern "C" {
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    char version[50];
    sprintf(version, "librtmp version :%d", RTMP_LibVersion());
    x264_picture_t *picture = new x264_picture_t;
    return env->NewStringUTF(version);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_native_1start(JNIEnv *env, jobject instance, jstring path_,
                                                             jobject surface) {
    const char *path = env->GetStringUTFChars(path_, 0);


    //Draw video
    __android_log_write(ANDROID_LOG_DEBUG, "FFmpeg", "Start draw video");
    //************* FFmpeg init part ************************
    //init network module of ffmpeg if needs to play live video
    avformat_network_init();
    //General context
    __android_log_write(ANDROID_LOG_DEBUG, "FFmpeg", "General context");
    AVFormatContext* formatContext = avformat_alloc_context();

    AVDictionary *opts = NULL;

    av_dict_set(&opts, "timeout", "3000000", 0);

    int ret = avformat_open_input(&formatContext, path, NULL, &opts);

    if(ret){
        __android_log_print(ANDROID_LOG_DEBUG, "FFmpeg", "open input error %s", av_err2str(ret));
       return;
    }
    __android_log_write(ANDROID_LOG_DEBUG, "FFmpeg", "file is open");

   //************** Get video stream ***********************
    int video_stream_index= -1;
    avformat_find_stream_info(formatContext, NULL);
    for(int i = 0; i < formatContext -> nb_streams; ++i){
        //Get codec parameter then get codec type
        if(formatContext -> streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            break;
        }
    }
    __android_log_print(ANDROID_LOG_DEBUG, "FFmpeg", "video stream found %d", video_stream_index);
    AVCodecParameters *codecpar = formatContext -> streams[video_stream_index]->codecpar;

    //Decoder h264
    AVCodec *dec = avcodec_find_decoder(codecpar ->codec_id);
    //Context of decoder
    AVCodecContext *codecContext = avcodec_alloc_context3(dec);
    //Copy codec parameters to context
    avcodec_parameters_to_context(codecContext, codecpar);
    __android_log_print(ANDROID_LOG_DEBUG, "FFmpeg", "Avcodec found");

   avcodec_open2(codecContext, dec, NULL);

   //Create an empty container
   AVPacket *packet = av_packet_alloc();

    __android_log_print(ANDROID_LOG_DEBUG, "FFmpeg", "empty container created");


    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    if(nativeWindow == nullptr){
        __android_log_print(ANDROID_LOG_DEBUG, "FFmpeg", "nativeWindow null");
    }
    ANativeWindow_setBuffersGeometry(nativeWindow, codecContext->width,
            codecContext->height,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer outBuffer;
    __android_log_print(ANDROID_LOG_DEBUG, "FFmpeg", "Reading data from video stream");
   //Read a data packet from the video stream, the data is still compressed
   while(av_read_frame(formatContext, packet) >=0 ){
       //Extract the packet from the queue.
       avcodec_send_packet(codecContext, packet);

       //Packet -> Frame
       AVFrame *frame = av_frame_alloc();
       ret = avcodec_receive_frame(codecContext, frame);
       if(ret == AVERROR(EAGAIN)){
           continue;
       } else if (ret < 0){
           break;
       }

       //the video has ended or has error

       //************* Draw frame data on the surface view **************************
       //YUV -> RGB, SWS: parameter of transform(quality or speed).
       SwsContext * swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                                                codecContext->width, codecContext->height,AV_PIX_FMT_RGBA, SWS_FAST_BILINEAR, 0, 0, 0);
       //The container to receive data. 4 = R + G + B + A
       uint8_t *dst_data[4];
       //The header address of every line
       int dst_linesize[4];
       av_image_alloc(dst_data, dst_linesize, codecContext -> width, codecContext ->height, AV_PIX_FMT_RGBA, 1);

       ANativeWindow_lock(nativeWindow, &outBuffer, NULL);

       sws_scale(swsContext, frame->data, frame->linesize, 0 , frame ->height, dst_data, dst_linesize);

       uint8_t *dst= (uint8_t *) outBuffer.bits;
       int destStride=outBuffer.stride*4;
       uint8_t *src_data = dst_data[0];
       int src_linesize = dst_linesize[0];
       uint8_t *firstWindown = static_cast<uint8_t *>(outBuffer.bits);




       //Copy RGB data from dst_data to native window data buffer
       for(int i=0; i<outBuffer.height; i++){
           memcpy(firstWindown + i * destStride, src_data + i * src_linesize, destStride);
       }


       ANativeWindow_unlockAndPost(nativeWindow);
       usleep(1000 * 16);
       av_frame_free(&frame);


   }
   ANativeWindow_release(nativeWindow);
   avcodec_close(codecContext);
   avformat_free_context(formatContext);
    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_native_1sound(JNIEnv *env, jobject instance, jstring input_,
                                                             jstring output_) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);


    avformat_network_init();
    AVFormatContext *formatContext = avformat_alloc_context();
    if(avformat_open_input(&formatContext, input, NULL,NULL)){
        __android_log_write(ANDROID_LOG_DEBUG, "FFmpeg mp3 player","cannot open mp3 file");
        return;
    }
    if(avformat_find_stream_info(formatContext, NULL)){
        __android_log_write(ANDROID_LOG_DEBUG, "FFmpeg mp3 player","cannot get mp3 stream info");
        return;
    }

    int audio_stream_index= -1;
    for(int i = 0; i < formatContext -> nb_streams; ++i){
        //Get codec parameter then get codec type
        if(formatContext -> streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_index = i;
            break;
        }
    }
    AVCodecParameters *codecParameters = formatContext->streams[audio_stream_index]->codecpar;
    AVCodec *dec = avcodec_find_decoder(codecParameters->codec_id);
    AVCodecContext *codecContext = avcodec_alloc_context3(dec);
    avcodec_parameters_to_context(codecContext,codecParameters);

    SwrContext *swrContext = swr_alloc();

    //Input sample
    AVSampleFormat in_sample = codecContext->sample_fmt;
    int in_sample_rate = codecContext->sample_rate;
    uint64_t in_ch_layout = codecContext->channel_layout;

    //Output sample (fixed)
    AVSampleFormat  out_sample = AV_SAMPLE_FMT_S16;
    int out_sample_rate = 44100;
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(swrContext, out_ch_layout, out_sample, out_sample_rate,
                       in_ch_layout, in_sample, in_sample_rate, 0 , NULL);

    swr_init(swrContext);

    //Buffer size = channel count * sample rate
    uint8_t *out_buffer = static_cast<uint8_t *>(av_malloc(2 * 44100));

    //Open mp3 file
    FILE *fp_pcm = fopen(output, "wb");

    AVPacket *packet = av_packet_alloc();

    while(av_read_frame(formatContext, packet)>=0){
        avcodec_send_packet(codecContext, packet);
        AVFrame *frame = av_frame_alloc();
        int ret = avcodec_receive_frame(codecContext, frame);
        if(ret ==  AVERROR(EAGAIN)){
            continue;
        } else {
            __android_log_write(ANDROID_LOG_DEBUG, "FFmpeg mp3 player","decode finished!");
        }
        if(packet->stream_index != audio_stream_index){
            continue;
        }


        //frame to unified format
        swr_convert(swrContext, &out_buffer, 2* 44100, (const uint8_t **)frame->data, frame->nb_samples);

        int out_channel_nb =  av_get_channel_layout_nb_channels(out_ch_layout);
        //out_buffer-->File
        int out_buffer_size = av_samples_get_buffer_size(NULL,out_channel_nb,frame->nb_samples,out_sample,1);
        fwrite(out_buffer, 1, out_buffer_size, fp_pcm);
    }
    fclose(fp_pcm);
    av_free(out_buffer);
    swr_free(&swrContext);
    avcodec_close(codecContext);
    avformat_close_input(&formatContext);



    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}

JavaVM *javaVM;
jint JNI_OnLoad(JavaVM *vm, void *reserved){
    javaVM = vm;
    return JNI_VERSION_1_4;
}


FFmpegCore *fFmpegCore;
ANativeWindow *window = 0;
pthread_mutex_t mutex = PTHREAD_COND_INITIALIZER;

void renderFrame(uint8_t *src_data, int lineSize, int width, int height){
    pthread_mutex_lock(&mutex);
    if(!window){
        LOGE("window nullptr");
        pthread_mutex_unlock(&mutex);
        return;
    }

    ANativeWindow_setBuffersGeometry(window, width, height ,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer outBuffer;
    int ret = ANativeWindow_lock(window, &outBuffer,0);
    if(ret){
        LOGE("render frame %d", ret);
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    //assign data to buffer
    uint8_t  *dst_data = static_cast<uint8_t  *>(outBuffer.bits);
    // The number of *pixels* that a line in the buffer takes in
    // memory. This may be >= width.
    //ARGB needs to multiple 4
    int dst_lineSize = outBuffer.stride * 4;

    for(int i = 0; i<outBuffer.height; ++i){
        memcpy(dst_data + i*dst_lineSize, src_data + i*lineSize, dst_lineSize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_native_1prepare(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    JavaCallHelper *helper =new  JavaCallHelper(javaVM,env, instance);
    fFmpegCore = new FFmpegCore(helper, const_cast<char *>(path));
    fFmpegCore -> setRenderCallback(renderFrame);
    fFmpegCore -> prepare();
    env->ReleaseStringUTFChars(path_, path);
}



extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_native_1start2(JNIEnv *env, jobject instance) {

    if(fFmpegCore){
        fFmpegCore -> start();
    }

}


extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_setSufaceNative(JNIEnv *env, jobject instance, jobject surface) {
    pthread_mutex_lock(&mutex);
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);

    pthread_mutex_unlock(&mutex);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_releaseNative(JNIEnv *env, jobject instance) {

    pthread_mutex_lock(&mutex);

    if(window){
        ANativeWindow_release(window);
        window = 0;
    }
    pthread_mutex_unlock(&mutex);
    DELETE(fFmpegCore)

}extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_stopNative(JNIEnv *env, jobject instance) {

    if(fFmpegCore){
        fFmpegCore->stop();
    }
}extern "C"
JNIEXPORT jint JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_getDurationNative(JNIEnv *env, jobject instance) {


    if(fFmpegCore){
        return fFmpegCore->getDuration();
    }
    return 0;

}extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_seekToNative(JNIEnv *env, jobject instance, jint playProgress) {

    if(fFmpegCore){
        return fFmpegCore->seekTo(playProgress);
    }

}

VideoPushChannel *videoPushChannel;
AudioPushChannel *audioPushChannel;
BasePushChannel *basePushChannel;


extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_PushManager_native_1init(JNIEnv *env, jobject instance) {

    //encoder
    basePushChannel = new BasePushChannel;
    videoPushChannel = new VideoPushChannel(basePushChannel);
    audioPushChannel = new AudioPushChannel(basePushChannel);


}

extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_PushManager_native_1start_1live(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    char *url = new char(strlen(path) + 1);
    strcpy(url, path);

    //create thread to push live
    basePushChannel->startLive(url);

    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_PushManager_native_1stop(JNIEnv *env, jobject instance) {

    basePushChannel->stopLive();

}extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_PushManager_native_1pushVideo(JNIEnv *env, jobject instance, jbyteArray data_) {
    jbyte *data = env->GetByteArrayElements(data_, NULL);

    if(!videoPushChannel || !basePushChannel->isConnected){
        return;
    }

    videoPushChannel->encodeData(data);
    jbyte *data1= env->GetByteArrayElements(data_,NULL);
    env->ReleaseByteArrayElements(data_,data1, 0);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_PushManager_native_1initVideoEncoder(JNIEnv *env, jobject instance, jint w, jint h,
                                                                       jint fps, jint bitrate) {

    if(videoPushChannel){
        videoPushChannel->initVideoEncoder(w,h,fps, bitrate);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_PushManager_native_1initAudioEncoder(JNIEnv *env, jobject thiz,
                                                                       jint sample_rate,
                                                                       jint num_channels) {
    if(audioPushChannel){
        audioPushChannel->initAudioEncoder(sample_rate, num_channels);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_netatmo_ylu_ffmpegplayer_PushManager_getInputSamples(JNIEnv *env, jobject thiz) {
    if (audioPushChannel) {
        return audioPushChannel->getInputSamples();
    }
    return -1;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_PushManager_native_1pushAudio(JNIEnv *env, jobject thiz,
                                                                jbyteArray bytes) {
    if(!audioPushChannel|| !basePushChannel->isConnected){
        return;
    }

    jbyte *data1= env->GetByteArrayElements(bytes,NULL);
    audioPushChannel->encodeData(data1);
    env->ReleaseByteArrayElements(bytes,data1, 0);
}