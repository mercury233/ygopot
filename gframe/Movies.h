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
	SDL_mutex       *pictq_mutex;     //�������
	SDL_cond        *pictq_cond;      //��������
	SDL_Thread      *read_tid;        //���Ǵ��ļ���ȡ���ݷ�����е��߳�
	SDL_Thread      *video_tid;       //������Ƶ��ȡ��������߳�
	SDL_Thread      *refresh_tid;     //����ˢ���߳�
	double          audio_clock;      //��Ƶ��ʱ��� 
	double          video_clock;      //��Ƶ��ʱ���
	double          seek_time;        //Ҫ�ƶ���ʱ�䣨�룩
	int             seek_req;         //seek�ı�־ �Ƿ���Ҫseek
	int             seek_flags;       //seek�ķ�ʽ AVSEEK_FLAG_FRAME��
	int64_t         seek_pos;         //seek�����ʱ��
} StreamState;


class CMovies
{
public:
	CMovies();           // ��׼���캯��
	virtual ~CMovies();

	//�����������ĳ�ʼ�� �򿪱����� SDL��ʼ����,����Ĭ�ϴ���  ֻ����һ��
	int InitProgram(); 

	//�����˳����������رտ����ٴ��ڵ�  //ֻ����һ��
	int UinitProgram();

	//��������Ҫ�ı��������
	int InitVariable();

	//��������Ҫ�ı���������
	int UinitVariable();

	//���ļ����ҵ������
	int Open_codec_context(int * stream_idx, AVFormatContext * fmt_ctx, enum AVMediaType type); 

	//���� ������Ƶ�ص���������Ƶ�߳�
	int stream_component_open(StreamState *is, unsigned  stream_index);

	//�ر���Ƶ �ر���Ƶ�豸
	int stream_component_close(StreamState *is, unsigned  stream_index); 

	//ʼ������
	void packet_queue_init(PacketQueue *q) ;   

	//��packet������У�����ֻ�����ĺ�ˢ��packet�жϣ�
	int packet_queue_put(PacketQueue *q, AVPacket *pkt);   

	//�Ӷ�������ȡ������
	int packet_queue_get(PacketQueue *q, AVPacket *pkt, int queue_type);

	//��ֹ������������
	void packet_queue_abort(PacketQueue *q);             

	//������������к���
	int packet_queue_put_private(PacketQueue *q, AVPacket *pkt);    

	//ˢ��packet�ĳ�ʼ��
	void packet_queue_start(PacketQueue *q); 

	//���ٶ��� 
	void packet_queue_destroy(PacketQueue *q);     

	//��ն���
	void packet_queue_flush(PacketQueue *q); 

	//ˢ�²�������Ƶ��
	void video_refresh(void *opaque);  

	//������Ƶ����
	int audio_decode_frame(StreamState *is, uint8_t *audio_buf, int buf_size,double *pts_ptr);  

	//��������������ӵ�VideoPicture�ṹ����
	int queue_picture(StreamState *is, AVFrame *pFrame, double pts); 

	//��ʾ��Ƶ
	void video_display(StreamState *is);  

	//�������SDL_Overlay yuv����
	void alloc_picture(void *userdata) ;

	//�����򿪴������ô�С
	int video_open(StreamState *is);  

	static int read_thread( LPVOID lpParam );                                     //���ļ���ȡ����Ƶ ����ˢ���߳�
	static int refresh_thread(LPVOID lpParam);                                    //ˢ���߳�
	static void audio_callback(void *userdata, unsigned char *stream, int len) ;  //��Ƶ�ص�����
	static int video_thread(void *arg);                                           //��Ƶ�����߳̽����������ݴ���VideoPicture�ṹ��

	//��ʼ���ź�ֹͣ����
	void OnStartMovies(char* Path), OnStopMovies();


	//�Զ������
	StreamState  * m_streamstate;            //����Ƶȫ�ֽṹ��
	int m_fs_screen_width;                   //ȫ���Ŀ�
	int m_fs_screen_height;                  //ȫ���ĸ�
	AVPacket m_flush_pkt;                    //ˢ��packet
	char*   m_sourceFile;                    //Ҫ�򿪵��ļ�·��
	double m_file_duration;                  //�ļ���ʱ��
	AVCodecContext * m_video_dec_ctx ;       //��Ƶ����context
	AVCodecContext * m_audio_dec_ctx ;       //��Ƶ����context
	AVCodec * m_pvideo_codec;                //��Ƶ������
	AVCodec * m_paudio_codec;                //��Ƶ������
	SDL_Surface * m_screen;                  //sdl ��Ļ��ʾ�ṹ��
	int m_is_full_screen;                    //�Ƿ�ȫ������
	int m_screen_width ;                     //�ı��С��Ŀ�͸�
	int m_screen_height;                     //�ı��С��Ŀ�͸�
	int m_Isstop;                            //�Ƿ�ֹͣ 0��ʾ���� 1��ʾֹͣ
	int m_pause_play;                        //0��ʾ��ͣ 1��ʾ����
	int m_slider_pos ;                       //���Ź�����Ҫ����λ��
	int m_stream_type;                       //�ļ����� 1������Ƶ��������2��ֻ������Ƶ 3��ֻ������Ƶ
	//video pram
	int m_video_stream_idx;                  //��Ƶ���ļ��е������
	double m_dbFrameRate;                    //��Ƶ֡��
	double m_video_duration;                 //��Ƶ��֡����ʱ�� �루s��
	int m_dwWidth;                           //��
	int m_dwHeight;                          //��
	AVCodecID m_video_codecID;               //��Ƶ��������
	AVPixelFormat m_video_pixelfromat;       //��Ƶyuv����
	char m_spspps[1000];                      //spspps_buf
	int m_spspps_size;                       //spspps_size
	//audio pram
	int m_audio_stream_idx;                  //��Ƶ���ļ��е������
	double m_audio_duration;                 //��Ƶ��֡����ʱ�� �루s��
	int m_dwChannelCount;                    //����
	int m_dwBitsPerSample;                   //����
	int m_dwFrequency;                       //������
	AVCodecID m_audio_codecID;               //��Ƶ��������
	int m_audio_frame_size;                  //һ֡���ݴ�С
	int m_IsEnd_audio;                       //��Ƶ�Ƿ񲥷���� 0 δ������� 1�������
	int m_Volume_pos;                        //�������Ʊ��� 0-128
	struct {
		int left, right, top, bottom;
	} m_Picture_rect;                    //��ʾͼ��������С
};

extern HWND hWnd;