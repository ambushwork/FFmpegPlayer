#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <zconf.h>
#include <android/log.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_netatmo_ylu_ffmpegplayer_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(av_version_info());
}


extern "C"
JNIEXPORT void JNICALL
Java_com_netatmo_ylu_ffmpegplayer_FFmpegPlayer_native_1start(JNIEnv *env, jobject instance, jstring path_,
                                                             jobject surface) {
    const char *path = env->GetStringUTFChars(path_, 0);

    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    //Draw video
    __android_log_write(ANDROID_LOG_ERROR, "FFmpeg", "Start draw video");
    //************* FFmpeg init part ************************
    //init network module of ffmpeg if needs to play live video
    avformat_network_init();
    //General context
    __android_log_write(ANDROID_LOG_ERROR, "FFmpeg", "General context");
    AVFormatContext* formatContext = avformat_alloc_context();

    AVDictionary *opts = NULL;

    av_dict_set(&opts, "timeout", "3000000", 0);

    int ret = avformat_open_input(&formatContext, path, NULL, &opts);

    if(ret){
        __android_log_print(ANDROID_LOG_ERROR, "FFmpeg", "open input error %s", av_err2str(ret));
       return;
    }
    __android_log_write(ANDROID_LOG_ERROR, "FFmpeg", "open input");

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
       AVCodecParameters *codecpar = formatContext -> streams[video_stream_index]->codecpar;

       //Decoder h264
       AVCodec *dec = avcodec_find_decoder(codecpar ->codec_id);
       //Context of decoder
       AVCodecContext *codecContext = avcodec_alloc_context3(dec);
       //Copy codec parameters to context
       avcodec_parameters_to_context(codecContext, codecpar);


       avcodec_open2(codecContext, dec, NULL);

       //Create an empty container
       AVPacket *packet = av_packet_alloc();

       ANativeWindow_setBuffersGeometry(nativeWindow, codecContext->width, codecContext->height,WINDOW_FORMAT_RGBA_8888);
       ANativeWindow_Buffer outBuffer;
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