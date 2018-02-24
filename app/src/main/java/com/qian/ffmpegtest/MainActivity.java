package com.qian.ffmpegtest;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Toast;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    FFmpegPlayer fFmpegPlayer;
    VideoView videoView ;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        videoView = findViewById(R.id.videoView);
    }

    public void mplay(View btn){
        Toast.makeText(MainActivity.this,"点击了",Toast.LENGTH_LONG).show();
        fFmpegPlayer = new FFmpegPlayer();
        String input = Environment.getExternalStorageDirectory()
                .getAbsolutePath()+ File.separatorChar+"今日介绍.flv";
        /*String output = Environment.getExternalStorageDirectory()
                .getAbsolutePath()+ File.separatorChar+"今日介绍.yuv";*/
        fFmpegPlayer.playMyMedia(input,videoView.getHolder().getSurface());
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */

}
