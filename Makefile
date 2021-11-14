# Suppress the output of all commands.
.SILENT:

# The additional include directories we want to search.
IncludeDirectories=-Icode -Ithird_party/acutest/include

# The compile flags we want to use through all our g++ calls.
CompileFlags=-g -Wno-write-strings -DISE_DEBUG=1 $(IncludeDirectories)

# The default target that builds all the executables.
all: build_tests | Setup

# This command runs the unit tests.
tests: build/tests
	./build/tests

# This target deletes the output directory.
.PHONY: clean
clean:
	rm -rf build

# This target sets up the output directory.
Setup:
	mkdir -p build

# These targets build all the translation units separately into object files.
ise_match: code/ise_match.cpp | Setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_keyword_list: code/ise_keyword_list.cpp | Setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_bk_tree: code/ise_bk_tree.cpp | Setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_interface: code/ise_interface.cpp | Setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_tests: tests/ise_tests.cpp | Setup
	g++ $(CompileFlags) -c tests/$@.cpp -o build/$@.o

# This target builds the unit tests application.
build_tests: ise_match ise_keyword_list ise_bk_tree ise_interface ise_tests | Setup
	g++ $(CompileFlags) build/ise_match.o build/ise_keyword_list.o build/ise_bk_tree.o build/ise_interface.o build/ise_tests.o -o build/tests
