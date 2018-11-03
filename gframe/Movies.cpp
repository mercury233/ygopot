// Movies.cpp : ʵ���ļ�
//

#include "Movies.h"
#include "mwin.h"

CMovies::CMovies()
{
    //////////////////////////////////////////////////////////////////////////
	//��ʼ��ȫ�ֱ��� ������ SDL��ʼ����

	InitProgram();
	//////////////////////////////////////////////////////////////////////////

	InitVariable();
	//////////////////////////////////////////////////////////////////////////
}


CMovies::~CMovies()
{
	OnStopMovies();
	UinitProgram();
}


int CMovies::InitProgram()
{
	/* ע��������� */
	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	//�����Ŵ��ں�sdl��ʾ���ڹ��� 
	//HWND hWnd ��Ĳ��Ŵ��ھ�� ��Ҫʹ��SDL�Դ��Ĵ��ڣ�ʹ��:SDL_putenv("SDL_VIDEO_CENTERED=certer"); 
	
	//SDL_putenv("SDL_VIDEO_CENTERED=certer");
	
	if( hWnd !=NULL)
	{
		char sdl_var[64];    
		sprintf_s(sdl_var, "SDL_WINDOWID=%d", hWnd);  //����һ�������пո�SDL_WINDOWID=%d"
		SDL_putenv(sdl_var);   
		SDL_getenv("SDL_WINDOWID");                   //��SDLȡ�ô���ID  
	} else {
		SDL_putenv("SDL_VIDEO_CENTERED=certer");
	}
	

	//SDL��ʼ��
	int flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
	int  sdlinit = SDL_Init (flags);
	if (sdlinit)
	{
		char * sss = SDL_GetError();
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		fprintf(stderr, "(Did you set the DISPLAY variable?)\n");
		return -1;
	} 

	//����SDL�¼�״̬
	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

	//////////////////////////////////////////////////////////////////////////
	//����Ƶȫ�ֽṹ���ʼ�� �����Ѿ�����
	m_streamstate = NULL;
	m_streamstate = (StreamState *)calloc(1,sizeof(StreamState));
	if (!m_streamstate)
	{
		return -1;
	}

	m_streamstate->pFormatCtx = NULL;
	m_streamstate->audio_st = NULL;
	m_streamstate->video_st = NULL;
	m_streamstate->audioq.first_pkt = NULL;
	m_streamstate->audioq.last_pkt = NULL;
	m_streamstate->audioq.nb_packets = 0;
	m_streamstate->audioq.size = 0;
	m_streamstate->audioq.mutex = NULL;
	m_streamstate->audioq.cond = NULL;
	m_streamstate->videoq.first_pkt = NULL;
	m_streamstate->videoq.last_pkt = NULL;
	m_streamstate->videoq.nb_packets = 0;
	m_streamstate->videoq.size = 0;
	m_streamstate->videoq.mutex = NULL;
	m_streamstate->videoq.cond = NULL;
	memset(m_streamstate->audio_buf,0,(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2);
	m_streamstate->audio_buf_size = 0;
	m_streamstate->audio_buf_index = 0;

	//AVPacket       audio_pkt;
	m_streamstate->audio_pkt_data = NULL;
	m_streamstate->audio_pkt_size = 0;
	m_streamstate->pictq[0].bmp = NULL;
	m_streamstate->pictq[0].width = 0;
	m_streamstate->pictq[0].height = 0;
	m_streamstate->pictq[0].allocated = 0;
	m_streamstate->pictq[0].pts = 0.0;
	m_streamstate->pictq_size = 0;
	m_streamstate->pictq_rindex = 0;
	m_streamstate->pictq_windex = 0;
	m_streamstate->pictq_mutex = NULL;
	m_streamstate->pictq_cond = NULL;
	m_streamstate->read_tid = NULL; 
	m_streamstate->video_tid = NULL;  
	m_streamstate->refresh_tid = NULL;
	m_streamstate->audio_clock = 0.0;
	m_streamstate->video_clock = 0.0;
	m_streamstate->seek_time = 0.0;
	m_streamstate->seek_req = 0;
	m_streamstate->seek_flags = 0;
	m_streamstate->seek_pos = 0;

	//////////////////////////////////////////////////////////////////////////
	//��ȡȫ������Ļ���  ����ֻ�ܵ���һ�� �ڶ��ε���ʱ ��С���ı�
	const SDL_VideoInfo *vi = SDL_GetVideoInfo();
	m_fs_screen_width = vi->current_w;
	m_fs_screen_height = vi->current_h;

	//AVPacket m_flush_pkt;
	m_sourceFile = "";
	m_file_duration = 0.0;
	m_video_dec_ctx = NULL; 
	m_audio_dec_ctx = NULL ; 
	m_pvideo_codec = NULL; 
	m_paudio_codec = NULL;
	m_screen = NULL;
	m_is_full_screen = 0;
	m_screen_width  = 0;
	m_screen_height = 0;
	m_Isstop = 0;
	m_pause_play = 0;                          
	m_slider_pos = 0;    
	m_stream_type = 0;

	//////////////////////////////////////////////////////////////////////////
	//video pram
	m_video_stream_idx = 0;              
	m_dbFrameRate = 0.0;               
	m_video_duration = 0.0;              
	m_dwWidth = 0;                          
	m_dwHeight = 0;                     
	m_video_codecID = AV_CODEC_ID_NONE;
	m_video_pixelfromat = AV_PIX_FMT_NONE;  
	memset(m_spspps,0,100);
	m_spspps_size = 0;

	//////////////////////////////////////////////////////////////////////////
	//audio pram
	m_audio_stream_idx = 0; 
	m_audio_duration = 0.0;              
	m_dwChannelCount = 0;              
	m_dwBitsPerSample = 0;             
	m_dwFrequency = 0;                 
	m_audio_codecID = AV_CODEC_ID_NONE;               
	m_audio_frame_size = 0;                  
	m_IsEnd_audio = 0;                         
	m_Volume_pos = 0;     

	//////////////////////////////////////////////////////////////////////////
	m_Picture_rect.left = 0;
	m_Picture_rect.right = 0;
	m_Picture_rect.top = 0;
	m_Picture_rect.bottom = 0;
	//////////////////////////////////////////////////////////////////////////
	return 1;
}


int CMovies::UinitProgram()
{
	//////////////////////////////////////////////////////////////////////////
	//�ͷ�ȫ�ֽṹ��
	m_streamstate->pFormatCtx = NULL;
	m_streamstate->audio_st = NULL;
	m_streamstate->video_st = NULL;
	m_streamstate->audioq.first_pkt = NULL;
	m_streamstate->audioq.last_pkt = NULL;
	m_streamstate->audioq.nb_packets = 0;
	m_streamstate->audioq.size = 0;
	m_streamstate->audioq.mutex = NULL;
	m_streamstate->audioq.cond = NULL;
	m_streamstate->videoq.first_pkt = NULL;
	m_streamstate->videoq.last_pkt = NULL;
	m_streamstate->videoq.nb_packets = 0;
	m_streamstate->videoq.size = 0;
	m_streamstate->videoq.mutex = NULL;
	m_streamstate->videoq.cond = NULL;
	memset(m_streamstate->audio_buf,0,(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2);
	m_streamstate->audio_buf_size = 0;
	m_streamstate->audio_buf_index = 0;
	//AVPacket       audio_pkt;
	m_streamstate->audio_pkt_data = NULL;
	m_streamstate->audio_pkt_size = 0;
	m_streamstate->pictq[0].bmp = NULL;
	m_streamstate->pictq[0].width = 0;
	m_streamstate->pictq[0].height = 0;
	m_streamstate->pictq[0].allocated = 0;
	m_streamstate->pictq[0].pts = 0.0;
	m_streamstate->pictq_size = 0;
	m_streamstate->pictq_rindex = 0;
	m_streamstate->pictq_windex = 0;
	m_streamstate->pictq_mutex = NULL;
	m_streamstate->pictq_cond = NULL;
	m_streamstate->read_tid = NULL; 
	m_streamstate->video_tid = NULL;  
	m_streamstate->refresh_tid = NULL;
	m_streamstate->audio_clock = 0.0;
	m_streamstate->video_clock = 0.0;
	m_streamstate->seek_time = 0.0;
	m_streamstate->seek_req = 0;
	m_streamstate->seek_flags = 0;
	m_streamstate->seek_pos = 0;
	free(m_streamstate);
	m_streamstate = NULL;

	//////////////////////////////////////////////////////////////////////////
	m_fs_screen_width = 0;
	m_fs_screen_height = 0; 
	//AVPacket m_flush_pkt;
	m_sourceFile = "";
	m_file_duration = 0.0;
	m_video_dec_ctx = NULL; 
	m_audio_dec_ctx = NULL ; 
	m_pvideo_codec = NULL; 
	m_paudio_codec = NULL;
	m_screen = NULL;
	m_is_full_screen = 0;
	m_screen_width  = 0;
	m_screen_height = 0;
	m_Isstop = 0;
	m_pause_play = 0;                          
	m_slider_pos = 0;  
	m_stream_type = 0;

	//////////////////////////////////////////////////////////////////////////
	//video pram
	m_video_stream_idx = 0;              
	m_dbFrameRate = 0.0;               
	m_video_duration = 0.0;              
	m_dwWidth = 0;                          
	m_dwHeight = 0;                     
	m_video_codecID = AV_CODEC_ID_NONE;
	m_video_pixelfromat = AV_PIX_FMT_NONE;  
	memset(m_spspps,0,100);
	m_spspps_size = 0;

	//////////////////////////////////////////////////////////////////////////
	//audio pram
	m_audio_stream_idx = 0; 
	m_audio_duration = 0.0;              
	m_dwChannelCount = 0;              
	m_dwBitsPerSample = 0;             
	m_dwFrequency = 0;                 
	m_audio_codecID = AV_CODEC_ID_NONE;               
	m_audio_frame_size = 0;                  
	m_IsEnd_audio = 0;                         
	m_Volume_pos = 0;                          
	//////////////////////////////////////////////////////////////////////////
	m_Picture_rect.left = 0;
	m_Picture_rect.right = 0;
	m_Picture_rect.top = 0;
	m_Picture_rect.bottom = 0;
	//////////////////////////////////////////////////////////////////////////
	avformat_network_deinit();
	SDL_Quit();
	return 1;
}


int CMovies::InitVariable()
{
	//////////////////////////////////////////////////////////////////////////
	//����Ƶȫ�ֽṹ���ʼ��
	m_streamstate->pFormatCtx = NULL;
	m_streamstate->audio_st = NULL;
	m_streamstate->video_st = NULL;
	m_streamstate->audioq.first_pkt = NULL;
	m_streamstate->audioq.last_pkt = NULL;
	m_streamstate->audioq.nb_packets = 0;
	m_streamstate->audioq.size = 0;
	m_streamstate->audioq.mutex = NULL;
	m_streamstate->audioq.cond = NULL;
	m_streamstate->videoq.first_pkt = NULL;
	m_streamstate->videoq.last_pkt = NULL;
	m_streamstate->videoq.nb_packets = 0;
	m_streamstate->videoq.size = 0;
	m_streamstate->videoq.mutex = NULL;
	m_streamstate->videoq.cond = NULL;
	memset(m_streamstate->audio_buf,0,(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2);
	m_streamstate->audio_buf_size = 0;
	m_streamstate->audio_buf_index = 0;

	//AVPacket       audio_pkt;
	m_streamstate->audio_pkt_data = NULL;
	m_streamstate->audio_pkt_size = 0;
	m_streamstate->pictq[0].bmp = NULL;
	m_streamstate->pictq[0].width = 0;
	m_streamstate->pictq[0].height = 0;
	m_streamstate->pictq[0].allocated = 0;
	m_streamstate->pictq[0].pts = 0.0;
	m_streamstate->pictq_size = 0;
	m_streamstate->pictq_rindex = 0;
	m_streamstate->pictq_windex = 0;
	m_streamstate->pictq_mutex = NULL;
	m_streamstate->pictq_cond = NULL;

	//����ȫ�����������������
	m_streamstate->pictq_mutex = SDL_CreateMutex();
	m_streamstate->pictq_cond  = SDL_CreateCond();
	m_streamstate->read_tid = NULL; 
	m_streamstate->video_tid = NULL;  
	m_streamstate->refresh_tid = NULL;
	m_streamstate->audio_clock = 0.0;
	m_streamstate->video_clock = 0.0;
	m_streamstate->seek_time = 0.0;
	m_streamstate->seek_req = 0;
	m_streamstate->seek_flags = 0;
	m_streamstate->seek_pos = 0;

	//////////////////////////////////////////////////////////////////////////
	//m_fs_screen_width
	//m_fs_screen_height
	//ˢ��packet��ʼ��

	av_init_packet(&m_flush_pkt);
	m_flush_pkt.data = (uint8_t *)(intptr_t)"FLUSH";
	m_sourceFile = "";
	m_file_duration = 0.0;
	m_video_dec_ctx = NULL; 
	m_audio_dec_ctx = NULL ; 
	m_pvideo_codec = NULL; 
	m_paudio_codec = NULL;
	m_screen = NULL;
	m_is_full_screen = 0;

	//��ʼ��Ƶ��С //HWND hWnd ��Ĳ��Ŵ��ھ��
	//GetWindowRect(hWnd, &m_Picture_rect); 
	//m_screen_width = m_Picture_rect.right - m_Picture_rect.left;
	m_screen_width = M_WIDTH;
	//m_screen_height = m_Picture_rect.bottom - m_Picture_rect.top;
	m_screen_height = M_HEIGHT;

	//�����ǻ�ȡ�� STATIC�ؼ��Ĵ�С 
	//�������ж�����Ϊ ����ı��Сʱ �� �� �͸� ���� ������ͼ��᲻��ȷ �� �ж� Ҫż��
	if (m_screen_width % 2 != 0)
	{
		m_screen_width -= 1;
	}
	if (m_screen_height % 2 != 0)
	{
		m_screen_height -= 1;
	}
	m_Isstop = 0;

	//���������ļ��մ�ʱ���״̬0 ��ͣ 1���� 
	m_pause_play = 1;                           
	m_slider_pos = 0; 
	m_stream_type = 0;

	//////////////////////////////////////////////////////////////////////////
	//video pram
	m_video_stream_idx = -1;
	m_dbFrameRate = 25.0;
	m_video_duration = 0.040; 

	//��ʼ����ʱ�� ����Ϊstatic�ؼ��Ŀ�,��
	m_dwWidth = m_Picture_rect.right - m_Picture_rect.left;  
	m_dwHeight = m_Picture_rect.bottom - m_Picture_rect.top;      
	m_video_codecID = AV_CODEC_ID_H264;
	m_video_pixelfromat = AV_PIX_FMT_YUV420P;  
	memset(m_spspps,0,100);                    
	m_spspps_size = 0;

	//////////////////////////////////////////////////////////////////////////
	//audio pram
	m_audio_stream_idx = -1;
	m_audio_duration = 0; 
	m_dwChannelCount = 2;                  
	m_dwBitsPerSample = 16;                 
	m_dwFrequency = 44100;                       
	m_audio_codecID = AV_CODEC_ID_AAC;              
	m_audio_frame_size = 1024;   
	m_IsEnd_audio = 0;

	//�����ļ���ʱ��������С
	m_Volume_pos = 64;             
	//////////////////////////////////////////////////////////////////////////
	m_Picture_rect.left = 0;
	m_Picture_rect.right = 0;
	m_Picture_rect.top = 0;
	m_Picture_rect.bottom = 0;
	//////////////////////////////////////////////////////////////////////////
	return 1;
}


int CMovies::UinitVariable()
{
	VideoPicture * vp;
	int i;
	//�ͷ�packet_queue
	if (m_streamstate->read_tid)
	{
		SDL_WaitThread(m_streamstate->read_tid,NULL);
		//SDL_KillThread(m_streamstate->read_tid);
	}
	if (m_streamstate->refresh_tid)
	{
		//SDL_WaitThread(m_streamstate->refresh_tid,NULL);
		SDL_KillThread(m_streamstate->refresh_tid);

	}
	if (m_streamstate->videoq.mutex)
	{
		packet_queue_destroy(&m_streamstate->videoq);
	}
	if (m_streamstate->audioq.mutex)
	{
		packet_queue_destroy(&m_streamstate->audioq);
	}
	/* free all pictures */
	for (i = 0; i < VIDEO_PICTURE_QUEUE_SIZE; i++) 
	{
		vp = &m_streamstate->pictq[i];
		if (vp->bmp) 
		{
			SDL_FreeYUVOverlay(vp->bmp);
			vp->bmp = NULL;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	m_streamstate->pFormatCtx = NULL;
	m_streamstate->audio_st = NULL;
	m_streamstate->video_st = NULL;
	m_streamstate->audioq.first_pkt = NULL;
	m_streamstate->audioq.last_pkt = NULL;
	m_streamstate->audioq.nb_packets = 0;
	m_streamstate->audioq.size = 0;
	m_streamstate->audioq.mutex = NULL;
	m_streamstate->audioq.cond = NULL;
	m_streamstate->videoq.first_pkt = NULL;
	m_streamstate->videoq.last_pkt = NULL;
	m_streamstate->videoq.nb_packets = 0;
	m_streamstate->videoq.size = 0;
	m_streamstate->videoq.mutex = NULL;
	m_streamstate->videoq.cond = NULL;
	memset(m_streamstate->audio_buf,0,(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2);
	m_streamstate->audio_buf_size = 0;
	m_streamstate->audio_buf_index = 0;
	//ˢ��packet
	if (m_flush_pkt.data) 
	{
		av_free_packet(&m_flush_pkt);
	}
	m_streamstate->audio_pkt_data = NULL;
	m_streamstate->audio_pkt_size = 0;
	m_streamstate->pictq[0].bmp = NULL;
	m_streamstate->pictq[0].width = 0;
	m_streamstate->pictq[0].height = 0;
	m_streamstate->pictq[0].allocated = 0;
	m_streamstate->pictq[0].pts = 0.0;
	m_streamstate->pictq_size = 0;
	m_streamstate->pictq_rindex = 0;
	m_streamstate->pictq_windex = 0;
	//�ͷŻ������ ��������
	if (m_streamstate->pictq_mutex)
	{
		SDL_DestroyMutex(m_streamstate->pictq_mutex);
	}
	if (m_streamstate->pictq_cond)
	{
		SDL_DestroyCond(m_streamstate->pictq_cond);
	}
	m_streamstate->pictq_mutex = NULL;
	m_streamstate->pictq_cond = NULL;
	m_streamstate->read_tid = NULL; 
	m_streamstate->video_tid = NULL;  
	m_streamstate->refresh_tid = NULL;
	m_streamstate->audio_clock = 0.0;
	m_streamstate->video_clock = 0.0;
	m_streamstate->seek_time = 0.0;
	m_streamstate->seek_req = 0;
	m_streamstate->seek_flags = 0;
	m_streamstate->seek_pos = 0;

	//////////////////////////////////////////////////////////////////////////
	//m_fs_screen_width;
	//m_fs_screen_height; 
	//AVPacket m_flush_pkt;
	m_sourceFile = "";
	m_file_duration = 0.0;
	m_video_dec_ctx = NULL; 
	m_audio_dec_ctx = NULL ; 
	m_pvideo_codec = NULL; 
	m_paudio_codec = NULL;
	//�ͷŴ��� 
	if (m_screen)
	{ 
		SDL_FreeSurface( m_screen);
		m_screen = NULL;
	}
	m_is_full_screen = 0;
	m_screen_width  = 0;
	m_screen_height = 0;
	m_Isstop = 1;  //�������Ϊ1 Ҫ��Ȼ����߳̿��ܻ�û�˳� �������˱���
	m_pause_play = 0;                          
	m_slider_pos = 0;  
	m_stream_type = 0;

	//////////////////////////////////////////////////////////////////////////
	//video pram
	m_video_stream_idx = 0;              
	m_dbFrameRate = 0.0;               
	m_video_duration = 0.0;              
	m_dwWidth = 0;                          
	m_dwHeight = 0;                     
	m_video_codecID = AV_CODEC_ID_NONE;
	m_video_pixelfromat = AV_PIX_FMT_NONE;  
	memset(m_spspps,0,1000);
	m_spspps_size = 0;

	//////////////////////////////////////////////////////////////////////////
	//audio pram
	m_audio_stream_idx = 0; 
	m_audio_duration = 0.0;              
	m_dwChannelCount = 0;              
	m_dwBitsPerSample = 0;             
	m_dwFrequency = 0;                 
	m_audio_codecID = AV_CODEC_ID_NONE;               
	m_audio_frame_size = 0;                  
	m_IsEnd_audio = 0;   
	//////////////////////////////////////////////////////////////////////////
	m_Picture_rect.left = 0;
	m_Picture_rect.right = 0;
	m_Picture_rect.top = 0;
	m_Picture_rect.bottom = 0;
	//////////////////////////////////////////////////////////////////////////
	return 1;
}


int CMovies::Open_codec_context(int * stream_idx, AVFormatContext * fmt_ctx, enum AVMediaType type)
{
	int ret;
	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if (ret < 0)
	{
		return ret;
	} 
	else 
	{
		*stream_idx = ret;
	}
	return 0;
}


int CMovies::stream_component_open(StreamState *m_streamstate, unsigned stream_index)
{
	AVFormatContext *ic = m_streamstate->pFormatCtx;
	SDL_AudioSpec wanted_spec, spec;

	if (stream_index < 0 || stream_index >= ic->nb_streams)
	{
		return -1;
	}

	ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;

	/* prepare audio output */
	if (stream_index == m_audio_stream_idx) 
	{
		if (!m_paudio_codec || avcodec_open2(m_audio_dec_ctx, m_paudio_codec,NULL) < 0)
		{
			return -1;
		}

		// Set audio settings from codec info
		wanted_spec.freq = m_dwFrequency;

		switch (m_dwBitsPerSample)
		{
		case 8:
			wanted_spec.format = AUDIO_S8;
			break;
		case 16:
			wanted_spec.format = AUDIO_S16SYS;
			break;
		default:
			wanted_spec.format = AUDIO_S16SYS;
			break;
		}
		wanted_spec.channels = m_dwChannelCount;
		wanted_spec.silence = 0;//silenceֵ������Ϊsigned����Ϊ0 �ź�
		wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
		wanted_spec.callback = audio_callback;
		wanted_spec.userdata = this;
		wanted_spec.size = 0;  //SDL_OpenAudio().����ʱ����õ�

		//����Ƶ�豸
		if(SDL_OpenAudio(&wanted_spec, &spec) < 0)
		{
			fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
			return -1;
		}

		if (spec.format != AUDIO_S16SYS) 
		{
			fprintf(stderr, "SDL advised audio format %d m_streamstate not supported!\n", spec.format);
			return -1;
		}
		m_streamstate->audio_st = ic->streams[stream_index];
		m_streamstate->audio_buf_size  = 0;
		m_streamstate->audio_buf_index = 0;
		memset(&m_streamstate->audio_pkt, 0, sizeof(m_streamstate->audio_pkt));
		packet_queue_init(&m_streamstate->audioq);
		packet_queue_start(&m_streamstate->audioq);
		SDL_PauseAudio(0);
	}
	else if (stream_index == m_video_stream_idx)
	{
		if (!m_pvideo_codec || avcodec_open2(m_video_dec_ctx, m_pvideo_codec,NULL) < 0)
		{
			return -1;
		}
		m_streamstate->video_st = ic->streams[stream_index];
		packet_queue_init(&m_streamstate->videoq);
		packet_queue_start(&m_streamstate->videoq);
		m_streamstate->video_tid = SDL_CreateThread(video_thread, this);
	}
	return 1;
}


int CMovies::stream_component_close(StreamState *m_streamstate, unsigned stream_index)
{
	AVFormatContext *ic = m_streamstate->pFormatCtx;

	if (stream_index < 0 || stream_index >= ic->nb_streams)
	{
		return - 1;
	}

	ic->streams[stream_index]->discard = AVDISCARD_ALL;

	if (stream_index == m_audio_stream_idx) 
	{
		packet_queue_abort(&m_streamstate->audioq);
		SDL_CloseAudio();
		packet_queue_flush(&m_streamstate->audioq);
		av_free_packet(&m_streamstate->audio_pkt);
		m_streamstate->audio_st = NULL;
		avcodec_close(m_audio_dec_ctx);
	}
	else if (stream_index == m_video_stream_idx)
	{
		packet_queue_abort(&m_streamstate->videoq);
		/* note: we also signal this mutex to make sure we deblock the
		video thread in all cases */
		SDL_LockMutex(m_streamstate->pictq_mutex);
		SDL_CondSignal(m_streamstate->pictq_cond);
		SDL_UnlockMutex(m_streamstate->pictq_mutex);
		//SDL_WaitThread(m_streamstate->video_tid,NULL);
		SDL_KillThread(m_streamstate->video_tid);
		packet_queue_flush(&m_streamstate->videoq);
		m_streamstate->video_st = NULL;
		avcodec_close(m_video_dec_ctx);
	}
	return 1;
}


void CMovies::packet_queue_init(PacketQueue *q) 
{
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}


int CMovies::packet_queue_put(PacketQueue *q, AVPacket *pkt) 
{
	int ret;

	/* duplicate the packet */
	if (pkt != &m_flush_pkt && av_dup_packet(pkt) < 0)
	{
		return -1;
	}

	SDL_LockMutex(q->mutex);
	ret = packet_queue_put_private(q, pkt);
	SDL_UnlockMutex(q->mutex);

	if (pkt != &m_flush_pkt && ret < 0)
	{
		av_free_packet(pkt);
	}

	return ret;
}


int CMovies::packet_queue_get(PacketQueue *q, AVPacket *pkt, int queue_type)
{
	AVPacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for(;;)
	{
		pkt1 = q->first_pkt;
		if (pkt1) 
		{
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
			{
				q->last_pkt = NULL;
			}
			q->nb_packets--;
			//q->size -= pkt1->pkt.size;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		}
		else 
		{
			int sdlcondwait = SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}


void CMovies::packet_queue_abort(PacketQueue *q)
{
	SDL_LockMutex(q->mutex);
	SDL_CondSignal(q->cond);
	SDL_UnlockMutex(q->mutex);
}


int CMovies::packet_queue_put_private(PacketQueue *q, AVPacket *pkt)
{
	AVPacketList *pkt1;

	pkt1 = (AVPacketList * )av_malloc(sizeof(AVPacketList));
	if (!pkt1)
	{
		return -1;
	}
	pkt1->pkt = *pkt;
	pkt1->next = NULL;

	if (!q->last_pkt)
	{
		q->first_pkt = pkt1;
	}
	else
	{
		q->last_pkt->next = pkt1;
	}
	q->last_pkt = pkt1;
	q->nb_packets++;
	//q->size += pkt1->pkt.size + sizeof(*pkt1);
	q->size += pkt1->pkt.size;
	/* XXX: should duplicate packet data in DV case */
	SDL_CondSignal(q->cond);
	return 0;
}


void CMovies::packet_queue_start(PacketQueue *q)
{
	SDL_LockMutex(q->mutex);
	packet_queue_put_private(q, &m_flush_pkt);
	SDL_UnlockMutex(q->mutex);
}


void CMovies::packet_queue_destroy(PacketQueue *q)
{
	packet_queue_flush(q);
	SDL_DestroyMutex(q->mutex);
	SDL_DestroyCond(q->cond);
}


void CMovies::packet_queue_flush(PacketQueue *q) 
{
	AVPacketList *pkt, *pkt1;

	SDL_LockMutex(q->mutex);
	for(pkt = q->first_pkt; pkt != NULL; pkt = pkt1) 
	{
		pkt1 = pkt->next;
		av_free_packet(&pkt->pkt);
		av_freep(&pkt);
	}

	q->last_pkt = NULL;
	q->first_pkt = NULL;
	q->nb_packets = 0;
	q->size = 0;
	SDL_UnlockMutex(q->mutex);
}


void CMovies::video_refresh(void * opaque)
{
	StreamState *streamstate = (StreamState *)opaque;
	if (streamstate == NULL)
	{
		return;
	}
	VideoPicture *vp;
	double delay,diff;

	//////////////////////////////////////////////////////////////////////////
	//���õ�ǰʱ����ʾ
	int h_minute = 0;
	int h_second = 0;

	//���ֻ����Ƶ
	if (m_stream_type == 3)
	{
		h_minute = (int)m_streamstate->video_clock / 60;
		h_second = (int)m_streamstate->video_clock % 60;
	}
	//���ֻ����Ƶ ����
	else 
	{
		h_minute = (int)m_streamstate->audio_clock / 60;
		h_second = (int)m_streamstate->audio_clock % 60;
	}
	//////////////////////////////////////////////////////////////////////////

	if(streamstate->video_st)
	{
		if(streamstate->pictq_size == 0)
		{
			Sleep(1);
		} 
		else
		{
			//ȡ��Ҫ��ʾ��Ƶ��ͼ������
			vp = &streamstate->pictq[streamstate->pictq_rindex];
			//���һ֡��Ƶ��Ӧ����ʾ��ʱ��
			delay = m_video_duration;
			/* update delay to sync to audio */
			diff = vp->pts - streamstate->audio_clock;

			if(fabs(diff) < AV_NOSYNC_THRESHOLD) //�󸡵���x�ľ���ֵ
			{
				if(diff <= -delay) 
				{
					delay = 0.01;       //��� ��Ƶ��ʾ����������Ƶ ����Զ �� ��ʾʱ��Ϊ10ms
				} 
				else if(diff >= delay)
				{
					delay = 2 * delay;  //��� ��Ƶ��ʾ���� ��ͣ�� ��֡��ʱ��
				}
			}

			//���ֻ����Ƶ ���������� 
			if (m_stream_type == 3 && m_dbFrameRate == 25 )
			{
				//������ˢ��ʱ����
				Sleep(m_video_duration * 1000);
			}
			else
			{
				Sleep(delay * 1000);
			}

			/* update queue for next picture! */
			if(++streamstate->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) 
			{
				streamstate->pictq_rindex = 0;
			}

			SDL_LockMutex(streamstate->pictq_mutex);
			streamstate->pictq_size--;
			SDL_CondSignal(streamstate->pictq_cond);
			SDL_UnlockMutex(streamstate->pictq_mutex);

			/* show the picture! */
			video_display(streamstate);  
		}
	}

	else if (streamstate->audio_st) 
	{
		/* draw the next audio frame */

		/* if only audio stream, then display the audio bars (better
		than nothing, just to test the implementation */
		video_display(streamstate);
	}
}


int CMovies::audio_decode_frame(StreamState *m_streamstate, uint8_t *audio_buf, int buf_size,double *pts_ptr) 
{
	int len1, data_size, n;
	AVPacket *pkt = &m_streamstate->audio_pkt;
	m_streamstate->audio_pkt_size  = 0;
	double pts;

	for(;;) 
	{
		if (m_Isstop || m_IsEnd_audio )
		{
			return -1;
		}
		while(m_streamstate->audio_pkt_size > 0) 
		{
			data_size = buf_size;
			len1 = avcodec_decode_audio3(m_streamstate->audio_st->codec, (int16_t *)audio_buf, &data_size,pkt);
			if(len1 < 0) 
			{
				/* if error, skip frame */
				m_streamstate->audio_pkt_size = 0;
				break;
			}
			m_streamstate->audio_pkt_data += len1;
			m_streamstate->audio_pkt_size -= len1;
			if(data_size <= 0)
			{
				/* No data yet, get more frames */
				continue;
			}
			pts = m_streamstate->audio_clock;
			*pts_ptr = pts;

			//�����һ���ж������ڶԲ�ͬ�ĸ�ʽ����pts��λ��ͳһ����ͳһ�� ���롱������չ������ʽ
			if (strstr(m_streamstate->pFormatCtx->iformat->name,"mpegts")!= NULL)
			{
				double time_base = 90 * 1000;
				n = 2 * m_streamstate->audio_st->codec->channels;
				double rate_size = data_size / n;
				m_streamstate->audio_clock +=  rate_size * time_base/ m_streamstate->audio_st->codec->sample_rate /100000;
			}
			else
			{
				n = 2 * m_streamstate->audio_st->codec->channels;
				double rate_size = data_size / n;
				m_streamstate->audio_clock +=  rate_size / m_streamstate->audio_st->codec->sample_rate;
			}
			/* We have data, return it and come back for more later */
			return data_size;
		}
		if(pkt->data)
		{
			av_free_packet(pkt);
		}

		if (m_pause_play)
		{
			/* next packet */
			if(packet_queue_get(&m_streamstate->audioq, pkt, AUDIO_ID) < 0)
			{
				return -1;
			}
			if(pkt->data == m_flush_pkt.data) 
			{
				avcodec_flush_buffers(m_streamstate->audio_st->codec);
				continue;
			}
			m_streamstate->audio_pkt_data = pkt->data;
			m_streamstate->audio_pkt_size = pkt->size;
		}
	}
}


int CMovies::queue_picture(StreamState *m_streamstate, AVFrame *pFrame,double pts) 
{
	VideoPicture * vp = NULL;
	AVFrame * pOutputFrame = NULL;
	uint8_t * pOutput_buf = NULL;
	int Out_size = 0;
	struct SwsContext * img_convert_ctx;

	pOutputFrame = avcodec_alloc_frame(); 

	/* wait until we have space for a new pic */
	SDL_LockMutex(m_streamstate->pictq_mutex);
	while (m_streamstate->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE ) 
	{
		SDL_CondWait(m_streamstate->pictq_cond, m_streamstate->pictq_mutex);
	}
	SDL_UnlockMutex(m_streamstate->pictq_mutex);

	// windex m_streamstate set to 0 initially
	vp = &m_streamstate->pictq[m_streamstate->pictq_windex];

	/* allocate or resize the buffer! */
	if(!vp->bmp ||
		vp->width != m_screen_width ||
		vp->height != m_screen_height) 
	{
		vp->allocated = 0;
		/* we have to do it in the main thread */
		alloc_picture(m_streamstate);
		/* wait until we have a picture allocated */
		SDL_LockMutex(m_streamstate->pictq_mutex);
		while (!vp->allocated) 
		{
			SDL_CondWait(m_streamstate->pictq_cond, m_streamstate->pictq_mutex);
		}
		
		SDL_UnlockMutex(m_streamstate->pictq_mutex);
	}
	/* We have a place to put our picture on the queue */

	if(vp->bmp) 
	{

		SDL_LockYUVOverlay(vp->bmp);
		/* point pict at the queue */

		pOutputFrame->data[0] = vp->bmp->pixels[0];
		pOutputFrame->data[1] = vp->bmp->pixels[2];
		pOutputFrame->data[2] = vp->bmp->pixels[1];

		pOutputFrame->linesize[0] = vp->bmp->pitches[0];
		pOutputFrame->linesize[1] = vp->bmp->pitches[2];
		pOutputFrame->linesize[2] = vp->bmp->pitches[1];

		img_convert_ctx = sws_getContext(pFrame->width,pFrame->height, m_streamstate->video_st->codec->pix_fmt,
			m_screen_width,m_screen_height,AV_PIX_FMT_YUV420P,SWS_BICUBIC, NULL,NULL,NULL);      
		if(img_convert_ctx == NULL)
		{
			fprintf(stderr, "Cannot initialize the conversion context!\n");
			getchar();
		}

		// ��ͼƬת��ΪRGB��ʽ
		sws_scale(img_convert_ctx,pFrame->data,pFrame->linesize,0,
			pFrame->height,pOutputFrame->data,pOutputFrame->linesize);

		SDL_UnlockYUVOverlay(vp->bmp);
		vp->pts = pts;
		/* now we inform our display thread that we have a pic ready */
		if(++m_streamstate->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) 
		{
			m_streamstate->pictq_windex = 0;
		}
		SDL_LockMutex(m_streamstate->pictq_mutex);
		m_streamstate->pictq_size++;
		SDL_UnlockMutex(m_streamstate->pictq_mutex);
	}
	return 0;
}


void CMovies::video_display(StreamState *m_streamstate) 
{
	SDL_Rect rect;
	VideoPicture *vp;

	if (!m_screen)
	{
		video_open(m_streamstate); 
	}
	vp = &m_streamstate->pictq[m_streamstate->pictq_rindex];
	if(vp->bmp) 
	{
		if (vp->bmp->w != m_screen_width || vp->bmp->h != m_screen_height)
		{
			//����һ��Ҫע�� ��sdl�Ĵ��ڴ�С�ı��ʱ�� �л���һ˲�� m_streamstate->pictq[m_streamstate->pictq_rindex] ����������п��ܻ���ԭ�����ڴ�С������ 
			//��ʱ����һ֡������sdl�����Ϸ�ֹ����������Ҳ������get_next_frame����
			return ;
		}
		rect.x = 0;
		rect.y = 0;
		rect.w = m_screen_width;
		rect.h = m_screen_height;
		SDL_DisplayYUVOverlay(vp->bmp, &rect);
	}
}


void CMovies::alloc_picture(void *userdata)
{
	StreamState *m_streamstate = (StreamState *)userdata;
	VideoPicture *vp;

	vp = &m_streamstate->pictq[m_streamstate->pictq_windex];
	if(vp->bmp) 
	{
		// we already have one make another, bigger/smaller
		SDL_FreeYUVOverlay(vp->bmp);
	}

	video_open(m_streamstate); 
	// Allocate a place to put our YUV image on that screen
	vp->bmp = SDL_CreateYUVOverlay(m_screen_width,
		m_screen_height,
		SDL_YV12_OVERLAY,
		m_screen);
	vp->width = m_screen_width;
	vp->height = m_screen_height;

	SDL_LockMutex(m_streamstate->pictq_mutex);
	vp->allocated = 1;
	SDL_CondSignal(m_streamstate->pictq_cond);
	SDL_UnlockMutex(m_streamstate->pictq_mutex);
}


int CMovies::video_open(StreamState *m_streamstate)
{
	int flags = SDL_HWSURFACE | SDL_ASYNCBLIT | SDL_HWACCEL | SDL_NOFRAME;
	int w,h;
	VideoPicture *vp = &m_streamstate->pictq[m_streamstate->pictq_rindex];

	////�����ǻ�ȡ�� ������ �Ĵ�С 
	////�������ж�����Ϊ ����ı��Сʱ �� �� �͸� ���� ������ͼ��᲻��ȷ �� �ж� Ҫż��
	if (m_is_full_screen)
	{
		flags |= SDL_FULLSCREEN;
	}
	else         
	{
		flags |= SDL_RESIZABLE;
	}

	if (m_is_full_screen && m_fs_screen_width) 
	{
		w = m_fs_screen_width;
		h = m_fs_screen_height;
	} 
	else
	{
		//Ҫ�ı�Ĵ�С
		w = m_screen_width;
		h = m_screen_height;
	}

	if (m_screen && m_screen_width == m_screen->w && m_screen->w == w
		&& m_screen_height== m_screen->h && m_screen->h == h)
	{
		return 0;
	}

	m_screen = SDL_SetVideoMode(w, h, 0, flags);
	
	//SDLˢ�� �������Ȼ����С������ 
	SDL_Flip(m_screen);

	if (!m_screen)
	{
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		OnStopMovies();
	}
	return 0;
}


int CMovies::read_thread( LPVOID lpParam )
{
	CMovies * pDlg = (CMovies *)lpParam;
	//ASSERT(pDlg);

	int ret = -1;
	int error = -1;
	int eof = 0;
	AVPacket pkt1, *pkt = &pkt1;

	//����ˢ���߳�
	pDlg->m_streamstate->refresh_tid = SDL_CreateThread(refresh_thread, pDlg);

	/* open the streams */
	if (AVMEDIA_TYPE_AUDIO >= 0)
	{
		ret = pDlg->stream_component_open(pDlg->m_streamstate, AVMEDIA_TYPE_AUDIO);
	}
	if (AVMEDIA_TYPE_VIDEO >= 0)
	{
		ret = pDlg->stream_component_open(pDlg->m_streamstate, AVMEDIA_TYPE_VIDEO);
	}

	if (pDlg->m_video_stream_idx < 0 && pDlg->m_audio_stream_idx < 0) 
	{
		ret = -1;
		goto fail;
	}

	for(;;)
	{
		if (pDlg->m_Isstop || eof)
		{
			break;
		}
		// seek stuff goes here
		if(pDlg->m_streamstate->seek_req) 
		{
			int stream_index = -1;
			//ת��������
			int64_t seek_target = pDlg->m_streamstate->seek_pos * 1000000;

			//�������ֻ����Ƶ
			if (pDlg->m_stream_type != 2)
			{
				if(pDlg->m_video_stream_idx >= 0)
				{
					stream_index = pDlg->m_video_stream_idx;
				}
				else if(pDlg->m_audio_stream_idx >= 0) 
				{
					stream_index = pDlg->m_audio_stream_idx;
				}
			}
			else
			{
				if(pDlg->m_audio_stream_idx >= 0) 
				{
					stream_index = pDlg->m_audio_stream_idx;
				}
				else if(pDlg->m_video_stream_idx >= 0)
				{
					stream_index = pDlg->m_video_stream_idx;
				}
			}
			if(stream_index >= 0)
			{
				AVRational time_base_q ;
				time_base_q.num = 1;
				time_base_q.den = AV_TIME_BASE;

				//����һ��Ҫע�⣺���������Ǵ���ת�ɺ��룬//seek_target = seek_target / 1000; �������ǲ��Ե�
				seek_target = av_rescale_q(seek_target, time_base_q, pDlg->m_streamstate->pFormatCtx->streams[stream_index]->time_base);
			}
			error = av_seek_frame(pDlg->m_streamstate->pFormatCtx, stream_index, seek_target, pDlg->m_streamstate->seek_flags);

			//seek�ɹ�
			if( error >= 0)
			{
				if(pDlg->m_audio_stream_idx >= 0) 
				{
					pDlg->packet_queue_flush(&pDlg->m_streamstate->audioq);
					pDlg->packet_queue_put(&pDlg->m_streamstate->audioq, &pDlg->m_flush_pkt);
				}
				if(pDlg->m_video_stream_idx >= 0)
				{
					pDlg->packet_queue_flush(&pDlg->m_streamstate->videoq);
					pDlg->packet_queue_put(&pDlg->m_streamstate->videoq, &pDlg->m_flush_pkt);
				}

				//����Ҫ��������video_clock �� audio_clock
				//�������Ƶ����
				if (pDlg->m_stream_type == 1)
				{
					pDlg->m_streamstate->video_clock = ((int)(pDlg->m_streamstate->seek_pos / pDlg->m_video_duration) + 1) * pDlg->m_video_duration ;
					if (strstr(pDlg->m_streamstate->pFormatCtx->iformat->name,"mpegts")!= NULL)
					{
						double time_base = 90 * 1000;
						pDlg->m_streamstate->audio_clock = (pDlg->m_streamstate->seek_pos / (1024.0 * time_base/ pDlg->m_streamstate->audio_st->codec->sample_rate /100000) + 1) * 
							(1024.0 * time_base/ pDlg->m_streamstate->audio_st->codec->sample_rate /100000);
					}
					else
					{
						pDlg->m_streamstate->audio_clock = (double)pDlg->m_streamstate->seek_pos ;
						pDlg->m_streamstate->audio_clock = ((pDlg->m_streamstate->seek_pos / (1024.0 / pDlg->m_streamstate->audio_st->codec->sample_rate)) + 1) * 
							(1024.0 / pDlg->m_streamstate->audio_st->codec->sample_rate);
					}
				}

				//ֻ����Ƶ
				else if (pDlg->m_stream_type == 2)
				{
					if (strstr(pDlg->m_streamstate->pFormatCtx->iformat->name,"mpegts")!= NULL)
					{
						double time_base = 90 * 1000;
						pDlg->m_streamstate->audio_clock = (pDlg->m_streamstate->seek_pos / (1024.0 * time_base/ pDlg->m_streamstate->audio_st->codec->sample_rate /100000) + 1) * 
							(1024.0 * time_base/ pDlg->m_streamstate->audio_st->codec->sample_rate /100000);
					}
					else
					{
						pDlg->m_streamstate->audio_clock = (double)pDlg->m_streamstate->seek_pos ;
						pDlg->m_streamstate->audio_clock = ((pDlg->m_streamstate->seek_pos / (1024.0 / pDlg->m_streamstate->audio_st->codec->sample_rate)) + 1) * 
							(1024.0 / pDlg->m_streamstate->audio_st->codec->sample_rate);
					}
				}

				//ֻ����Ƶ
				else if (pDlg->m_stream_type == 3)
				{
					pDlg->m_streamstate->video_clock = ((int)(pDlg->m_streamstate->seek_pos / pDlg->m_video_duration) + 1) * pDlg->m_video_duration ;
				}
			} 
			pDlg->m_streamstate->seek_req = 0;
			pDlg->m_streamstate->seek_time = 0;
			eof = 0;
		}

		/* if the queue are full, no need to read more */
		if (   pDlg->m_streamstate->audioq.size + pDlg->m_streamstate->videoq.size > MAX_QUEUE_SIZE
			|| (   (pDlg->m_streamstate->audioq.nb_packets > MIN_FRAMES )
			&& (pDlg->m_streamstate->videoq.nb_packets > MIN_FRAMES)))
		{
			/* wait 10 ms */
			SDL_Delay(10);
			continue;
		}

		if (eof)
		{
			//���ļ�ĩβ �������һ���հ�
			if (pDlg->m_video_stream_idx >= 0) 
			{
				av_init_packet(pkt);
				pkt->data = NULL;
				pkt->size = 0;
				pkt->stream_index = pDlg->m_video_stream_idx;
				pDlg->packet_queue_put(&pDlg->m_streamstate->videoq, pkt);
			}

			if (pDlg->m_audio_stream_idx >= 0 ) 
			{
				if (pDlg->m_streamstate->audioq.nb_packets == 0)
				{
					pDlg->m_IsEnd_audio = 1;
				}
				av_init_packet(pkt);
				pkt->data = NULL;
				pkt->size = 0;
				pkt->stream_index = pDlg->m_audio_stream_idx;
				pDlg->packet_queue_put(&pDlg->m_streamstate->audioq, pkt);
			}

			SDL_Delay(10);
			if (pDlg->m_streamstate->audioq.size + pDlg->m_streamstate->videoq.size == 0) 
			{
				ret = AVERROR_EOF;
				goto fail;
			}

			eof = 0;
			continue;
		}

		//����ȷ���ļ�����ĩβ
		ret = av_read_frame(pDlg->m_streamstate->pFormatCtx, pkt);
		if (ret < 0) 
		{
			if (ret == AVERROR_EOF || url_feof(pDlg->m_streamstate->pFormatCtx->pb))
			{
				eof = 1;
			}
			if (pDlg->m_streamstate->pFormatCtx->pb && pDlg->m_streamstate->pFormatCtx->pb->error)
			{
				break;
			}

			SDL_Delay(100); /* wait for user event */
			continue;
		}

		// Is this a packet from the video stream?
		if(pkt->stream_index == pDlg->m_video_stream_idx)
		{
			pDlg->packet_queue_put(&pDlg->m_streamstate->videoq, pkt);
		} 
		else if(pkt->stream_index == pDlg->m_audio_stream_idx) 
		{
			pDlg->packet_queue_put(&pDlg->m_streamstate->audioq, pkt);
		} 
		else 
		{
			av_free_packet(pkt);
		}
	}
	ret = 0;

fail:
	/* close each stream */
	VideoPicture * vp;
	int i;
	pDlg->m_Isstop = 1;

	if (pDlg->m_audio_stream_idx >= 0)
	{
		pDlg->stream_component_close(pDlg->m_streamstate, pDlg->m_audio_stream_idx);
	}
	if (pDlg->m_video_stream_idx >= 0)
	{
		pDlg->stream_component_close(pDlg->m_streamstate,pDlg->m_video_stream_idx);
	}
	if (pDlg->m_streamstate->pFormatCtx) 
	{
		avformat_close_input(&pDlg->m_streamstate->pFormatCtx);
	}
	if (ret == AVERROR_EOF) 
	{
		//�˳�����
		//////////////////////////////////////////////////////////////////////////
		if (pDlg->m_streamstate->refresh_tid)
		{
			//SDL_WaitThread(pDlg->m_streamstate->refresh_tid,NULL);
			SDL_KillThread(pDlg->m_streamstate->refresh_tid);
		}
		if (pDlg->m_streamstate->videoq.mutex)
		{
			pDlg->packet_queue_destroy(&pDlg->m_streamstate->videoq);
		}
		if (pDlg->m_streamstate->audioq.mutex)
		{
			pDlg->packet_queue_destroy(&pDlg->m_streamstate->audioq);
		}
		/* free all pictures */
		for (i = 0; i < VIDEO_PICTURE_QUEUE_SIZE; i++) 
		{
			vp = &pDlg->m_streamstate->pictq[i];
			if (vp->bmp) 
			{
				SDL_FreeYUVOverlay(vp->bmp);
				vp->bmp = NULL;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		pDlg->m_streamstate->pFormatCtx = NULL;
		pDlg->m_streamstate->audio_st = NULL;
		pDlg->m_streamstate->video_st = NULL;
		pDlg->m_streamstate->audioq.first_pkt = NULL;
		pDlg->m_streamstate->audioq.last_pkt = NULL;
		pDlg->m_streamstate->audioq.nb_packets = 0;
		pDlg->m_streamstate->audioq.size = 0;
		pDlg->m_streamstate->audioq.mutex = NULL;
		pDlg->m_streamstate->audioq.cond = NULL;
		pDlg->m_streamstate->videoq.first_pkt = NULL;
		pDlg->m_streamstate->videoq.last_pkt = NULL;
		pDlg->m_streamstate->videoq.nb_packets = 0;
		pDlg->m_streamstate->videoq.size = 0;
		pDlg->m_streamstate->videoq.mutex = NULL;
		pDlg->m_streamstate->videoq.cond = NULL;
		memset(pDlg->m_streamstate->audio_buf,0,(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2);
		pDlg->m_streamstate->audio_buf_size = 0;
		pDlg->m_streamstate->audio_buf_index = 0;

		//ˢ��packet
		if (pDlg->m_flush_pkt.data) 
		{
			av_free_packet(&pDlg->m_flush_pkt);
		}
		pDlg->m_streamstate->audio_pkt_data = NULL;
		pDlg->m_streamstate->audio_pkt_size = 0;
		pDlg->m_streamstate->pictq[0].bmp = NULL;
		pDlg->m_streamstate->pictq[0].width = 0;
		pDlg->m_streamstate->pictq[0].height = 0;
		pDlg->m_streamstate->pictq[0].allocated = 0;
		pDlg->m_streamstate->pictq[0].pts = 0.0;
		pDlg->m_streamstate->pictq_size = 0;
		pDlg->m_streamstate->pictq_rindex = 0;
		pDlg->m_streamstate->pictq_windex = 0;

		//�ͷŻ������ ��������
		if (pDlg->m_streamstate->pictq_mutex)
		{
			SDL_DestroyMutex(pDlg->m_streamstate->pictq_mutex);
		}
		if (pDlg->m_streamstate->pictq_cond)
		{
			SDL_DestroyCond(pDlg->m_streamstate->pictq_cond);
		}

		pDlg->m_streamstate->pictq_mutex = NULL;
		pDlg->m_streamstate->pictq_cond = NULL;
		pDlg->m_streamstate->read_tid = NULL; 
		pDlg->m_streamstate->video_tid = NULL;  
		pDlg->m_streamstate->refresh_tid = NULL;
		pDlg->m_streamstate->audio_clock = 0.0;
		pDlg->m_streamstate->video_clock = 0.0;
		pDlg->m_streamstate->seek_time = 0.0;
		pDlg->m_streamstate->seek_req = 0;
		pDlg->m_streamstate->seek_flags = 0;
		pDlg->m_streamstate->seek_pos = 0;

		//////////////////////////////////////////////////////////////////////////
		//pDlg->m_fs_screen_width;
		//pDlg->m_fs_screen_height; 
		//AVPacket m_flush_pkt;
		pDlg->m_sourceFile = "";
		pDlg->m_file_duration = 0.0;
		pDlg->m_video_dec_ctx = NULL; 
		pDlg->m_audio_dec_ctx = NULL ; 
		pDlg->m_pvideo_codec = NULL; 
		pDlg->m_paudio_codec = NULL;

		//�ͷŴ��� 
		if (pDlg->m_screen)
		{ 
			SDL_FreeSurface( pDlg->m_screen);
			pDlg->m_screen = NULL;
		}

		pDlg->m_is_full_screen = 0;
		pDlg->m_screen_width  = 0;
		pDlg->m_screen_height = 0;
		pDlg->m_Isstop = 0;
		pDlg->m_pause_play = 0;                          
		pDlg->m_slider_pos = 0;  
		pDlg->m_stream_type = 0;

		//////////////////////////////////////////////////////////////////////////
		//video pram
		pDlg->m_video_stream_idx = 0;              
		pDlg->m_dbFrameRate = 0.0;               
		pDlg->m_video_duration = 0.0;              
		pDlg->m_dwWidth = 0;                          
		pDlg->m_dwHeight = 0;                     
		pDlg->m_video_codecID = AV_CODEC_ID_NONE;
		pDlg->m_video_pixelfromat = AV_PIX_FMT_NONE;  
		memset(pDlg->m_spspps,0,100);
		pDlg->m_spspps_size = 0;

		//////////////////////////////////////////////////////////////////////////
		//audio pram
		pDlg->m_audio_stream_idx = 0; 
		pDlg->m_audio_duration = 0.0;              
		pDlg->m_dwChannelCount = 0;              
		pDlg->m_dwBitsPerSample = 0;             
		pDlg->m_dwFrequency = 0;                 
		pDlg->m_audio_codecID = AV_CODEC_ID_NONE;               
		pDlg->m_audio_frame_size = 0;                  
		pDlg->m_IsEnd_audio = 0;   
		//////////////////////////////////////////////////////////////////////////
		pDlg->m_Picture_rect.left = 0;
		pDlg->m_Picture_rect.right = 0;
		pDlg->m_Picture_rect.top = 0;
		pDlg->m_Picture_rect.bottom = 0;
		//////////////////////////////////////////////////////////////////////////
	}

	HideMWindow();
	return 1;
}


int CMovies::refresh_thread(LPVOID lpParam)
{
	CMovies * pDlg = (CMovies *)lpParam;
	//ASSERT(pDlg);

	while (!pDlg->m_Isstop) 
	{
		//���ˢ�º���
		pDlg->video_refresh(pDlg->m_streamstate);
		//FIXME ideally we should wait the correct time but SDLs event passing m_streamstate so slow it would be silly
	}

	return 0;
}


void CMovies::audio_callback(void * userdata, unsigned char *stream, int len) 
{
	CMovies * pDlg = (CMovies *)userdata;
	//ASSERT(pDlg);
	int len1, audio_size;
	double pts;

	while(len > 0) 
	{
		if(pDlg->m_streamstate->audio_buf_index >= pDlg->m_streamstate->audio_buf_size)
		{
			/* We have already sent all our data; get more */
			int size = sizeof(pDlg->m_streamstate->audio_buf);
			audio_size = pDlg->audio_decode_frame(pDlg->m_streamstate, pDlg->m_streamstate->audio_buf, sizeof(pDlg->m_streamstate->audio_buf),&pts);
			if(audio_size < 0)
			{
				/* If error, output silence */
				pDlg->m_streamstate->audio_buf_size = 1024;
				memset(pDlg->m_streamstate->audio_buf, 0, pDlg->m_streamstate->audio_buf_size);
			}
			else
			{
				pDlg->m_streamstate->audio_buf_size = audio_size;
			}
			pDlg->m_streamstate->audio_buf_index = 0;
		}
		len1 = pDlg->m_streamstate->audio_buf_size - pDlg->m_streamstate->audio_buf_index;
		if(len1 > len)
		{
			len1 = len;
		}
		//////////////////////////////////////////////////////////////////////////
		//memcpy(stream, (unsigned char  *)pDlg->m_streamstate->audio_buf + pDlg->m_streamstate->audio_buf_index, len1);
		//�����ǿ���������С��
		SDL_MixAudio(stream, (unsigned char  *)pDlg->m_streamstate->audio_buf + pDlg->m_streamstate->audio_buf_index, len1, pDlg->m_Volume_pos);
		//////////////////////////////////////////////////////////////////////////
		len -= len1;
		stream += len1;
		pDlg->m_streamstate->audio_buf_index += len1;
	}
}


int CMovies::video_thread(void *arg)
{
	CMovies * pDlg = (CMovies *)arg;
	//ASSERT(pDlg);

	//AVPacket pkt1, *packet = &pkt1;
	int frameFinished;
	AVFrame * pFrame = NULL;

	pFrame = avcodec_alloc_frame();

	for(;;) 
	{
		AVPacket pkt;
		if (pDlg->m_Isstop)
		{
			break;
		}
		if (pDlg->m_pause_play) //������ڲ���
		{
			if(pDlg->packet_queue_get(&pDlg->m_streamstate->videoq, &pkt, VIDEO_ID) < 0)
			{
				// means we quit getting packets
				break;
			}
			if(pkt.data == pDlg->m_flush_pkt.data) 
			{
				avcodec_flush_buffers(pDlg->m_streamstate->video_st->codec);
				continue;
			}
			// Decode video frame
			avcodec_decode_video2(pDlg->m_streamstate->video_st->codec, pFrame, &frameFinished,&pkt);
			av_free_packet(&pkt);

			// Did we get a video frame?
			if(frameFinished) 
			{
				pDlg->m_streamstate->video_clock +=  pDlg->m_video_duration;
				if(pDlg->queue_picture(pDlg->m_streamstate, pFrame,pDlg->m_streamstate->video_clock) < 0)  
				{
					break;
				}
			}
		}
		else
		{
			continue;
		}
	}
	avcodec_free_frame(&pFrame);
	return 0;
}


void CMovies::OnStartMovies(char* Path)
{
	//////////////////////////////////////////////////////////////////////////
	//����һ��Ҫ�������� ����ļ����ڲ��� ���´�һ���µ��ļ�ʱ Ҫ����������

	//�ر��̵߳�
	OnStopMovies(); 

	//��ʼ��������
	InitVariable();
	Sleep(100);
	//////////////////////////////////////////////////////////////////////////

	//������Ƶ����Դ��������Ƶ�ļ���
	m_sourceFile = Path;

	 int err_code;
	 char* buf = NULL;

         // ת��
	 //USES_CONVERSION;
	 //const char *pStr = T2CA(m_sourceFile);
	 const char *pStr = m_sourceFile;

   	//��Ҫ��Ӧ�õ������ļ�
	if (err_code = avformat_open_input(&m_streamstate->pFormatCtx,pStr, NULL, NULL) < 0)                 
	{
		// err_code �������.
		av_strerror(err_code, buf, 1024);
		return;
	}

	//�鿴����Ϣ
	if (avformat_find_stream_info(m_streamstate->pFormatCtx, NULL) < 0)                                   
	{
		//FFMPEG ��ȡ�ļ���Ϣʧ��
		return;
	}

	//������Ϣ����д��
	av_dump_format(m_streamstate->pFormatCtx, 0, m_sourceFile, 0); 

	//�õ��ļ���ʱ��
	m_file_duration = m_streamstate->pFormatCtx->duration / 1000.0 / 1000.0; //������ת������

	//���û���ļ���ʱ��
	if (m_file_duration < 0)
	{
		m_file_duration = 0;
	}


	//�õ���Ƶ��Ϣ��CONTEXT
	if (Open_codec_context(&m_video_stream_idx, m_streamstate->pFormatCtx, AVMEDIA_TYPE_VIDEO) >= 0)
	{
		m_streamstate->video_st = m_streamstate->pFormatCtx->streams[m_video_stream_idx];
		m_video_dec_ctx = m_streamstate->video_st->codec;
		// Find the decoder for the video stream
		m_pvideo_codec = avcodec_find_decoder(m_video_dec_ctx->codec_id);
		if(m_pvideo_codec == NULL) 
		{
			// Find video decoderʧ�� 
			return;
		}
		// Open codec
		if(avcodec_open(m_video_dec_ctx, m_pvideo_codec) < 0)
		{
			// Could not open video codec
			return;
		}

		//��Ƶ��֡��
		m_dbFrameRate = av_q2d(m_streamstate->video_st->r_frame_rate);
		//����˵��������
		if (m_dbFrameRate < 15 || m_dbFrameRate > 50)
		{
			//������MP3 ��ͼƬ��
			m_stream_type = 2;
			m_dbFrameRate = 25.0;
		}
		//���,��Ƶ��������,��Ƶyuv����,spspps_buf,spspps_size
		m_dwWidth = m_streamstate->pFormatCtx->streams[m_video_stream_idx]->codec->width;
		m_dwHeight = m_streamstate->pFormatCtx->streams[m_video_stream_idx]->codec->height;
		m_video_codecID = m_streamstate->pFormatCtx->streams[m_video_stream_idx]->codec->codec_id;
		m_video_pixelfromat = m_streamstate->pFormatCtx->streams[m_video_stream_idx]->codec->pix_fmt;
		m_spspps_size = m_streamstate->pFormatCtx->streams[m_video_stream_idx]->codec->extradata_size;

		if(m_spspps_size < 1000)
		{
			memcpy(m_spspps,m_streamstate->pFormatCtx->streams[m_video_stream_idx]->codec->extradata,m_spspps_size);
		}

		//�����һ���ж������ڶԲ�ͬ�ĸ�ʽ����pts��λ��ͳһ����ͳһ�� ���롱,����չ������ʽ
		if (strstr(m_streamstate->pFormatCtx->iformat->name,"mpegts")!= NULL)
		{
			double time_base = 90 * 1000;
			m_video_duration = time_base / m_dbFrameRate /100000;
		}
		else
		{
			m_video_duration = 1 / m_dbFrameRate;
		}
	}

	//�õ���Ƶ��Ϣ��CONTEXT
	if (Open_codec_context(&m_audio_stream_idx, m_streamstate->pFormatCtx, AVMEDIA_TYPE_AUDIO) >= 0) 
	{
		m_streamstate->audio_st = m_streamstate->pFormatCtx->streams[m_audio_stream_idx];
		m_audio_dec_ctx = m_streamstate->audio_st->codec;
		// Find the decoder for the video stream
		m_paudio_codec = avcodec_find_decoder(m_audio_dec_ctx->codec_id);
		if(m_paudio_codec == NULL) 
		{
			// Find audio decoderʧ��
			return;
		}

		// Open codec
		if(avcodec_open(m_audio_dec_ctx, m_paudio_codec) < 0)
		{
			// Could not open ayudio codec
			return;
		}

		//�����������������ʣ���Ƶ�������ͣ�һ֡���ݴ�С
		m_dwChannelCount = m_streamstate->pFormatCtx->streams[m_audio_stream_idx]->codec->channels;
		switch (m_streamstate->pFormatCtx->streams[m_audio_stream_idx]->codec->sample_fmt)
		{
		case AV_SAMPLE_FMT_U8:
			m_dwBitsPerSample  = 8;
			break;
		case AV_SAMPLE_FMT_S16:
			m_dwBitsPerSample  = 16;
			break;
		case AV_SAMPLE_FMT_S32:
			m_dwBitsPerSample  = 32;
			break;
		default:
			break;
		}
		m_dwFrequency = m_streamstate->pFormatCtx->streams[m_audio_stream_idx]->codec->sample_rate;
		m_audio_codecID = m_streamstate->pFormatCtx->streams[m_audio_stream_idx]->codec->codec_id;
		m_audio_frame_size = m_streamstate->pFormatCtx->streams[m_audio_stream_idx]->codec->frame_size;

		//�����һ���ж������ڶԲ�ͬ�ĸ�ʽ����pts��λ��ͳһ����ͳһ�� ���롱,����չ������ʽ
		if (strstr(m_streamstate->pFormatCtx->iformat->name,"mpegts")!= NULL)
		{
			double time_base = 90 * 1000;
			m_audio_duration = time_base * m_audio_frame_size / m_dwFrequency /100000;
		}
		else
		{
			double time_base = 1.0;
			m_audio_duration = time_base * m_audio_frame_size / m_dwFrequency;
		}
	}

	//�����̲߳�������
	m_streamstate->read_tid  = SDL_CreateThread(read_thread,(void*)this);
	if (!m_streamstate->read_tid)
	{
		//������ȡ�߳�ʧ��
		return;
	}
}


void CMovies::OnStopMovies()
{
	m_Isstop = 1;
	Sleep(100);

	//���ٱ�����
	UinitVariable();
}
