CommonCompilerFlags=-Wall -Wno-address-of-packed-member -Wno-write-strings -pthread -DISE_LINUX=1 -DISE_DEBUG=1 -g

build_all: build_ise build_tests build_example

build_ise: $(wildcard code/*.cpp) $(wildcard code/*.h) | setup
	g++ $(CommonCompilerFlags) -shared code/ise.cpp -o build/libcore.so

build_tests: $(wildcard tests/*.cpp) $(wildcard tests/*.h) | setup
	g++ $(CommonCompilerFlags) -Icode -Ithird_party/acutest/include tests/tests.cpp -Lbuild -lcore -o build/tests -Wl,-rpath=build

build_example: $(wildcard example/*.cpp) $(wildcard example/*.h) | setup
	g++ $(CommonCompilerFlags) -Icode example/example.cpp -Lbuild -lcore -o build/example -Wl,-rpath=build

run: build/example
	./build/example

tests: build/tests
	./build/tests

setup:
	mkdir -p build

clean:
	rm -r -f build
