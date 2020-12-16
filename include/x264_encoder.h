
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
#ifndef __X264_ENCODER_H
#define __X264_ENCODER_H
#include<stdint.h>
#include"x264.h"
#include<linux/videodev2.h>
#include<stdlib.h>
#include<string.h>
typedef struct encode{
	x264_param_t param; //相关配置信息
	x264_nal_t *nal;
	x264_picture_t picture;	
	x264_t *handle;
}Encode;  
/*SPS PPS*/
typedef struct sps_pps{
	uint8_t * sps;
	uint8_t * pps;
	uint32_t sps_len;
	uint32_t pps_len;
}sps_pps;
/*x264编码*/

typedef struct sps_pps_buf{
    char *buf;
    unsigned int length;
}sps_pps_buf;



/*编码一帧数据*/
int Encode_frame(Encode *en, uint32_t pixformat,int fd ,sps_pps_buf * buf, uint8_t *frame, uint32_t width, uint32_t height, uint32_t timer);

int Encode_init(Encode *en, sps_pps *sp, uint32_t pixformat, uint32_t width, uint32_t height, uint32_t fps, uint32_t bitrate, int ConstantBitRate);

void Encode_end(x264_t *handle, x264_picture_t *picture, uint8_t * sps, uint8_t *pps);
 
int sps_pps_packet(sps_pps * sp);



#endif
/*防止头文件重复定义*/
