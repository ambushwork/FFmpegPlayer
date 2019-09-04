//
// Created by LUYI on 2019/8/11.
//

#include "AudioChannel.h"
#include "macro.h"

AudioChannel::AudioChannel(int id,AVCodecContext* codecContext) : BaseChannel(id, codecContext) {
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sampleSize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sampleRate = 44100;
    // 2(通道数) * 2（16bit=2字节）*44100（采样率）
    out_buffers_size = out_channels * out_sampleSize * out_sampleRate;
    out_buffers = static_cast<uint8_t *>(malloc(out_buffers_size));
    memset(out_buffers, 0, out_buffers_size);
    swrContext = swr_alloc_set_opts(0,AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,
                                                out_sampleRate,
                                                avCodecContext->channel_layout,
                                                avCodecContext->sample_fmt,
                                                avCodecContext->sample_rate,
                                                0,
                                                0);
    //初始化重采样上下文
    swr_init(swrContext);
}

AudioChannel::~AudioChannel() {
    if(swrContext){
        swr_free(&swrContext);
        swrContext = 0;
    }
    if(out_buffers){
        DELETE(out_buffers);
    }
}



void *task_audio_decode(void *args){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel -> audio_decode();
    return 0;
}

void *task_audio_play(void *args){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel -> audio_play();
    return 0;
}

void AudioChannel::start() {
    isPlaying = 1;
    packets.setWork(1);
    frames.setWork(1);
    pthread_create(&pid_audio_decode, 0, task_audio_decode, this);
    pthread_create(&pid_audio_start, 0, task_audio_play, this);
}


void AudioChannel::stop() {

}

void AudioChannel::audio_decode() {
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

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void * context){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    int pcm_size = audioChannel->getPCM();
    if (pcm_size > 0) {
        (*bq)->Enqueue(bq, audioChannel->out_buffers, pcm_size);
    }
}

void AudioChannel::audio_play() {

    //step 1
    SLresult result;
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //step 2
    // 2.1 创建混音器：SLObjectItf outputMixObject
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0,
                                                 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
// 2.2 初始化混音器
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //不启用混响可以不用获取混音器接口
    // 获得混音器接口
    //result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
    //                                         &outputMixEnvironmentalReverb);
    //if (SL_RESULT_SUCCESS == result) {
    //设置混响 ： 默认。
    //SL_I3DL2_ENVIRONMENT_PRESET_ROOM: 室内
    //SL_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM : 礼堂 等
    //const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
    //(*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
    //       outputMixEnvironmentalReverb, &settings);
    //}

    //step 3 create player

    //创建buffer缓冲类型的队列 2个队列
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                       2};
    //pcm数据格式
    //SL_DATAFORMAT_PCM：数据格式为pcm格式
    //2：双声道
    //SL_SAMPLINGRATE_44_1：采样率为44100
    //SL_PCMSAMPLEFORMAT_FIXED_16：采样格式为16bit
    //SL_PCMSAMPLEFORMAT_FIXED_16：数据大小为16bit
    //SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT：左右声道（双声道）
    //SL_BYTEORDER_LITTLEENDIAN：小端模式
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                                   SL_BYTEORDER_LITTLEENDIAN};

//数据源 将上述配置信息放到这个数据源中
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    //3.2 配置音轨（输出）
//设置混音器
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
//需要的接口 操作队列的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
//3.3 创建播放器
    result = (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &audioSrc, &audioSnk, 1, ids, req);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
//3.4 初始化播放器：SLObjectItf bqPlayerObject
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
//3.5 获取播放器接口：SLPlayItf bqPlayerPlay
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //4.1 获取播放器队列接口：SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);

//4.2 设置回调 void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    // 5、设置播放器状态为播放状态
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    //6、手动激活回调函数
    bqPlayerCallback(bqPlayerBufferQueue, this);
}

/**
 * 获取pcm数据
 * @return 数据大小
 */
int AudioChannel::getPCM() {
    int pcm_data_size = 0;
    AVFrame *frame = 0;
    SwrContext *swrContext = swr_alloc_set_opts(0,AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,
                                                out_sampleRate,
                                                avCodecContext->channel_layout,
                                                avCodecContext->sample_fmt,
                                                avCodecContext->sample_rate,
                                                0,
                                                0);
    //初始化重采样上下文
    swr_init(swrContext);

    while (isPlaying) {
        int ret = frames.pop(frame);
        if (!isPlaying) {
            //如果停止播放了，跳出循环 释放packet
            break;
        }
        if (!ret) {
            //取数据包失败
            continue;
        }
        LOGE("音频播放中");
        //pcm数据在 frame中
        //这里获得的解码后pcm格式的音频原始数据，有可能与创建的播放器中设置的pcm格式不一样
        //重采样？example:resample

        //假设输入10个数据，有可能这次转换只转换了8个，还剩2个数据（delay）
        //断点：1024 * 48000

        //swr_get_delay: 下一个输入数据与下下个输入数据之间的时间间隔
        int64_t delay = swr_get_delay(swrContext, frame->sample_rate);

        //a * b / c
        //AV_ROUND_UP：向上取整
        int64_t out_max_samples = av_rescale_rnd(frame->nb_samples + delay, frame->sample_rate,
                                                 out_sampleRate,
                                                 AV_ROUND_UP);

        //上下文
        //输出缓冲区
        //输出缓冲区能容纳的最大数据量
        //输入数据
        //输入数据量
        int out_samples = swr_convert(swrContext, &out_buffers, out_max_samples,
                                      (const uint8_t **) (frame->data), frame->nb_samples);

        // 获取swr_convert转换后 out_samples个 *2 （16位）*2（双声道）
        pcm_data_size = out_samples * out_sampleSize * out_channels;
        break;

    }//end while
    releaseAVFrame(&frame);
    return pcm_data_size;
}