.PHONY: build test_all

build:
	rm -rf build
	./build.sh

test_all:
	cd ./build/test && ./tests
