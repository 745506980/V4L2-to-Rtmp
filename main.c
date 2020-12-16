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




#include <linux/videodev2.h>
#include <stdio.h>
#include <sys/select.h>
#include "include/camer.h"
#include "include/rtmp_send.h"
#include "include/x264.h"
#include "include/x264_encoder.h"

#define DEVICE_NAME "/dev/video0"

#define FILE_NAME "./out.h264"

RTMP *rtmp = NULL;
RTMPPacket *packet_sp = NULL;

#define URL "rtmp://127.0.0.1/live/yuan"
int operation()
{
	/*	1、图像格式　　如YUYV  YUV420 
	 *	2、宽高　　640x480
	 *	3、帧率	
	 *	4、
	 *	
	 *
	 * */		
	int fps = 30;
	int bitrate = 800;

	uint32_t pixformat = V4L2_PIX_FMT_YUV420;
	int ret = 0;
	Encode en ;
	sps_pps sp;
	unsigned int timer_ = 1000/fps;  //ms
	//初始化和连接RMTP 
	
	ret = Rtmp_Begin(URL);	
	if (ret == -1){
		fprintf(stderr,"RTMP_Begin is error!\n");
		exit(EXIT_FAILURE);
	}	
	//编码器初始化
	ret = Encode_init(&en, &sp, pixformat, WIDTH, HEIGHT, fps, bitrate, 0);
	if (ret < 0){
		fprintf(stderr,"Encode_init is error\n");
		exit(EXIT_FAILURE);
	}
	packet_sp = Create_sps_packet(&sp);

	//0x00 00 00 01 + sps 0x00 00 00 01 + pps
	//封装H.264文件的sps和pps
	sps_pps_buf buf;
	buf.buf = malloc(sp.sps_len + sp.pps_len + 8);
	int j = 0;
	buf.buf[j++] = 0x00;
	buf.buf[j++] = 0x00;
	buf.buf[j++] = 0x00;
	buf.buf[j++] = 0x01;
	memcpy(&buf.buf[j], sp.sps, sp.sps_len);
	j += sp.sps_len;
	buf.buf[j++] = 0x00;
	buf.buf[j++] = 0x00;
	buf.buf[j++] = 0x00;
	buf.buf[j++] = 0x01;
	memcpy(&buf.buf[j],sp.pps, sp.pps_len);	
	j += sp.pps_len;
	buf.length = j;

	ret = open_device(DEVICE_NAME);
	if (ret == -1) 
		exit(EXIT_FAILURE);
	open_file(FILE_NAME);
	init_device(pixformat);
	init_mmap();
	start_stream();
	uint32_t timer = 0;
	while(1)
	{
		timer = timer + timer_;
		ret = process_frame(&en, &buf, pixformat, timer);	
		if (ret == -1) break;
	}
	end_stream();
	close_mmap();
	close_device();
	Encode_end(en.handle, &en.picture, sp.sps, sp.pps);
	RTMP_END(rtmp);
	free(buf.buf);
	free(packet_sp);
	return 0;
}




int main(int argc, char *argv[])
{
	if (argc > 1){
		fprintf(stderr,"Invalid parameter!\n");
		exit(EXIT_FAILURE);
	}

	operation();	
	return 0;
}
