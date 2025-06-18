.PHONY: build test_all

build:
	./build.sh

test_all:
	cd ./build/test && ./tests
