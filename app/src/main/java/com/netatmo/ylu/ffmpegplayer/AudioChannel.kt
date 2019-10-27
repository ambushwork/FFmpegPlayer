package com.netatmo.ylu.ffmpegplayer

import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import java.util.concurrent.Executors

class AudioChannel(val pushManager: PushManager){

    private var isLive = false
    private val executor = Executors.newSingleThreadExecutor()
    private var inputSamples: Int = 0
    private val audioRecord :AudioRecord = AudioRecord(MediaRecorder.AudioSource.MIC,
        44100,
        AudioFormat.CHANNEL_IN_STEREO,
        AudioFormat.ENCODING_PCM_16BIT,
        AudioRecord.getMinBufferSize(44100,
            AudioFormat.CHANNEL_IN_STEREO,
            AudioFormat.ENCODING_PCM_16BIT ) * 2
        )

    fun startLive(){
        isLive = true
        pushManager.native_initAudioEncoder(44100, 2)
        inputSamples = pushManager.inputSamples *2
        executor.submit(AudioTask())

    }

    fun stopLive(){
        isLive = false
    }

    fun release(){
        audioRecord.release()

    }

    inner class AudioTask : Runnable {
        override fun run() {
            audioRecord.startRecording()
            val bytes = ByteArray(inputSamples)
            while (isLive) {
                val len = audioRecord.read(bytes, 0, bytes.size)
                pushManager.native_pushAudio(bytes)
            }
            audioRecord.stop()
        }

    }
}


