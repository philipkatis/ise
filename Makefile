# Suppress the output of all commands.
.SILENT:

# The additional include directories we want to search.
IncludeDirectories=-Icode -Ithird_party/acutest/include

# The compile flags we want to use through all our g++ calls.
CompileFlags=-g -Wall -Wno-unused-function -Wno-address-of-packed-member -Wno-write-strings $(IncludeDirectories)

# The default target that builds all the executables.
all: build_lib build_tests build_example | setup

# This command runs the test application.
run: build/example
	./build/example

#  This command runs the unit tests.
tests: build/tests
	./build/tests

# This command runs valgrind and cheks for any memory errors.
valgrind: build/example
	valgrind ./build/example

# This target deletes the output directory.
.PHONY: clean
clean:
	rm -rf build

# This target sets up the output directory.
setup:
	mkdir -p build

ise_example: example/example.cpp | setup
	g++ $(CompileFlags) -c example/example.cpp -o build/$@.o

build_lib: code/ise.cpp | setup
	g++ $(CompileFlags) -shared -fPIC code/ise.cpp -o build/libcore.so

build_tests: tests/tests.cpp | setup
	g++ $(CompileFlags) tests/tests.cpp -o build/tests

# This target builds the example.
build_example: ise_example | setup
	g++ $(CompileFlags) build/ise_example.o -Lbuild -lcore -o build/example -Wl,-rpath=build
