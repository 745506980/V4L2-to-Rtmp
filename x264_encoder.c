/*
*    Copyright (C) 2020   JunKe Yuan<745506980@qq.com>
*
*    This  is free software; you can redistribute it and/or
*    modify it under the terms of the GNU Lesser General Public
*    License as published by the Free Software Foundation; either
*    version 2.1 of the License, or (at your option) any later version.
*
*    This  is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    Lesser General Public License for more details.
*
*    You should have received a copy of the GNU Lesser General Public
*    License along with this library; if not, write to the Free Software
*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
*    USA
*    
*/



#include "include/x264_encoder.h"
#include <sys/types.h>
#include "include/rtmp_send.h"
#include "include/x264.h"
#include<stdio.h>
#include<unistd.h>
int 
Encode_init(struct encode *en, sps_pps *sp, uint32_t pixformat, uint32_t width, uint32_t height, uint32_t fps, uint32_t bitrate, int ConstantBitRate)
{
	//初始化配置
	x264_param_default(&en->param);
	// zerolatency 选项是为了降低在线转码编码的延迟 
	// 缓存帧的个数为0 rc_lookahead = 0
	// sync_lookhead 关闭线程预测 可减小延迟，但也会降低性能
	// B 帧为0  实时视频需要
	// sclied_threads = 1　基于分片的线程，默认off 开启该方法在压缩率和编码效率上都略低于默认方法，但是没有编码延迟。除非在编码实时流或者对地延迟要求较高的场合开启该方法
	x264_param_default_preset(&en->param, "ultrafast", "zerolatency");
	
	/*CPU FLAGS*/
	// 线程设置　自动获取线程大小
	en->param.i_sync_lookahead = X264_SYNC_LOOKAHEAD_AUTO;//自动获取线程超前缓冲区的大小  
	/*自动选择并行编码多帧*/
	en->param.i_threads = X264_SYNC_LOOKAHEAD_AUTO;		//取空缓冲区继续使用，不死锁的保证
	
//	en->param.i_threads = 1;

	/*VIDEO FLAGS*/
	en->param.i_width = width;
	en->param.i_height = height;
	en->param.i_frame_total = 0;			//如果已知要编码的帧数，否则为0
	en->param.i_keyint_min = 0;				//I帧的最小间隔
	en->param.i_keyint_max = (int)fps * 2;		//I帧的最大间隔　　也就是最大2s一个Ｉ帧
	en->param.b_annexb = 1;					//为1将开始码0x0000 0001放在NAL单元之前
	en->param.b_repeat_headers = 0;			// 关键帧前面是否放SPS和PPS 0 否 1 放
	//在RTMP协议中，SPS和PPS有单独的格式发送，所以这里不让关键帧前面放SPS和PPS
/*	if (pixformat == V4L2_PIX_FMT_YUYV)
		en->param.i_csp = X264_CSP_I422;		//CSP 图像输入格式   YUYV  YUV420
	
	if (pixformat == V4L2_PIX_FMT_YUV420)*/
		en->param.i_csp = X264_CSP_I420;
	/*I帧间隔*/
	en->param.i_fps_den = 1;					//帧率分母
	en->param.i_fps_num = fps;					//帧率分子
	en->param.i_timebase_num = (int)(fps * 1000 + .5);			
	en->param.i_timebase_den = 1000;	


	/*B帧设置*/
	en->param.i_bframe = 0;					//2幅参考图之间有多少个B帧 作为实时传输 B帧为0
	en->param.i_bframe_pyramid = 0;			// 保留一些B帧作为引用(参考) 0 off 不使用B帧作为参考帧  1 严格分层  2 正常的
	en->param.b_open_gop = 0;				//不使用open_gop 码流里面包含B帧的时候才会出现open_gop,一个GOP里面的某一帧在解码时要依赖于前一个GOP的某些帧，这个GOP就成为open-gop。有些解码器不能完全支持open-gop，所以默认是关闭的
	en->param.i_bframe_adaptive = X264_B_ADAPT_FAST;	//B帧的适应算法
	
	/*log参数*/  
//	en->param.i_log_level = X264_LOG_DEBUG;	//打印编码信息
	
	/*速率控制参数*/
	en->param.rc.i_bitrate = bitrate;		//设置比特率 单位时间发送的比特数量 kbps
	en->param.rc.i_lookahead = 0;
	/*是否为恒定码率*/
	if (!ConstantBitRate)
	{
		en->param.rc.i_rc_method = X264_RC_ABR;	//码率控制，CQP(恒定质量)， CRF(恒定码率)，ABR(平均码率)
		en->param.rc.i_vbv_max_bitrate = bitrate; //平均码率下，最大瞬时码率，默认0
		en->param.rc.i_vbv_buffer_size = bitrate;
	}
	else{
		en->param.rc.b_filler = 1;					//设置为CBR模式 
		en->param.rc.i_rc_method = X264_RC_CRF;		//恒定码率
		en->param.rc.i_vbv_buffer_size = bitrate;	//VBV Video Buffering Verifier 视频缓存检验器
		en->param.rc.i_vbv_max_bitrate = bitrate;  //平均码率模式下最大瞬时码率
	}
	/*根据参数初始化X264级别*/
	en->handle = x264_encoder_open(&en->param);	
	/*初始化图片信息*/
	x264_picture_init(&en->picture);
	/*按YUYV格式分配空间  最后要x264_picture_clean*/
	if (pixformat == V4L2_PIX_FMT_YUYV){
		x264_picture_alloc(&en->picture, X264_CSP_I422, width, height);	
	}
	if (pixformat == V4L2_PIX_FMT_YUV420){
		x264_picture_alloc(&en->picture, X264_CSP_I420, width, height);
	}
	en->picture.i_pts = 0;
	int pi_nal = 0;	
	/*en->nal 返回用于整个流的SPS、PPS 和SEI
	 *pi_nal 返回的是en->nal的单元数 3 SPS PPS SEI 
	 *en->nal->i_payload 是p_payload有效负载大小也就是p_payload的长度 包含起始码0x00000001
	 *en->nal->p_payload 是里面存放的是SPS PPS SEI的数据 包含起始码0x00000001
	 * */
	x264_encoder_headers(en->handle, &en->nal, &pi_nal);
	if ( pi_nal > 0 )
	{
		int i = 0;
		for(i = 0; i < pi_nal; i++)
		{
			if (en->nal[i].i_type == NAL_SPS) //SPS数据 0x67&1F
			{
				sp->sps = malloc(en->nal[i].i_payload - 4); //去掉起始码的四个字节
				sp->sps_len = en->nal[i].i_payload - 4;
				memcpy(sp->sps, en->nal[i].p_payload + 4, sp->sps_len);//跳过起始码				
			}
			/*PPS*/
			if (en->nal[i].i_type == NAL_PPS)
			{
				sp->pps = malloc(en->nal[i].i_payload - 4);
				sp->pps_len = en->nal[i].i_payload - 4;
				memcpy(sp->pps, en->nal[i].p_payload + 4, sp->pps_len);
			}
		}
		return 1;  //成功获取SPS 和 PPS
	}
	/*对获取SPS PPS*/
	return 0;
}



