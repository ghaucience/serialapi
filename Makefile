ROOTDIR=$(shell pwd)
WORKDIR=$(ROOTDIR)/build

LIBVER			:=4.3

targets			+= libZWave_$(LIBVER).a
targets			+= libZWaveEx_$(LIBVER).a
targets			+= zwtest
targets			+= zwdevd

.PHONY: targets

all : $(targets)


srcs				+= $(ROOTDIR)/libs/osi/src/cond.c
srcs				+= $(ROOTDIR)/libs/osi/src/mutex.c
srcs				+= $(ROOTDIR)/libs/osi/src/list.c
srcs				+= $(ROOTDIR)/libs/osi/src/lockqueue.c
#srcs				+= $(ROOTDIR)/libs/osi/src/serial.c
#srcs				+= $(ROOTDIR)/libs/osi/src/tcp.c

srcs				+= $(ROOTDIR)/libs/osi/src/nameval.c
srcs				+= $(ROOTDIR)/libs/osi/src/log.c
#srcs				+= $(ROOTDIR)/libs/osi/src/crc.c
srcs				+= $(ROOTDIR)/libs/osi/src/time_utils.c
srcs				+= $(ROOTDIR)/libs/osi/src/timer.c
srcs				+= $(ROOTDIR)/libs/osi/src/hex.c
srcs				+= $(ROOTDIR)/libs/osi/src/parse_argv.c
srcs				+= $(ROOTDIR)/libs/osi/src/file_io.c
srcs				+= $(ROOTDIR)/libs/osi/src/file_event.c
#srcs				+= $(ROOTDIR)/libs/osi/src/filesystem_monitor.c
#srcs				+= $(ROOTDIR)/libs/osi/src/hashmap.c
#srcs				+= $(ROOTDIR)/libs/osi/src/base64.c
#srcs				+= $(ROOTDIR)/libs/osi/src/md5.c
#srcs				+= $(ROOTDIR)/libs/osi/src/des.cpp
#srcs				+= $(ROOTDIR)/libs/osi/src/util.c


srcs				+= $(ROOTDIR)/libs/third/json/src/dump.c
srcs				+= $(ROOTDIR)/libs/third/json/src/error.c
srcs				+= $(ROOTDIR)/libs/third/json/src/hashtable.c
srcs				+= $(ROOTDIR)/libs/third/json/src/hashtable_seed.c
srcs				+= $(ROOTDIR)/libs/third/json/src/json_parser.c
srcs				+= $(ROOTDIR)/libs/third/json/src/load.c
srcs				+= $(ROOTDIR)/libs/third/json/src/memory.c
srcs				+= $(ROOTDIR)/libs/third/json/src/pack_unpack.c
srcs				+= $(ROOTDIR)/libs/third/json/src/strbuffer.c
srcs				+= $(ROOTDIR)/libs/third/json/src/strconv.c
srcs				+= $(ROOTDIR)/libs/third/json/src/utf.c
srcs				+= $(ROOTDIR)/libs/third/json/src/value.c


srcs				+= $(ROOTDIR)/libs/third/libhttpd/src/api.c
srcs				+= $(ROOTDIR)/libs/third/libhttpd/src/ip_acl.c
srcs				+= $(ROOTDIR)/libs/third/libhttpd/src/version.c
srcs				+= $(ROOTDIR)/libs/third/libhttpd/src/ember.c
srcs				+= $(ROOTDIR)/libs/third/libhttpd/src/protocol.c

srcs	      := $(subst .cpp,.c,$(srcs))


testsrcs		+= $(ROOTDIR)/main.c
testsrcs		+= $(srcs)

testsrcs		+= $(ROOTDIR)/src/cmd.c  
testsrcs		+= $(ROOTDIR)/src/schedule.c  
testsrcs		+= $(ROOTDIR)/src/system.c  
testsrcs		+= $(ROOTDIR)/src/amber_mtksdk.c
##testsrcs		+= $(ROOTDIR)/src/uproto.c  
##testsrcs		+= $(ROOTDIR)/src/mqtt.c  
#testsrcs		+= $(ROOTDIR)/src/ziru.c

