//
// Created by LUYI on 2019/10/27.
//

#include <cstring>
#include "AudioPushChannel.h"
#include "libfaac/include/faac.h"
#include "librtmp/rtmp.h"
#include "macro.h"


RTMPPacket *AudioPushChannel::getAudioTag() {
    u_char *buf;
    u_long len;
    faacEncGetDecoderSpecificInfo(audioCodec, &buf, &len);
    int bodySizw = 2 + len;
    int bodySize = 2 + len;
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodySize);
    packet->m_body[0] = 0xAF;
    if (mChannels == 1) {
        packet->m_body[0] = 0xAE;
    }
    packet->m_body[1] = 0x00;
    memcpy(&packet->m_body[2], buf, len);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x11;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    return packet;


}
//不断调用
void AudioPushChannel::encodeData(int8_t *data) {
    int bytelen= faacEncEncode(audioCodec, reinterpret_cast<int32_t *>(data), inputSamples, buffer, maxOutputBytes);
    if (bytelen > 0) {
        RTMPPacket *packet = new RTMPPacket;
        int bodySize = 2 + bytelen;
        RTMPPacket_Alloc(packet, bodySize);
        packet->m_body[0] = 0xAF;
        if (mChannels == 1) {
            packet->m_body[0] = 0xAE;
        }
        packet->m_body[1] = 0x01;

        memcpy(&packet->m_body[2], buffer, bytelen);


//        aac
        packet->m_hasAbsTimestamp = 0;
        packet->m_nBodySize = bodySize;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 0x11;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

        LOGE("Push Audio packet!")
        basePushChannel->pushPacket(packet);
    }

}
void AudioPushChannel::initAudioEncoder(int samplesInHZ, int channels){
    audioCodec=faacEncOpen(samplesInHZ, channels, &inputSamples, &maxOutputBytes);
    faacEncConfigurationPtr config=faacEncGetCurrentConfiguration(audioCodec);
    config->mpegVersion = MPEG4;
    config->aacObjectType = LOW;
    config->inputFormat = FAAC_INPUT_16BIT;
    config->outputFormat = 0;
    faacEncSetConfiguration(audioCodec, config);
    buffer = new u_char[maxOutputBytes];
}


int AudioPushChannel::getInputSamples() {
    return inputSamples;
}
AudioPushChannel::~AudioPushChannel() {
    DELETE(buffer);
    if (audioCodec) {
        faacEncClose(audioCodec);
        audioCodec = 0;
    }
}