int 
Encode_frame(Encode *en, uint32_t pixformat, int fd, sps_pps_buf *buf, uint8_t *frame, uint32_t width, uint32_t height, uint32_t timer)
{
	
	/*H264编码并发送*/
	int num, i;
	int index_y, index_u, index_v;
	/*plane[0] 、plane[1] 、 plane[2] 分别存储Y、U、V分量*/
	
	uint8_t * y = en->picture.img.plane[0];
	uint8_t * u = en->picture.img.plane[1];
	uint8_t * v = en->picture.img.plane[2];	
	uint8_t * frame_ = frame;
	/*对YUYV图像YUV分量分离*/
	if (pixformat == V4L2_PIX_FMT_YUYV)
	{
		index_y = 0;
		index_u = 0;
		index_v = 0;
		num = (width * height)*2 - 4;
		/*YUYV*/
		for (i = 0; i < num; i = i + 4)
		{
			*(y + (index_y++)) = *(frame_ + i);
			*(u + (index_u++)) = *(frame_ + i + 1);
			*(y + (index_y++)) = *(frame_ + i + 2);
			*(v + (index_v++)) = *(frame_ + i + 3);
		}		
	}
	/*YUV420*/		
	if (pixformat == V4L2_PIX_FMT_YUV420){
		index_y = width * height;
		index_u =index_y >> 2;  //u v 分量长度一致
		index_v = index_y + index_u;
		/*摄像头采集的是YU12        Y = w*h   u = w*h >> 2
		 *307200
		 *384000
		 *
		 *
		 * 460800
		 * */
		memcpy(y, frame_, index_y);	//分离Y分量
		memcpy(u, frame_ + index_y, index_u);
		memcpy(v, frame_ + index_v, index_u);	
#if 0
		frame_ = frame_ + index_y;		
		
		num = (index_y >> 1) - 2;
		index_v = 0;
		index_u = 0;
		for (i = 0; i < num; i = i + 2)
		{
			*(u + index_v++) = *(frame_ + i);
			*(v + index_u++) = *(frame_ + i + 1);
		}
#endif
	}
	/*对图像进行编码*/	
	int pi_nal = 0;
	x264_picture_t out_picture;
	x264_picture_init(&out_picture);
	int type = 0;
	int ret = x264_encoder_encode(en->handle, &en->nal, &pi_nal, &en->picture, &out_picture);
	if (ret < 0){
		fprintf(stderr,"x264_encoder_encode is error\n");
		return -1;
	}
//	en->picture.i_pts++;
/* 获取编码后的数据*/		
	if (pi_nal > 0)
	{	
		for (i = 0; i < pi_nal; i++)
		{
			type = 0;
			if (en->nal[i].i_type == NAL_SLICE || en->nal[i].i_type == NAL_SLICE_IDR) 
			{
				if (en->nal[i].i_type == NAL_SLICE_IDR) //如果是关键帧 
				{
					type = 1; //标记为关键帧							
					//写入文件
#if 0
					//I帧前面需要有sps + pps信息
	                ret = write(fd, buf->buf, buf->length); //写入sps pps
					if (ret == -1){
			           fprintf(stderr, "write sps_pps_buf is error!\n");
				        return -1;
					 }
#endif 					
				}

				/*发送编码过后的数据*/
#if 1				
				int ret = Send_h264_packet(en->nal[i].p_payload + 4, en->nal[i].i_payload - 4, type, timer);
				if (ret < 0){
					fprintf(stderr,"Send_h264_packet is error\n");
					return -1;
				}
#if 0
			     //写入文件
                ret = write(fd, en->nal[i].p_payload, en->nal[i].i_payload);
                if (ret == -1)
                {
                    fprintf(stderr, "write frame is error !\n");
                    return -1;
                }
#endif 
#endif
			}						
		}
	}
	return 0;
}

void Encode_end(x264_t *handle, x264_picture_t * picture, uint8_t *sps, uint8_t * pps)
{
	/*释放内存*/
	free(sps);
	sps = NULL;
	free(pps);
	pps = NULL;

	/*清空picture*/
	x264_picture_clean(picture);

	/*关闭编码*/	
	x264_encoder_close(handle);
	
}


/*对sps和pps进行封包*/




