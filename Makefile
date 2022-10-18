CXX = clang++
CFLAGS = -g -O0 -pedantic -std=c++2b -stdlib=libc++
LIBS=-lm -ldl -lpthread -Llib/ -l:libmemflow_ffi.a

ODIR=./

main.o: main.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

main.out: main.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: all
all: main.out

.DEFAULT_GOAL := all

clean:
	rm -f $(ODIR)/*.o
