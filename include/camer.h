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
#ifndef	__CAMER_H 
#define __CAMER_H
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<linux/videodev2.h>
#include<sys/ioctl.h>
#include<errno.h>                                                                                     
#include<string.h>
#include<assert.h>
#include<getopt.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<asm/types.h>
#include<linux/fb.h>
#include"rtmp_send.h"
#include"x264_encoder.h"

#define CLEAN(x) (memset(&(x), 0, sizeof(x)))

#define WIDTH 640

#define HEIGHT 480

typedef struct Video_Buffer{
	void * start;
	unsigned int length;
}Video_Buffer;


int ioctl_(int fd, int request, void *arg);

void sys_exit(const char *s);

int open_device(const char * device_name);

int open_file(const char * file_name);

void start_stream(void);

void end_stream(void);

int init_device(uint32_t pixformat);

int init_mmap(void);

int read_frame(Encode *en, sps_pps_buf * buf, uint32_t pixformat, uint32_t timer);

int process_frame(Encode *en, sps_pps_buf *buf, uint32_t pixformat, uint32_t timer);

void close_mmap(void);

void close_device(void);


#endif
/*防止头文件重复定义*/
