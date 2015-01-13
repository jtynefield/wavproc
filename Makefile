# debug
#CFLAGS= -std=c++11 -march=native -stdlib=libc++ -g # 7.8 GF/s

# opt
CFLAGS= -std=c++11 -march=native -stdlib=libc++ -Ofast -mllvm -force-vector-unroll=8 -Wno-deprecated-declarations -framework GLUT -framework OpenGL
DUMPFLAGS= -S $(CFLAGS) -mllvm --x86-asm-syntax=intel

TARGET=wavproc
SRCFILES=main.cpp
HEADERS =riff.h

all: $(TARGET)

$(TARGET): $(SRCFILES) $(HEADERS) Makefile
	clang++ $(CFLAGS) $< -o $@
	clang++ $(DUMPFLAGS) $< 

clean:
	rm $(TARGET)

