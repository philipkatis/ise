# Suppress the output of all commands.
.SILENT:

# The additional include directories we want to search.
IncludeDirectories=-Icode -Ithird_party/acutest/include

# The compile flags we want to use through all our g++ calls.
CompileFlags=-g -Wall -Wno-write-strings -DISE_DEBUG=0 $(IncludeDirectories)

# The default target that builds all the executables.
all: build_tests | setup

# This command runs the unit tests.
tests: build/tests
	./build/tests

# This command runs valgrind and cheks for any memory errors.
valgrind:
	valgrind ./build/tests

# This target deletes the output directory.
.PHONY: clean
clean:
	rm -rf build

# This target sets up the output directory.
setup:
	mkdir -p build

# These targets build all the translation units separately into object files.
ise_match: code/ise_match.cpp | setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_keyword_list: code/ise_keyword_list.cpp | setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_bk_tree: code/ise_bk_tree.cpp | setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_interface: code/ise_interface.cpp | setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_tests: tests/ise_tests.cpp | setup
	g++ $(CompileFlags) -c tests/$@.cpp -o build/$@.o

# This target builds the unit tests application.
build_tests: ise_match ise_keyword_list ise_bk_tree ise_interface ise_tests | setup
	g++ $(CompileFlags) build/ise_match.o build/ise_keyword_list.o build/ise_bk_tree.o build/ise_interface.o build/ise_tests.o -o build/tests
