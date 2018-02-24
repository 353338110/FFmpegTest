package com.qian.ffmpegtest;

import android.view.Surface;

/**
 * File: FFmpegPlayer.java
 * Created by master
 * Create: 2018/2/8 15:41
 */

public class FFmpegPlayer {

    static
    {
        /*System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("swscale-4");
        System.loadLibrary("postproc-54");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avdevice-57");*/
        System.loadLibrary("ffmpeg");
    }
    public native void playMyMedia(String input,Surface surface);
}
