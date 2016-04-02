CC=emcc
SOURCES:= 	../kegs/Src/adb.cpp \
			../kegs/Src/async_event.cpp \
			../kegs/Src/clock.cpp \
			../kegs/Src/compression.cpp \
			../kegs/Src/config_generic.cpp \
			../kegs/Src/dis.cpp \
			../kegs/Src/driver.cpp \
			../kegs/Src/engine_c.cpp \
			../kegs/Src/GraphCounter.cpp \
			../kegs/Src/iwm.cpp \
			../kegs/Src/marinetti.cpp \
			../kegs/Src/moremem.cpp \
			../kegs/Src/paddles.cpp \
			../kegs/Src/SaveState.cpp \
			../kegs/Src/smartport.cpp \
			../kegs/Src/sound.cpp \
			../kegs/Src/sound_driver.cpp \
			../kegs/Src/scc.cpp \
			../kegs/Src/scc_socket_driver.cpp \
			../kegs/Src/video.cpp \
			../kegs/Src/sim65816.cpp \
			../kegs/Dummy/sounddriver_dummy.cpp \
			../kegs/Dummy/joystick_dummy.cpp \
			../Libraries/zlib123/adler32.cpp \
			../Libraries/zlib123/compress.cpp \
			../Libraries/zlib123/crc32.cpp \
			../Libraries/zlib123/deflate.cpp \
			../Libraries/zlib123/inflate.cpp \
			../Libraries/zlib123/inftrees.cpp \
			../Libraries/zlib123/trees.cpp \
			../Libraries/zlib123/uncompr.cpp \
			../Libraries/zlib123/zutil.cpp \
			../Libraries/zlib123/inffast.cpp \
			../kegs/Console/config_kegs.cpp \
			../kegs/AsmJS/asmjs_generic.cpp \
			../kegs/AsmJS/asmjs_console.cpp 


LDFLAGS=-O2 --llvm-opts 2
OUTPUT=activegs.js

all: $(SOURCES) $(OUTPUT)


$(OUTPUT): $(SOURCES) 
	$(CC) $(SOURCES) -s TOTAL_MEMORY=33554432 -DKEGS_LITTLE_ENDIAN=1 -DACTIVEGS_NOSAVESTATE=1 -DDO_VERBOSE=0 -DNO_MARINETTI_SUPPORT=1 -O2 -s ASM_JS=1 --embed-file config.kegs --preload-file ROM --preload-file Nucleus.2mg -o out/index.html

clean:
	rm $(OUTPUT)
	rm $(OUTPUT).map