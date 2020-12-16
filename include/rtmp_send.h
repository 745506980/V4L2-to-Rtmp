

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
#ifndef __RTMP_SEND_H 
#define __RTMP_SEND_H

#include"librtmp/rtmp.h"
#include"librtmp/amf.h"
#include"x264_encoder.h"

extern RTMPPacket * packet_sp;
extern RTMP * rtmp;

int Rtmp_Begin(char * URL);


/*对H264码流进行封包
 * type = 1 为IDR
 * type = 0 非IDR
 * */
int Send_h264_packet(uint8_t * H264_Stream, int length, int type, uint32_t timer);



/*获取对sps和pps进行封包*/
RTMPPacket * Create_sps_packet(sps_pps *sp);


void RTMP_END();

 
#endif
/*防止头文件重复定义*/
