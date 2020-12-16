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

#include"./include/camer.h"
#include <linux/i2c.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "include/rtmp_send.h"
#include "include/x264_encoder.h"
#include <sys/select.h>
#include<time.h>


int fd;
int file_fd;
int frame_size;
static Video_Buffer * buffer = NULL;
int ioctl_(int fd, int request, void *arg)
{
	int ret = 0;
	do{
		ret = ioctl(fd, request, arg);
	}while(ret == -1 && ret == EINTR);
	return ret;	
}

int open_device(const char * device_name)
{
	struct stat st;
    if( -1 == stat( device_name, &st ) )
    {
        printf( "Cannot identify '%s'\n" , device_name );
        return -1;
    }

    if ( !S_ISCHR( st.st_mode ) )
    {
        printf( "%s is no device\n" , device_name );
        return -1;
    }

    fd = open(device_name, O_RDWR | O_NONBLOCK , 0);
    if ( -1 == fd )
    {
        printf( "Cannot open '%s'\n" , device_name );
        return -1;
    }
    return 0;	
}



int init_device(uint32_t pixformat)
{
	//查询设备信息
	struct v4l2_capability cap;
	
	if (ioctl_(fd, VIDIOC_QUERYCAP, &cap) == -1)
	{
		perror("VIDIOC_QUERYCAP");
		return -1;
	}
	printf("---------------------LINE:%d\n", __LINE__);
	printf("DriverName:%s\nCard Name:%s\nBus info:%s\nDriverVersion:%u.%u.%u\n",
		cap.driver,cap.card,cap.bus_info,(cap.version>>16)&0xFF,(cap.version>>8)&0xFF,(cap.version)&0xFF);	



	//选择视频输入
	struct v4l2_input input;
	CLEAN(input);
	input.index = 0;
	if ( ioctl_(fd, VIDIOC_S_INPUT,&input) == -1){
		printf("VIDIOC_S_INPUT IS ERROR! LINE:%d\n",__LINE__);
		return -1;
	}


	/*查看摄像头支持的视频格式*/
	struct v4l2_fmtdesc fmtdesc;
//  struct v4l2_frmsizeenum frmsize;
	fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("fm:\n");
    while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1){  //列举出所有支持的格式
        printf("%d.%s   %c%c%c%c\n", fmtdesc.index + 1, fmtdesc.description, 
            fmtdesc.pixelformat &  0xFF,
            (fmtdesc.pixelformat >> 8) & 0xFF,
            (fmtdesc.pixelformat >> 16) & 0xFF,
            (fmtdesc.pixelformat >> 24) & 0xFF);
#if 0
		frmsize.pixel_format = fmtdesc.pixelformat;
		frmsize.index = 0;

		while(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) != -1){			
			printf("%dx%d\n",frmsize.discrete.width, frmsize.discrete.height);
			frmsize.index++;
		}				
