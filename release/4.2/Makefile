ROOTDIR=$(shell pwd)

ARCH								:= MT7620

ifeq ($(ARCH),MT7620)
CROSSTOOLDIR 				:= /home/au/all/gwork/openwrt
CROSS   						:= mipsel-openwrt-linux-
export  STAGING_DIR	:= $(CROSSTOOLDIR)/staging_dir
export  PATH				:= $(PATH):$(STAGING_DIR)/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin

CROSS_CFLAGS				:= -I$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/usr/include
CROSS_CFLAGS				+= -I$(CROSSTOOLDIR)/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include
CROSS_LDFLAGS			:= -L$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/usr/lib
CROSS_LDFLAGS			+= -L$(CROSSTOOLDIR)/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib/ 
endif


CC=$(CROSS)gcc


srcs 		:= $(ROOTDIR)/zwtest.c
srcs 		+= $(ROOTDIR)/ctimer_imp.c

CFLAGS 	:= -I$(ROOTDIR)/include -I$(ROOTDIR)/include/zwave

LDFLAGS	:=  -fPIC

ALIBS		:= $(ROOTDIR)/lib/libZWave_4.2.a

$(warning $(ALIBS))

zwtest : $(srcs) $(ALIBS)
	@mkdir -p $(ROOTDIR)/build
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $(ROOTDIR)/build/$@

clean:
	rm -rf $(ROOTDIR)/build



