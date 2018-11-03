#pragma once
#include "windows.h"
#include "windef.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/mathematics.h"
#include "inttypes.h"
#include "SDL.h"
#include "SDL_thread.h"
};

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"postproc.lib")
#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")

#pragma comment(lib,"SDL.lib")


#define VIDEO_PICTURE_QUEUE_SIZE 1
#define SDL_AUDIO_BUFFER_SIZE 1024
#define AV_NOSYNC_THRESHOLD 10.0
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES      5

#define VIDEO_ID 100
#define AUDIO_ID 101

typedef struct PacketQueue 
{
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

typedef struct VideoPicture
{
	SDL_Overlay *bmp;
	int width, height; /* source height & width */
	int allocated;
	double pts;
} VideoPicture;

typedef struct StreamState_t 
{
	AVFormatContext *pFormatCtx;
	AVStream        *audio_st;
	AVStream        *video_st;
	PacketQueue     audioq;
	PacketQueue     videoq;
	uint8_t         audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	unsigned int    audio_buf_size;
	unsigned int    audio_buf_index;
	AVPacket        audio_pkt;
	uint8_t         *audio_pkt_data;
	int             audio_pkt_size;
	VideoPicture    pictq[VIDEO_PICTURE_QUEUE_SIZE];
	int             pictq_size, pictq_rindex, pictq_windex;
	SDL_mutex       *pictq_mutex;     //互斥变量
	SDL_cond        *pictq_cond;      //条件变量
	SDL_Thread      *read_tid;        //这是从文件读取数据放入队列的线程
	SDL_Thread      *video_tid;       //这是视频读取放入队列线程
	SDL_Thread      *refresh_tid;     //这是刷新线程
	double          audio_clock;      //音频的时间戳 
	double          video_clock;      //视频的时间戳
	double          seek_time;        //要移动的时间（秒）
	int             seek_req;         //seek的标志 是否需要seek
	int             seek_flags;       //seek的方式 AVSEEK_FLAG_FRAME等
	int64_t         seek_pos;         //seek过后的时间
} StreamState;


class CMovies
{
public:
	CMovies();           // 标准构造函数
	virtual ~CMovies();

	//程序启动做的初始化 打开编解码库 SDL初始化等,创建默认窗口  只调用一次
	int InitProgram(); 

	//程序退出做的析构关闭库销毁窗口等  //只调用一次
	int UinitProgram();

	//程序所需要的变量分配等
	int InitVariable();

	//程序所需要的变量析构等
	int UinitVariable();

	//在文件中找到流标号
	int Open_codec_context(int * stream_idx, AVFormatContext * fmt_ctx, enum AVMediaType type); 

	//创建 创建音频回调函数，视频线程
	int stream_component_open(StreamState *is, unsigned  stream_index);

	//关闭视频 关闭音频设备
	int stream_component_close(StreamState *is, unsigned  stream_index); 

	//始化队列
	void packet_queue_init(PacketQueue *q) ;   

	//将packet填入队列（这里只是做的和刷新packet判断）
	int packet_queue_put(PacketQueue *q, AVPacket *pkt);   

	//从队列里面取出数据
	int packet_queue_get(PacketQueue *q, AVPacket *pkt, int queue_type);

	//终止向队列输入输出
	void packet_queue_abort(PacketQueue *q);             

	//真正的填入队列函数
	int packet_queue_put_private(PacketQueue *q, AVPacket *pkt);    

	//刷新packet的初始化
	void packet_queue_start(PacketQueue *q); 

	//销毁队列 
	void packet_queue_destroy(PacketQueue *q);     

	//清空队列
	void packet_queue_flush(PacketQueue *q); 

	//刷新操作（视频）
	void video_refresh(void *opaque);  

	//解码音频数据
	int audio_decode_frame(StreamState *is, uint8_t *audio_buf, int buf_size,double *pts_ptr);  

	//将解码后的数据添加到VideoPicture结构体中
	int queue_picture(StreamState *is, AVFrame *pFrame, double pts); 

	//显示视频
	void video_display(StreamState *is);  

	//创建输出SDL_Overlay yuv容器
	void alloc_picture(void *userdata) ;

	//创建打开窗口设置大小
	int video_open(StreamState *is);  

	static int read_thread( LPVOID lpParam );                                     //从文件读取音视频 创建刷新线程
	static int refresh_thread(LPVOID lpParam);                                    //刷新线程
	static void audio_callback(void *userdata, unsigned char *stream, int len) ;  //音频回调函数
	static int video_thread(void *arg);                                           //视频解码线程将解码后的数据传入VideoPicture结构体

	//开始播放和停止播放
	void OnStartMovies(char* Path), OnStopMovies();


	//自定义变量
	StreamState  * m_streamstate;            //音视频全局结构体
	int m_fs_screen_width;                   //全屏的宽
	int m_fs_screen_height;                  //全屏的高
	AVPacket m_flush_pkt;                    //刷新packet
	char*   m_sourceFile;                    //要打开的文件路径
	double m_file_duration;                  //文件总时常
	AVCodecContext * m_video_dec_ctx ;       //视频解码context
	AVCodecContext * m_audio_dec_ctx ;       //音频解码context
	AVCodec * m_pvideo_codec;                //视频解码器
	AVCodec * m_paudio_codec;                //音频解码器
	SDL_Surface * m_screen;                  //sdl 屏幕显示结构体
	int m_is_full_screen;                    //是否全屏播放
	int m_screen_width ;                     //改变大小后的宽和高
	int m_screen_height;                     //改变大小后的宽和高
	int m_Isstop;                            //是否停止 0表示播放 1表示停止
	int m_pause_play;                        //0表示暂停 1表示播放
	int m_slider_pos ;                       //播放滚动条要到的位置
	int m_stream_type;                       //文件类型 1：音视频都包含，2：只包含音频 3：只包含视频
	//video pram
	int m_video_stream_idx;                  //视频在文件中的流标号
	double m_dbFrameRate;                    //视频帧率
	double m_video_duration;                 //视频单帧持续时间 秒（s）
	int m_dwWidth;                           //宽
	int m_dwHeight;                          //高
	AVCodecID m_video_codecID;               //视频编码类型
	AVPixelFormat m_video_pixelfromat;       //视频yuv类型
	char m_spspps[1000];                      //spspps_buf
	int m_spspps_size;                       //spspps_size
	//audio pram
	int m_audio_stream_idx;                  //音频在文件中的流标号
	double m_audio_duration;                 //音频单帧持续时间 秒（s）
	int m_dwChannelCount;                    //声道
	int m_dwBitsPerSample;                   //样本
	int m_dwFrequency;                       //采样率
	AVCodecID m_audio_codecID;               //视频编码类型
	int m_audio_frame_size;                  //一帧数据大小
	int m_IsEnd_audio;                       //音频是否播放完毕 0 未播放完毕 1播放完毕
	int m_Volume_pos;                        //音量控制变量 0-128
	struct {
		int left, right, top, bottom;
	} m_Picture_rect;                    //显示图像的区域大小
};

extern HWND hWnd;