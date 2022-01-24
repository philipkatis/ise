IncludeDirectories=-Icode -Ithird_party/acutest/include

CompileFlags=-g -Wall -Wno-address-of-packed-member -Wno-write-strings -DISE_DEBUG=1 $(IncludeDirectories)

build_all: build_ise build_tests build_example | setup

run: build/example
	./build/example

#  This command runs the unit tests.
tests: build/tests
	./build/tests

setup:
	mkdir -p build

clean:
	rm -r -f build

ise_query_tree: code/ise_query_tree.cpp | setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_query_list: code/ise_query_list.cpp | setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise: code/ise.cpp | setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_tests: tests/tests.cpp | setup
	g++ $(CompileFlags) -c tests/tests.cpp -o build/$@.o

ise_example: example/example.cpp | setup
	g++ $(CompileFlags) -c example/example.cpp -o build/$@.o

build_ise: ise_query_tree ise_query_list ise | setup
	g++ $(CompileFlags) -shared build/ise_query_list.o build/ise_query_tree.o build/ise.o -o build/libcore.so

build_tests: ise_tests | setup
	g++ $(CompileFlags) build/ise_tests.o -Lbuild -lcore -o build/tests -Wl,-rpath=build

build_example: ise_example | setup
	g++ $(CompileFlags) build/ise_example.o -Lbuild -lcore -o build/example -Wl,-rpath=build