#endif		
        fmtdesc.index++;
	}   
    /*查看摄像头支持的分辨率*/
	
	//设置帧格式
	struct v4l2_format fmt;
	CLEAN(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = WIDTH;
	fmt.fmt.pix.height = HEIGHT;
	//视频格式
	
	fmt.fmt.pix.pixelformat = pixformat;

//	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YVU420;
//	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if (ioctl_(fd, VIDIOC_S_FMT, &fmt) == -1)
	{
		printf("VIDIOC_S_FMT IS ERROR! LINE:%d\n",__LINE__);
		return -1;
	}
	fmt.type = V4L2_BUF_TYPE_PRIVATE;
	if (ioctl_(fd, VIDIOC_S_FMT, &fmt) == -1){
		printf("VIDIOC_S_FMT IS ERROR! LINE:%d\n", __LINE__);
		return -1;
	}
	
	//查看帧格式
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if ( ioctl_(fd, VIDIOC_G_FMT, &fmt) == -1){
		printf("VIDIOC_G_FMT IS ERROR! LINE:%d\n", __LINE__);
		return -1;
	}
	printf("width:%d\nheight:%d\npixelformat:%c%c%c%c field:%d\n",
            fmt.fmt.pix.width, fmt.fmt.pix.height,
            fmt.fmt.pix.pixelformat &  0xFF,
            (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
            (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
            (fmt.fmt.pix.pixelformat >> 24) & 0xFF,
			fmt.fmt.pix.field

			);
#if 0	
	/*设置流相关　　帧率*/
	struct v4l2_streamparm parm;

	CLEAN(parm);
	
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	parm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME; //是否可以被timeperframe参数控制帧率
	parm.parm.capture.timeperframe.denominator = 30; //时间间隔分母
	parm.parm.capture.timeperframe.numerator = 1; //分子 

	if (ioctl_(fd, VIDIOC_S_PARM, &parm) == -1){
	//	printf("VIDIOC_S_PARM IS ERROR! \n");
		perror("VIDIOC_S_PARM");
		return -1;
	}

	if (ioctl_(fd, VIDIOC_G_PARM, (struct v4l2_streamparm*)&parm) == -1){		
		printf("VIDIOC_G_PARM IS ERROR! \n");
		return -1;
	} 
#endif

#if 1
/*YUYV*/
	__u32 min = fmt.fmt.pix.width * 2;
    if ( fmt.fmt.pix.bytesperline < min )
        fmt.fmt.pix.bytesperline = min;
/*YUV420*/
	min = ( unsigned int )WIDTH * HEIGHT * 3 / 2;
    if ( fmt.fmt.pix.sizeimage < min )
        fmt.fmt.pix.sizeimage = min;
    frame_size = fmt.fmt.pix.sizeimage;
	printf("After Buggy driver paranoia\n");
    printf("    >>fmt.fmt.pix.sizeimage = %d\n", fmt.fmt.pix.sizeimage);
    printf("    >>fmt.fmt.pix.bytesperline = %d\n", fmt.fmt.pix.bytesperline);
    printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");
    printf("\n");
#endif
	
	return 0;

}

 int init_mmap()
{
	//申请帧缓冲区
	struct v4l2_requestbuffers req;
	CLEAN(req);
	req.count = 4;
	req.memory = V4L2_MEMORY_MMAP;  //使用内存映射缓冲区
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//申请4个帧缓冲区，在内核空间中
	if ( ioctl_(fd, VIDIOC_REQBUFS, &req) == -1 ) 
	{
		printf("VIDIOC_REQBUFS IS ERROR! LINE:%d\n",__LINE__);
		return -1;
	}
	//获取每个帧信息，并映射到用户空间
	buffer = (Video_Buffer *)calloc(req.count, sizeof(Video_Buffer));
	if (buffer == NULL){
		printf("calloc is error! LINE:%d\n",__LINE__);
		return -1;
	}
	
	struct v4l2_buffer buf;
	int buf_index = 0;
	for (buf_index = 0; buf_index < req.count; buf_index ++)
	{
		CLEAN(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.index = buf_index;
		buf.memory = V4L2_MEMORY_MMAP;
		if (ioctl_(fd, VIDIOC_QUERYBUF, &buf) == -1) //获取每个帧缓冲区的信息 如length和offset
		{
			printf("VIDIOC_QUERYBUF IS ERROR! LINE:%d\n",__LINE__);
			return -1;
		}
		//将内核空间中的帧缓冲区映射到用户空间
		buffer[buf_index].length = buf.length;
		buffer[buf_index].start = mmap(NULL, //由内核分配映射的起始地址
									   buf.length,//长度
									   PROT_READ | PROT_WRITE, //可读写
									   MAP_SHARED,//可共享
									   fd,
									   buf.m.offset);
		if (buffer[buf_index].start == MAP_FAILED){
			printf("MAP_FAILED LINE:%d\n",__LINE__);
			return -1;
		}
		//将帧缓冲区放入视频输入队列
		if (ioctl_(fd, VIDIOC_QBUF, &buf) == -1)
		{
			printf("VIDIOC_QBUF IS ERROR! LINE:%d\n", __LINE__);
			return -1;
		}		
		printf("Frame buffer :%d   address :0x%x    length:%d\n",buf_index, (__u32)buffer[buf_index].start, buffer[buf_index].length);
	}	
	return 0;
}

void start_stream()
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl_(fd, VIDIOC_STREAMON, &type) == -1){
		fprintf(stderr, "VIDIOC_STREAMON IS ERROR! LINE:%d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
}
void end_stream()
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl_(fd, VIDIOC_STREAMOFF, &type) == -1){
		fprintf(stderr, "VIDIOC_STREAMOFF IS ERROR! LINE:%d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
}

int read_frame(Encode *en, sps_pps_buf *buf_sp, uint32_t pixformat, uint32_t timer)
{
	struct v4l2_buffer buf;
	int ret = 0;
	CLEAN(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (ioctl_(fd, VIDIOC_DQBUF, &buf) == -1){
		printf("VIDIOC_DQBUF! LINEL:%d\n", __LINE__);
		return -1;
	}
//	time_t start,end;
//	start = time(NULL);
#if 0	

	ret = write(file_fd, buffer[0].start ,frame_size);
//	printf("write:%d\n",frame_size);
	
#endif 
#if 1
	ret = Encode_frame(en, pixformat, file_fd, buf_sp, buffer[0].start, WIDTH, HEIGHT, timer);	
	if (ret == -1){
		fprintf(stderr,"Encode_frame\n");
		return -1;
	}
#endif 	
//	end = time(NULL);
//	printf("time:%0.f\n",difftime(end, start));
	if (ioctl_(fd, VIDIOC_QBUF, &buf) == -1){
		printf("VIDIOC_QBUF! LINE:%d\n", __LINE__);
		return -1;
	}
	return 0;
}


int open_file(const char * file_name)
{
	
	file_fd = open(file_name, O_RDWR | O_CREAT, 0777);
	if (file_fd == -1)
	{
		printf("open file is error! LINE:%d\n", __LINE__);
		return -1;
	}
	return 0;		
//	file = fopen(file_name, "wr+");
}

void close_mmap()
{
	int i = 0;
	for (i = 0; i < 4 ; i++)
	{
		munmap(buffer[i].start, buffer[i].length);
	}
	free(buffer);
}
void close_device()
{
	close(fd);
	close(file_fd);
}
int process_frame(Encode * en, sps_pps_buf * buf, uint32_t pixformat, uint32_t timer)
{
	struct timeval tvptr;
    int ret;
	tvptr.tv_usec = 0;  //等待50 us
    tvptr.tv_sec = 2;
    fd_set fdread;
    FD_ZERO(&fdread);
    FD_SET(fd, &fdread);
    ret = select(fd + 1, &fdread, NULL, NULL, &tvptr);
    if (ret == -1){
        perror("EXIT_FAILURE");
        exit(EXIT_FAILURE);
    }
	if (ret == 0){
		printf("timeout! \n");
	}
//	struct timeval start,end;
//	gettimeofday(&start, NULL);
	
	if(read_frame(en, buf, pixformat, timer) == -1)
	{
		fprintf(stderr, "readframe is error\n");
		return -1;
	}		

//	gettimeofday(&end, NULL);
//	printf("time:%ldms\n",(end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);
	
	return 0;
}
