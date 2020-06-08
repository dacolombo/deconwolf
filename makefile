# Normal build:
# make -B
# To build with debug flags and no OpenMP
# make DEBUG=1 OMP=0 -B
# For normal build
# make -B

CC_VERSION = "$(shell gcc --version | head -n 1)"
GIT_VERSION = "$(shell git log --pretty=format:'%aD:%H' -n 1)"

XFLAGS = -DCC_VERSION=\"$(CC_VERSION)\"
XFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"

CFLAGS = -Wall -std=gnu99 -march=native

DEBUG?=0
ifeq ($(DEBUG),1)
    CFLAGS = -g3 -DDEBUG 
else
    CFLAGS= -Wno-unknown-pragmas -DNDEBUG -O3 -flto -ftree-vectorize
endif

# on MacOS add -Xpreprocessor
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += 
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -Xpreprocessor
endif

OMP?=1
ifeq ($(OMP), 1)
  CFLAGS += -fopenmp
else
  CFLAGS += -fno-openmp
endif

CFLAGS += $(XFLAGS)

CC = cc $(CFLAGS)
SRCDIR = src/

dw = bin/dw
dw_OBJECTS = fim.o tiling.o fft.o fim_tiff.o dw.o deconwolf.o
dw_LIBRARIES = -lm -lfftw3f -lfftw3f_threads -ltiff

dwtm = bin/dw_tiffmax
dwtm_OBJECTS = fim_tiff.o deconwolf_tif_max.o
dwtm_LIBRARIES = -lm -ltiff


all: $(dw) $(dwtm)

$(dwtm): $(dwtm_OBJECTS)
	$(CC) -o $@ $^ $(dwtm_LIBRARIES)

$(dw): $(dw_OBJECTS)
	$(CC) -o $@ $^ $(dw_LIBRARIES)

%.o: $(SRCDIR)%.c
	$(CC) -c $<

clean:
	rm -f $(dw) $(dw_OBJECTS)
	rm -f $(dwtm) $(dwtm_OBJECTS)

install:
	# Binaries
	cp bin/dw /usr/bin/dw
	cp bin/dw_tiffmax /usr/bin/
	cp src/deconwolf_batch.py /usr/bin/dw_batch
	chmod +x /usr/bin/dw_batch
	# Man pages
	cp doc/deconwolf.1 .
	gzip deconwolf.1
	mv deconwolf.1.gz /usr/share/man/man1/dw.1.gz

uninstall:
	rm /usr/bin/dw
	rm /usr/bin/dw_tiffmax
	rm /usr/share/man/man1/dw.1.gz
	rm /usr/bin/dw_batch

