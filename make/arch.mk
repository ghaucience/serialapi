#ARCH								:= mips
ARCH								:= MT7620
#ARCH			:= LS1020

ifeq ($(ARCH),mips)
CROSSTOOLDIR				:=/opt/au/qsdk-dusun/qsdk
export	STAGING_DIR	:= $(CROSSTOOLDIR)/staging_dir
export	PATH				:=$(PATH):$(STAGING_DIR)/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-1.0.14/bin
CROSS_CFLAGS				:= -I$(CROSSTOOLDIR)/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-1.0.14/usr/include
CROSS_LDFLAGS				:= -L$(CROSSTOOLDIR)/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-1.0.14/usr/lib
CROSS								:= mips-openwrt-linux-
endif

ifeq ($(ARCH),MT7620)
#CROSSTOOLDIR 				:= /home/au/all/gwork/openwrt
#CROSS   						:= mipsel-openwrt-linux-
#export  STAGING_DIR	:= $(CROSSTOOLDIR)/staging_dir
#export  PATH				:= $(PATH):$(STAGING_DIR)/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin

#CROSS_CFLAGS				:= -I$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/usr/include
#CROSS_CFLAGS				+= -I$(CROSSTOOLDIR)/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include
#CROSS_LDFLAGS			:= -L$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/usr/lib
#CROSS_LDFLAGS			+= -L$(CROSSTOOLDIR)/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib/ 

CROSSTOOLDIR                           := /home/au/all/gwork/tmp/tools/openwrt-sdk-ramips-mt76x8_gcc-7.3.0_musl.Linux-x86_64
CROSS                                                  := mipsel-openwrt-linux-
export  STAGING_DIR := $(CROSSTOOLDIR)/staging_dir
export  PATH                           :=$(PATH):$(STAGING_DIR)/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin
CROSS_CFLAGS                           := -I$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/usr/include
CROSS_CFLAGS                           += -I$(CROSSTOOLDIR)/staging_dir/target-mipsel_24kc_musl/usr/include
CROSS_LDFLAGS                  := -L$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/usr/lib
CROSS_LDFLAGS                  += -L$(CROSSTOOLDIR)/staging_dir/target-mipsel_24kc_musl/usr/lib

endif

ifeq ($(ARCH),LS1020)
CROSS   := arm-openwrt-linux-
CROSSTOOLDIR 				:=/home/au/openwrt
export  STAGING_DIR := $(CROSSTOOLDIR)/staging_dir

#toolchain-arm_cortex-a7+neon_gcc-4.8-linaro_uClibc-0.9.33.2_eabi/bin/
#target-arm_cortex-a7+neon_uClibc-0.9.33.2_eabi

export  PATH         := $(PATH):$(STAGING_DIR)/toolchain-arm_cortex-a7+neon_gcc-4.8-linaro_uClibc-0.9.33.2_eabi/bin
CROSS_CFLAGS         := -I$(CROSSTOOLDIR)/staging_dir/toolchain-arm_cortex-a7+neon_gcc-4.8-linaro_uClibc-0.9.33.2_eabi/usr/include
CROSS_CFLAGS         += -I$(CROSSTOOLDIR)/staging_dir/target-arm_cortex-a7+neon_uClibc-0.9.33.2_eabi/usr/include
CROSS_LDFLAGS       := -L$(CROSSTOOLDIR)/staging_dir/toolchain-arm_cortex-a7+neon_gcc-4.8-linaro_uClibc-0.9.33.2_eabi/usr/lib
CROSS_LDFLAGS       += -L$(CROSSTOOLDIR)/staging_dir/target-arm_cortex-a7+neon_uClibc-0.9.33.2_eabi/usr/lib


endif



GCC 		:= $(CROSS)gcc
CXX			:= $(CROSS)g++
AR			:= $(CROSS)ar
AS			:= $(CROSS)gcc
RANLIB	:= $(CROSS)ranlib
STRIP 	:= $(CROSS)strip
OBJCOPY	:= $(CROSS)objcopy
OBJDUMP := $(CROSS)objdump
SIZE		:= $(CROSS)size
LD			:= $(CROSS)ld
MKDIR		:= mkdir -p

#CFLAGS							:= -Wall -g -O2 -I$(ROOTDIR)/libs/osi/include  -I$(ROOTDIR)/libs/other/include  -DLIBDEBUG
CFLAGS							:= -Wall -g -O2 -I$(ROOTDIR)/libs/osi/include  -I$(ROOTDIR)/libs/other/include
CFLAGS							+= -I$(ROOTDIR)/libs/third/json/include -DHAVE_CONFIG_H -DHAVE_STDINT_H
CFLAGS							+= -I$(ROOTDIR)/libs/third/libhttpd/include
CFLAGS							+= -I$(ROOTDIR)/zwsrc -I$(ROOTDIR)/src
CFLAGS							+= -I$(ROOTDIR)/zwsrc/include
CFLAGS							+= -fPIC
CFLAGS							+= -fpermissive
CXXFLAGS						:= -std=c++0x 
CXXFLAGS						+= $(CFLAGS)
TARGET_CFLAGS				+= $(CROSS_CFLAGS) 


#LDFLAGS							:= -L$(ROOTDIR)/lib -lm -lrt -ldl -lpthread -ljson-c -lubox -lblobmsg_json -lubus -lcares -lmosquitto -lssl -lcrypto
LDFLAGS							:= -L$(ROOTDIR)/lib -lm -lrt -ldl -lpthread -ljson-c -lxml2 -lz -lubox -lblobmsg_json -lubus -lcares  -llua
#LDFLAGS							:= -L$(ROOTDIR)/lib -lm -lrt -ldl
LDFLAGS							+= -lstdc++
TARGET_LDFLAGS			+= $(CROSS_LDFLAGS)
