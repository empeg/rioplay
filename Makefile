SUBDIRS = Main Audio Display Remote Web Utils Screens Fonts Playlists Log  \
          Wrapper Decoders Sources
export INCLUDEDIRS = -I../Main/ \
                     -I../Audio/ \
                     -I../Display/ \
                     -I../Remote/ \
                     -I../Web/ \
                     -I../Utils/ \
                     -I../Screens/ \
                     -I../Fonts/ \
                     -I../Playlists/ \
                     -I../Log/ \
                     -I../Decoders/ \
                     -I../Sources/

OBJS = Main/Main.o \
       Audio/Audio.o \
       Display/Display.o \
       Remote/Remote.o \
       Web/Web.o \
       Utils/Utils.o \
       Screens/Screens.o \
       Fonts/Fonts.o \
       Log/Log.o \
       Decoders/Decoders.o \
       Sources/Sources.o \
       Playlists/Playlist.o \
       /skiff/local/arm-linux/lib/libpthread.a
export CFLAGS = -O3 -Wall -D_REENTRANT
export CC = @gcc
export C++ = @g++
export LD = @ld

all: buildparts rioplay init

rioplay: $(OBJS)
	$(C++) -o rioplay $(CFLAGS) $(LIBS) $(OBJS)
	@echo -e "Successfully built rioplay.\n"
        

buildparts:
	@for dir in ${SUBDIRS}; \
	do \
		(cd $$dir; ${MAKE}); \
	done
	@echo -e "\n"

init: Wrapper/Wrapper.o
	$(CC) -o init $(CFLAGS) Wrapper/Wrapper.o
	@echo -e "Successfully built init.\n"

clean:
	find . -name *.o | xargs rm -f
	rm -f rioplay init
