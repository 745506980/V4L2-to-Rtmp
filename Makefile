#
#
#    Copyright (C) 2020   JunKe Yuan<745506980@qq.com>
#
#    This  is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    This  is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#	 Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
#    USA
#    



CROSS_COMPILE = arm-linux-
CC=$(CROSS_COMPILE)gcc

TARGET = Send_h264

OBJS += rtmp_send.o
OBJS += x264_encoder.o
OBJS += camer.o
OBJS += main.o


INCLUDES = -I./include/

LIBS = -lpthread -lrt

LINK_OPTS =-L./lib -lx264 -lrtmp -lssl -lz -lcrypto

CFLAGS = -Wall -O2 -g 

LD_FLAGS = $(LIBS) $(INCLUDES) $(LINK_OPTS)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ $(LINK_OPTS)

%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm $(OBJS) $(TARGET)

