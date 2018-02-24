package com.qian.ffmpegtest;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Created by Administrator on 2018/2/24/024.
 */

public class VideoView extends SurfaceView{
    public VideoView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init();
    }


    public VideoView(Context context, AttributeSet attrs) {
        super(context, attrs,0);

    }


    public VideoView(Context context) {
        super(context,null);

    }


    //初始化进行输出设置格式
    public void init(){
        SurfaceHolder holder = getHolder();
        holder.setFormat(PixelFormat.RGB_888);
    }
}
