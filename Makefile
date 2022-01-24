IncludeDirectories=-Icode -Ithird_party/acutest/include

CompileFlags=-g -Wall -Wno-address-of-packed-member -Wno-write-strings -DISE_DEBUG=1 $(IncludeDirectories)

build_all: build_ise build_tests build_example

build_ise: $(wildcard code/*.cpp) $(wildcard code/*.h) | setup
	g++ $(CompileFlags) -shared code/ise.cpp -o build/libcore.so

build_tests: $(wildcard tests/*.cpp) $(wildcard tests/*.h) | setup
	g++ $(CompileFlags) tests/tests.cpp -Lbuild -lcore -o build/tests -Wl,-rpath=build

build_example: $(wildcard example/*.cpp) $(wildcard example/*.h) | setup
	g++ $(CompileFlags) example/example.cpp -Lbuild -lcore -o build/example -Wl,-rpath=build

run: build/example
	./build/example

tests: build/tests
	./build/tests

setup:
	mkdir -p build

clean:
	rm -r -f build
