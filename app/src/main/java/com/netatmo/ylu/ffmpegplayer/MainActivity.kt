package com.netatmo.ylu.ffmpegplayer

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.SurfaceView
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File
import android.support.v4.content.FileProvider




class MainActivity : AppCompatActivity() {

    private lateinit var player: FFmpegPlayer
    private var path : String? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        player = FFmpegPlayer()
        player.setSurfaceView(surfaceView)

        button_select.setOnClickListener {
            chooseFile()
        }
        button_play.setOnClickListener {
            path?.let {
                playFile(it)
            }

        }
        button_transform.setOnClickListener {
            path?.let{
                transform(it)
            }
        }
    }

    private fun chooseFile() {
        val intent = Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*").addCategory(Intent.CATEGORY_OPENABLE)
        startActivityForResult(Intent.createChooser(intent, "Choose File"), 1)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        var uri : Uri? = null
        if (resultCode == Activity.RESULT_OK) {
            if (requestCode == 1) {
                uri = data?.data
            }
            path = FileSearch.getPath(this, uri)
            Log.d("Uri",path)
        } else {
            Log.e("Error", "onActivityResult() error, resultCode: $resultCode")
        }
        super.onActivityResult(requestCode, resultCode, data)
    }

    private fun  playFile(path: String){
        val video = File(path)
        if(video.exists()){
            player.start(video.absolutePath)
        } else {
            Log.e("Main", "File doesn't exist!")
        }
    }

    private fun transform(path: String){
        val audio = File(path)
        if(audio.exists()){
            val file = File("/storage/emulated/0/output.pcm")
            player.sound(path, file.absolutePath)
        }
    }
}
