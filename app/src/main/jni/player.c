#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <libavcodec/avcodec.h>
#include <android/log.h>
#include <libswscale/swscale.h>
#include<unistd.h>
#include <libavformat/avformat.h>
#include "libyuv.h"
#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"Sheng",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"Sheng",FORMAT,##__VA_ARGS__);

JNIEXPORT void JNICALL
Java_com_qian_ffmpegtest_FFmpegPlayer_playMyMedia(JNIEnv *env, jobject jobj, jstring input,
                                                  jobject surface) {
    char * input_char = (*env)->GetStringUTFChars(env,input,NULL);




    //头文件libavformat/avformat.h
    //注册所有组件
    /**
     * 初始化libavformat和注册所有的 muxers, demuxers 和协议，如果你不想使用次函数，
     * 则可以手动选择你想要的支持格式
     * 详情你可以选择下面的函数查询
     * @see av_register_input_format()
     * @see av_register_output_format()
     *
     * muxer是合并将视频文件、音频文件和字幕文件合并为某一个视频格式。如，可将a.avi, a.mp3, a.srt用muxer合并为mkv格式的视频文件。
     * demuxer是拆分这些文件的。
     */
    av_register_all();

    // 封装格式上下文结构体，也是统领全局的结构体，保存了视频文件封装格式相关信息。
    AVFormatContext * pFormatCtx = avformat_alloc_context();

    /**
     * 打开输入流并且读取头信息。但解码器没有打开
     * 这个输入流必须使用avformat_close_input()关闭
     * @param ps（第一个参数的形参名称） 指向 你由你提供AVFormatContext（AVFormatContext是由avformat_alloc_context函数分配的）。
     * 有可能ps指向空，在这种情况下，AVFormatContext由此函数分配并写入ps。
     * 注意： 你提供的AVFormatContext在函数执行失败的时候将会被释放
     * @param url 你要打开视频文件路径.
     * @param fmt  如果不为空，那么这个参数将强制作为输入格式，否则自动检索
     * @param options 一个关于AVFormatContext and demuxer-private 选项的字典.
     * 返回时，此参数将被销毁，并替换为包含未找到的选项的dict。有可能是空的
     *
     * @return 返回0表示成功, 一个负数常量AVERROR是失败的.
     *
     * @note 如果你想自定义IO,你需要预分配格式内容并且设置pd属性
     */
    if(avformat_open_input(&pFormatCtx,input_char,NULL,NULL)!=0){
        LOGE("NDK>>>%s","avformat_open_input打开失败");
        return;
    }

    //上面打开输入流后会将视频封装格式信息写入AVFormatContext中那么我们下一步就可以得到一些展示信息
    /**
     *
     * 读取媒体文件中的数据包以获取流信息，这个对于对于文件格式没有头信息的很有帮助，比如说mpeg
     * 这个函数还可以计算在MPEG-2重复帧模式的真实帧速率。
     * 逻辑文件位置不会被这个函数改变
     * 检索过的数据包或许会缓存以供后续处理
     * @param ic  第一个参数 封装格式上下文
     * @param options
     *              如果不为空， 一个长度为 ic.nb_streams （封装格式所有流，字幕 视频 音频等） 的字典。
     *              字典中第i个成员  包含一个对应ic第i个流中对的编码器。
     *              在返回时，每个字典将会填充没有找到的选项
     * @return 如果返回>=0 代表成功, AVERROR_xxx 表示失败
     *
     * @note 这个函数 不保证能打开所有编码器，所以返回一个非空的选项是一个完全正常的行为
     *
     *
     * @todo
     *  下个版本目标无视即可
     * Let the user decide somehow what information is needed so that
     *       we do not waste time getting stuff the user does not need.
     */
    if( avformat_find_stream_info(pFormatCtx,NULL)<0){
        LOGE("NDK>>>%s","avformat_find_stream_info失败");
        return ;
    }
    LOGE("NDK>>>%s","成功");
    //  //输出视频信息
    //  LOGI("视频的文件格式：%s",pFormatCtx->iformat->name);
    //  LOGI("视频时长：%d", (pFormatCtx->duration)/1000000);

    //获取视频流的索引位置
    //遍历所有类型的流（音频流、视频流、字幕流），找到视频流
    int v_stream_idx = -1;
    int i = 0;
    //遍历封装格式中所有流
    for (; i < pFormatCtx->nb_streams; ++i) {

        //获取视频流pFormatCtx->streams[i]
        //pFormatCtx->streams[i]->codec获取编码器
        //codec_type获取编码器类型
        //当前流等于视频 记录下标
        if (pFormatCtx->streams[i]->codec->codec_type ==AVMEDIA_TYPE_VIDEO) {
            v_stream_idx = i;
            break;
        }
    }
    if (v_stream_idx==-1) {
        LOGE("没有找视频流")
    }else{
        LOGE("找到视频流")
    }

    //编码器上下文结构体，保存了视频（音频）编解码相关信息
    //得到视频流编码器
    AVCodecContext *pCodecCtx = pFormatCtx->streams[v_stream_idx]->codec;

    //   每种视频（音频）编解码器(例如H.264解码器)对应一个该结构体。
    AVCodec *pCodec =avcodec_find_decoder(pCodecCtx->codec_id);

    //（迅雷看看，找不到解码器，临时下载一个解码器）
    if (pCodec == NULL)
    {
        LOGE("%s","找不到解码器\n");
        return;
    }else{
        LOGE("%s","找到解码器\n");
    }


    //打开解码器
    /**
     * 初始化 指定AVCodecContext去使用 给定的AVCodec
     * 在使用之前函数必须使用avcodec_alloc_context3（）分配上下文。
     *
     * 以下函数 avcodec_find_decoder_by_name(), avcodec_find_encoder_by_name(),
     * avcodec_find_decoder() and avcodec_find_encoder() 提供了一个简便的得到一个解码器的方法
     *
     * @warning 这个函数线程不是安全的
     *
     * @note 在使用解码程序之前，始终调用此函数 (如 @ref avcodec_decode_video2()).
     * 下面是示例代码
     * @code
     * avcodec_register_all();
     * av_dict_set(&opts, "b", "2.5M", 0);
     * codec = avcodec_find_decoder(AV_CODEC_ID_H264);
     * if (!codec)
     *     exit(1);
     *
     * context = avcodec_alloc_context3(codec);
     *
     * if (avcodec_open2(context, codec, opts) < 0)
     *     exit(1);
     * @endcode
     *
     *
     * @param avctx 要初始化的编码器
     * @param codec 用这个codec去打开给定的上下文编码器.如果 codec 不为空 那么必须
     * 事先用avcodec_alloc_context3和avcodec_get_context_defaults3传递给这个context，那么这个codec
     * 要么为NULL要么就是上面调用函数所使用的codec
     *
     * @param
     *
     * 选项填充AVCodecContext和编解码器私有选项的字典。返回时，此对象将填充未找到的选项。
     *
     * @return 返回0表示成功， 负数失败
     * @see avcodec_alloc_context3(), avcodec_find_decoder(), avcodec_find_encoder(),
     *      av_dict_set(), av_opt_find().
     */
    if(avcodec_open2(pCodecCtx,pCodec,NULL)==0){
        LOGE("%s","打开编码器成功\n");
    }else{
        LOGE("%s","打开编码器失败\n");
        return;
    }
    //输出视频信息
    LOGE("视频的文件格式：%s",pFormatCtx->iformat->name);
    //得到视频播放时长
    if(pFormatCtx->duration != AV_NOPTS_VALUE){
        int hours, mins, secs, us;
        int64_t duration = pFormatCtx->duration + 5000;
        secs = duration / AV_TIME_BASE;
        us = duration % AV_TIME_BASE;
        mins = secs / 60;
        secs %= 60;
        hours = mins/ 60;
        mins %= 60;
        LOGE("%02d:%02d:%02d.%02d\n", hours, mins, secs, (100 * us) / AV_TIME_BASE);

    }
    LOGE("视频的宽高：%d,%d",pCodecCtx->width,pCodecCtx->height);
    LOGE("解码器的名称：%s",pCodec->name);

    //
    //  //存储一帧压缩编码数据。
    AVPacket *packet =av_malloc(sizeof(AVPacket));
    //
    //  //输出转码文件地址
    //  FILE *fp_yuv = fopen(output_cstr,"wb+");
    //
    //  //AVFrame用于存储解码后的像素数据(YUV)
    //  //内存分配
    AVFrame *pFrame = av_frame_alloc();
    //
    //  //YUV420转码用
    //  AVFrame *pFrameYUV = av_frame_alloc();

    //  //avpicture_get_size()函数介绍:
    //  //
    //  /**
    //   * 如果给定存储图片的格式,那么计算给定的宽高所占用的大小
    //   *
    //   * @param pix_fmt   图片像素格式
    //   * @param width     图片宽
    //   * @param height     图片高
    //   * @return 返回计算的图片缓存大小或者错误情况下的负数错误代码
    //   *
    //   *
    //   * 这里计算缓存区的大小,但是没有分配,这里是用来后面转码使用
    //   */
    //  uint8_t *out_buffer = av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P,pCodecCtx->width,pCodecCtx->height));
    //
    //  //初始化缓冲区
    //  /**
    //   * 基于指定的图片参数和提供的图片缓存区去设置图片字段
    //   *
    //   * 使用ptr所指向的图片数据缓存  填充图片属性
    //   *
    //   * 如果 ptr是空,这个函数仅填充图片行大小(linesize)的数组并且返回图片缓存请求的大小
    //   *
    //   * 要分配图片缓存并且再一次填充图片数据请使用 avpicture_alloc().
    //   * @param picture       要填充的图片
    //   * @param ptr           存储图片的数据的缓存区, or NULL
    //   * @param pix_fmt       图片像素格式
    //   * @param width         图片宽
    //   * @param height        图片高
    //   * @return 返回请求的字节大小,在错误的情况下返回负数
    //   *
    //   * @see av_image_fill_arrays()
    //   */
    //  avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
    //  //用于转码（缩放）的参数，转之前的宽高，转之后的宽高，格式等
    //  /**
    //   *分配和返回 SwsContext. You need it to perform
    //   * scaling/conversion operations using sws_scale().
    //   *
    //   * @param srcW 原始图宽
    //   * @param srcH 原始图高
    //   * @param srcFormat 原始图格式
    //   * @param dstW 目标图宽
    //   * @param dstH 不解释
    //   * @param dstFormat 不解释
    //   * @param flags 指定一个标志用于重新调整算法和选项
    //   *  具体参考:http://blog.csdn.net/leixiaohua1020/article/details/12029505
    //   * @return 一个指定分配内容的指针, 错误情况返回空
    //   * @note this function is to be removed after a saner alternative is
    //   *       written
    //   */
    //      struct SwsContext *sws_ctx =sws_getContext(pCodecCtx->width, pCodecCtx->height,pCodecCtx->pix_fmt,
    //              pCodecCtx->width, pCodecCtx->height,AV_PIX_FMT_YUV420P,
    //              SWS_BICUBIC, NULL, NULL, NULL);
    //
    //
    //  //标志位
    int got_picture, ret;
    //
    //  //返回和java surface关联的ANativeWindow通过本地本地方法交互
    ANativeWindow * nativeWindow =ANativeWindow_fromSurface(env,surface);
    ////    //缓存
    ANativeWindow_Buffer outBuffer;
    ////    //设置缓存的几何信息
    AVFrame *rgb_frame = av_frame_alloc();

    uint8_t *out_buffer = av_malloc(avpicture_get_size(AV_PIX_FMT_RGBA,pCodecCtx->width,pCodecCtx->height));
    //
    //
    //读取每一帧
    /**
     *返回下一帧的流
     * 此函数返回存储在文件中的内容，并且不会验证解码器有什么有效帧。
     * 函数将存储在文件中的帧进行分割 并且返回给每一个调用者。
     *
     * 函数不会删除在有效帧之间的无效数据 以便在可能解码过程中提供解码器最大的信息帮助
     * 如果 pkt->buf 是空的,那么这个对应数据包是有效的直到下一次调用av_read_frame()
     * 或者直到使用avformat_close_input().否则包无期限有效
     * 在这两种情况下 这个数据包当你不在需要的时候,你必须使用使用av_free_packet释放它
     * 对于视屏,数据包刚好只包含一帧.对于音频,如果它每一帧是一个已知固定大小的,那么他包含整数帧(如. PCM or ADPCM data)
     * 如果音频帧具有可变大小(如. MPEG audio),那么他只包含一帧
     * pkt->pts, pkt->dts and pkt->duration 始终在AVStream.time_base 单位设置正确的数值
     *(如果这个格式无法提供.那么进行猜测)
     * 如果视频格式有B帧那么pkt->pts可以是 AV_NOPTS_VALUE.如果你没有解压他的有效部分那么最好依靠pkt->dts
     *
     * @return 0表示成功, < 0 错误或者文结束
     */

    while(av_read_frame(pFormatCtx,packet)>=0){

        //一个包里有很多种类型如音频视频等 所以判断 这个包对应的流的在封装格式的下表
        //如果这个包是视频频包那么得到压缩的视频包
        if (packet->stream_index==v_stream_idx) {
            LOGE("测试");
            /**
             * 解码视频帧 从avpkt->data读取数据并且解码avpkt->size的大小后转化为图片.
             * 一些解码器可以支持在一个ACpacket中存在多帧的情况,像这样的解码器将只解码第一帧
             *
             * @warning  输入缓存区必须 实际读取的字节流小于 FF_INPUT_BUFFER_PADDING_SIZE,
             * 一些优化过的比特流 读取32位或者64字节 的时候可以一次性读取完
             *
             * @warning 在缓冲器的buf结尾设置0以确保被破坏的MPEG流不会发生超线程
             *
             * @note 有 CODEC_CAP_DELAY 才能设置一个在输入和输出之间的延迟,这些需要使用avpkt->data=NULL,
             *  在结束返回剩余帧数的时候avpkt->size=0
             *
             * @note  这个AVCodecContext 在数据包传入解码器之前必须调用avcodec_open2
             *
             *
             * @param avctx 解码器上下文
             *
             * @param[out] 解码的视频帧图片将会被存储在AVFrame.
             *                 使用av_frame_alloc 得到一个AVFrame,
             *                 编码器将会分配 使用  AVCodecContext.get_buffer2() 回调
             *                 的实际图片的内存.
             *                 当AVCodecContext.refcounted_frames 设置为1,这帧(frame)是引用计数,并且返回
             *                 的引用计数是属于调用者的.
             *             frame在长实际不使用的时候调用者必须调用av_frame_unref()就行释放
             *             如果av_frame_is_writable()返回1那么调用者可以安全的写入到这个frame中。
             *                 当AVCodecContext.refcounted_frames设置为0，返回的引用属于解码器，
             *                只有下次使用这个函数或者关闭或者刷新这个编码器之前有效。调用者不会写入它
             *
             *@param[in,out] got_picture_ptr 如果为0表示不能解压, 否者它不是0.
             *
             * @param[in] avpkt 这个输入的avpkt包含输入缓存区
             *              你能使用av_init_packet（）创建像这样的packet然后设置数据和大小，
             *              一些解码器还可以添加一些其他字段 比如  flags&AV_PKT_FLAG_KEY  flags&AV_PKT_FLAG_KEY
             *          所有解码器都设计为尽可能少地使用
             *
             * @return 再错误时返回一个负数 , 否则返回使用字节数或者或者0(没有帧被解压返回0)otherwise the number of bytes
             *
             */
            ret=avcodec_decode_video2(pCodecCtx,pFrame,&got_picture,packet);
            if(ret>=0){
                LOGE("解压成功");
                //AVFrame转为像素格式YUV420，宽高
                //2 6输入、输出数据
                //3 7输入、输出画面一行的数据的大小 AVFrame 转换是一行一行转换的
                //4 输入数据第一列要转码的位置 从0开始
                //5 输入画面的高度
                //              sws_scale(sws_ctx,pFrame->data,pFrame->linesize,0,pCodecCtx->height,pFrameYUV->data,pFrameYUV->linesize);

                //输出到YUV文件
                //AVFrame像素帧写入文件
                //data解码后的图像像素数据（音频采样数据）
                //Y 亮度 UV 色度（压缩了） 人对亮度更加敏感
                //U V 个数是Y的1/4
                //              int y_size = pCodecCtx->width * pCodecCtx->height;
                ANativeWindow_lock(nativeWindow,&outBuffer,NULL);
//              ////
                ANativeWindow_setBuffersGeometry(nativeWindow,pCodecCtx->width,pCodecCtx->height,WINDOW_FORMAT_RGBA_8888);
                avpicture_fill((AVPicture *)rgb_frame,out_buffer, AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height);
//
//
                I420ToARGB(pFrame->data[0],pFrame->linesize[0],
                           pFrame->data[2],pFrame->linesize[2],
                           pFrame->data[1],pFrame->linesize[1],
                           rgb_frame->data[0], rgb_frame->linesize[0],
                           pCodecCtx->width,pCodecCtx->height);
                int h = 0;
                for (h = 0; h < pCodecCtx->height; h++) {
                    memcpy(outBuffer.bits + h * outBuffer.stride*4, out_buffer + h * rgb_frame->linesize[0], rgb_frame->linesize[0]);
//              memcpy(outBuffer.bits,out_buffer,pCodecCtx->width*pCodecCtx->height*4);
                }
                LOGE("锁定成功");

                ANativeWindow_unlockAndPost(nativeWindow);
//               //获取帧率tbr fbs
                //float fram_rate =pFormatCtx->streams[v_stream_idx]->avg_frame_rate.num/pFormatCtx->streams[v_stream_idx]->avg_frame_rate.den;
//              如果奔溃请开启下面的线程休眠
//              usleep(1000);


            }
        }
        av_free_packet(packet);
    }


    (*env)->ReleaseStringUTFChars(env,input,input_char);


    //关闭文件
    //  fclose(fp_yuv);
    ANativeWindow_release(nativeWindow);
//  //关闭资源
    av_frame_free(&pFrame);
//  //  av_frame_free(&pFrameYUV);
//  av_frame_free(&rgb_frame);
//  //关闭编码器上下文
    avcodec_close(pCodecCtx);
//  //关闭输入流
    avformat_close_input(&pFormatCtx);
//  //关闭封装格式
    avformat_free_context(pFormatCtx);
}