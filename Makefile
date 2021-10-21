# Makefile allows for easier execution of cmake - simply run 'make all' followed by 'make run' from the same directory of this Makefile

TARGET=zipline-takehome

all:
	mkdir build && cd build && cmake ../ && make all

update:
	cd build && make all

clean:
	rm -rf build

install:
	cd build && sudo make install

run:
	cd build && ./${TARGET}
