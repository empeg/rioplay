SUBDIRS = Main Audio Display Remote Web Utils Screens Fonts Playlists
export INCLUDEDIRS = -I../Main/ \
                     -I../Audio/ \
                     -I../Display/ \
                     -I../Remote/ \
                     -I../Web/ \
                     -I../Utils/ \
                     -I../Screens/ \
                     -I../Fonts/ \
                     -I../Playlists/
OBJS = Main/Main.o \
       Audio/Audio.o \
       Display/Display.o \
       Remote/Remote.o \
       Web/Web.o \
       Utils/Utils.o \
       Screens/Screens.o \
       Fonts/Fonts.o \
       Playlists/Playlists.o
export CFLAGS = -O3 -Wall
CC = gcc
export C++ = g++
LIBS = -lpthread

all: buildparts rioplay

rioplay: $(OBJS)
	arm-linux-gcc -o rioplay $(CFLAGS) $(LIBS) $(OBJS)

buildparts:
	@for dir in ${SUBDIRS}; \
    do \
		(cd $$dir; ${MAKE}); \
	done

clean:
	find . -name *.o | xargs rm -f
	rm -f rioplay
