CommonCompileFlags=-Wall -Wno-write-strings -Wno-unused-function -g -pthread -fPIC -Icode

build_all: build_library build_tests build_example

run: build/example
	./build/example

tests: build/tests
	./build/tests

clean:
	rm -rf build

setup:
	mkdir -p build

build_library: code/ise.cpp | setup
	g++ $(CommonCompileFlags) -shared code/ise.cpp -o build/libcore.so

build_tests: tests/tests.cpp | setup
	g++ $(CommonCompileFlags) -Ithird_party/acutest/include tests/tests.cpp -o build/tests

build_example: example/example.cpp | setup
	g++ $(CommonCompileFlags) example/example.cpp -Lbuild -lcore -o build/example -Wl,-rpath=build