testsrcs    := $(subst .cpp,.c,$(testsrcs))


zwlibsrcs	:= $(ROOTDIR)/zwsrc/conhandle.c
zwlibsrcs	+= $(ROOTDIR)/zwsrc/linux.c
zwlibsrcs	+= $(ROOTDIR)/zwsrc/Serialapi.c
zwlibsrcs	+= $(ROOTDIR)/zwsrc/ctimer.c
zwlibsrcs	+= $(ROOTDIR)/zwsrc/zgw_nodemask.c
zwlibsrcs	+= $(ROOTDIR)/zwsrc/adapter.c
zwlibsrcs := $(subst .cpp,.c,$(zwlibsrcs))

zwtestsrcs += $(ROOTDIR)/zwtest.c
zwtestsrcs += $(ROOTDIR)/zwsrc/ctimer_imp.c
zwtestsrcs := $(subst .cpp,.c,$(zwtestsrcs))

zwdevlibsrcs	+= $(srcs)
zwdevlibsrcs	+= $(ROOTDIR)/src/zwave.c
zwdevlibsrcs	+= $(ROOTDIR)/src/ctimer_imp.c
zwdevlibsrcs	+= $(ROOTDIR)/src/zwave_device.c
zwdevlibsrcs	+= $(ROOTDIR)/src/zwave_device_storage.c
zwdevlibsrcs	+= $(ROOTDIR)/src/zwave_ccdb.c
zwdevlibsrcs	+= $(ROOTDIR)/src/zwave_class_cmd.c
zwdevlibsrcs	+= $(ROOTDIR)/src/web.c
zwdevlibsrcs	+= $(ROOTDIR)/src/system.c  
zwdevlibsrcs	+= $(ROOTDIR)/src/cmd.c  
zwdevlibsrcs	+= $(ROOTDIR)/src/schedule.c  
zwdevlibsrcs	+= $(ROOTDIR)/src/amber_mtksdk.c
zwdevlibsrcs  += $(ROOTDIR)/main_run.c
zwdevlibsrcs	+= $(ROOTDIR)/src/uproto.c

zwdevsrcs += $(ROOTDIR)/main.c
zwdevsrcs	+= $(ROOTDIR)/src/uproto_dusun.c
zwdevsrcs := $(subst .cpp,.c,$(zwdevsrcs))

objs = $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(srcs)))

testobjs = $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(testsrcs)))

zwlibobjs := $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(zwlibsrcs)))
zwdevlibobjs := $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(zwdevlibsrcs)))
zwdevlibobjs += $(zwlibobjs)

zwtestobjs := $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(zwtestsrcs)))
zwtestobjs += $(ROOTDIR)/build/libZWave_$(LIBVER).a

zwdevobjs := $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(zwdevsrcs)))
zwdevobjs += $(ROOTDIR)/build/libZWaveEx_$(LIBVER).a




-include $(ROOTDIR)/make/arch.mk
-include $(ROOTDIR)/make/rules.mk


$(warning ----------$(zwdevlibobjs) ------------- )
#$(eval $(call LinkLio,libdbsync.so$(VERSION),$(dbsyncobjs)))
#$(eval $(call LinkApp,test,$(testobjs)))

$(eval $(call LinkApp,zwtest,$(zwtestobjs)))
$(eval $(call LinkLia,libZWave_$(LIBVER).a,$(zwlibobjs)))

$(eval $(call LinkLia,libZWaveEx_$(LIBVER).a,$(zwdevlibobjs)))
$(eval $(call LinkApp,zwdevd,$(zwdevobjs)))

run :
	sudo ./build/test

scp :
	scp -P2200 ./build/zwtest root@192.168.0.230:/root
	#scp -P2201 ./build/zwdevd ./build/zwtest root@192.168.0.230:/usr/bin
	#scp -P2204 ./build/zwdevd ./build/zwtest root@192.168.0.230:/tmp/

