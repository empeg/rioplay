SUBDIRS = Main Audio Display Remote Web Utils Screens Fonts Playlists Log Wrapper
export INCLUDEDIRS = -I../Main/ \
                     -I../Audio/ \
                     -I../Display/ \
                     -I../Remote/ \
                     -I../Web/ \
                     -I../Utils/ \
                     -I../Screens/ \
                     -I../Fonts/ \
                     -I../Playlists/ \
                     -I../Log/

OBJS = Main/Main.o \
       Audio/Audio.o \
       Display/Display.o \
       Remote/Remote.o \
       Web/Web.o \
       Utils/Utils.o \
       Screens/Screens.o \
       Fonts/Fonts.o \
       Playlists/Playlists.o \
       Log/Log.o \
       /skiff/local/arm-linux/lib/libpthread.a
export CFLAGS = -O3 -Wall
CC = gcc
export C++ = g++
#LIBS = -lpthread

all: buildparts rioplay init

rioplay: $(OBJS)
	$(C++) -o rioplay $(CFLAGS) $(LIBS) $(OBJS)

buildparts:
	@for dir in ${SUBDIRS}; \
	do \
		(cd $$dir; ${MAKE}); \
	done

init: Wrapper/Wrapper.o
	gcc -o init $(CFLAGS) Wrapper/Wrapper.o

clean:
	find . -name *.o | xargs rm -f
	rm -f rioplay init
