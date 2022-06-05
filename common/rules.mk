
#NDK_DIR := "../../tools/android-ndk-r8c"

#ifneq ($(shell if [ -d "$(NDK_DIR)" ]; then echo true; fi), true)
#$(error DIR=$(NDK_DIR) does not exit)
#endif

#NDK_DIR := $(shell cd $(NDK_DIR) && pwd)

CROSS_COMPILE := #$(NDK_DIR)/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86/bin/arm-linux-androideabi-
CC:=$(CROSS_COMPILE)gcc
STRIP:=$(CROSS_COMPILE)strip

#CFLAGS:=--sysroot=$(NDK_DIR)/platforms/android-9/arch-arm -march=armv7-a -mfloat-abi=softfp -mfpu=neon -Wall
#LDFLAGS:=--sysroot=$(NDK_DIR)/platforms/android-9/arch-arm -march=armv7-a -mfloat-abi=softfp -mfpu=neon -Wall

INCLUDE := -I../common/external/include
LIB :=   -ljpeg -lfreetype -lpng -lz -lm # ../common/external/lib/libturbojpeg.a ../common/external/lib/libfreetype.a ../common/external/lib/libpng12.a -lz -lm

EXESRCS := ../common/graphic.c ../common/touch.c ../common/external.c ../common/task.c $(EXESRCS)
EXEOBJS := $(patsubst %.c, %.o, $(EXESRCS))

$(EXENAME): $(EXEOBJS)
	$(CC) $(LDFLAGS) -o $(EXENAME) $(EXEOBJS) $(LIB)
#	$(STRIP) $(EXENAME)

clean:
	rm -f $(EXENAME) $(EXEOBJS)

%.o: %.c ../common/common.h
	$(CC) -ggdb3 $(CFLAGS) $(INCLUDE) -c -o $@ $<