release: libZWave_$(LIBVER).a libZWaveEx_$(LIBVER).a
	#mkdir -p $(ROOTDIR)/release/$(LIBVER)/include
	#mkdir -p $(ROOTDIR)/release/$(LIBVER)/lib
	#cp $(ROOTDIR)/zwsrc/Serialapi.h $(ROOTDIR)/release/$(LIBVER)/include
	#cp $(ROOTDIR)/zwsrc/zgw_nodemask.h $(ROOTDIR)/release/$(LIBVER)/include
	#cp $(ROOTDIR)/zwsrc/ctimer.h $(ROOTDIR)/release/$(LIBVER)/include
	#cp $(ROOTDIR)/zwsrc/ctimer_imp.h $(ROOTDIR)/release/$(LIBVER)/include
	#cp $(ROOTDIR)/zwsrc/include $(ROOTDIR)/release/$(LIBVER)/include/zwave -rf
	#cp $(ROOTDIR)/build/libZWave_$(LIBVER).a $(ROOTDIR)/release/$(LIBVER)/lib
	#cp $(ROOTDIR)/zwtest.c $(ROOTDIR)/release/$(LIBVER)/
	#cp $(ROOTDIR)/zwsrc/ctimer_imp.c $(ROOTDIR)/release/$(LIBVER)/
	#cp $(ROOTDIR)/Makefile.test $(ROOTDIR)/release/$(LIBVER)/Makefile

	mkdir -p $(ROOTDIR)/release/$(LIBVER)_Ex/include
	mkdir -p $(ROOTDIR)/release/$(LIBVER)_Ex/lib
	cp $(ROOTDIR)/zwsrc/Serialapi.h $(ROOTDIR)/release/$(LIBVER)_Ex/include
	cp $(ROOTDIR)/zwsrc/zgw_nodemask.h $(ROOTDIR)/release/$(LIBVER)_Ex/include
	cp $(ROOTDIR)/zwsrc/ctimer.h $(ROOTDIR)/release/$(LIBVER)_Ex/include
	cp $(ROOTDIR)/zwsrc/ctimer_imp.h $(ROOTDIR)/release/$(LIBVER)_Ex/include
	cp $(ROOTDIR)/libs/osi/include/*.h $(ROOTDIR)/release/$(LIBVER)_Ex/include
	cp $(ROOTDIR)/libs/third/json/include/*.h $(ROOTDIR)/release/$(LIBVER)_Ex/include
	cp $(ROOTDIR)/zwsrc/ctimer.h $(ROOTDIR)/release/$(LIBVER)_Ex/include
	cp $(ROOTDIR)/zwsrc/ctimer_imp.h $(ROOTDIR)/release/$(LIBVER)_Ex/include
	cp $(ROOTDIR)/src/*.h $(ROOTDIR)/release/$(LIBVER)_Ex/include
	cp $(ROOTDIR)/zwsrc/include $(ROOTDIR)/release/$(LIBVER)_Ex/include/zwave -rf
	cp $(ROOTDIR)/build/libZWaveEx_$(LIBVER).a $(ROOTDIR)/release/$(LIBVER)_Ex/lib
	cp $(ROOTDIR)/main.c $(ROOTDIR)/release/$(LIBVER)_Ex/
	cp $(ROOTDIR)/src/uproto.c $(ROOTDIR)/release/$(LIBVER)_Ex/
	cp $(ROOTDIR)/src/uproto_dusun.c $(ROOTDIR)/release/$(LIBVER)_Ex/
	cp $(ROOTDIR)/Makefile.Ex $(ROOTDIR)/release/$(LIBVER)_Ex/Makefile
	cp $(ROOTDIR)/zwave_ex.h $(ROOTDIR)/release/$(LIBVER)_Ex/include/zwave.h
	cp $(ROOTDIR)/uproto_dusun_ex.c $(ROOTDIR)/release/$(LIBVER)_Ex/uproto_dusun.c




	

