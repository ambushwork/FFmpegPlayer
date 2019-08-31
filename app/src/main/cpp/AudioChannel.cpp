//
// Created by LUYI on 2019/8/11.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int id,AVCodecContext* codecContext) : BaseChannel(id, codecContext) {}

AudioChannel::~AudioChannel() {

}

void AudioChannel::start() {
}

void AudioChannel::stop() {

}
