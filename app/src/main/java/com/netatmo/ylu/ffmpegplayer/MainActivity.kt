package com.netatmo.ylu.ffmpegplayer

import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.SurfaceView
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File


class MainActivity : AppCompatActivity() {

    private lateinit var player: FFmpegPlayer

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        player = FFmpegPlayer()
        player.setSurfaceView(surfaceView)

        button.setOnClickListener {
            val file = Environment.getExternalStorageDirectory()
            val path = file.absolutePath + "/../../sdcard0/input.mp4"
            val video = File(path)
            if(video.exists()){
                player.start(video.absolutePath)
            } else {
                Log.e("Main", "File doesn't exist!")
            }

        }
    }



}
