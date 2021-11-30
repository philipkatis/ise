# We want the executable to find libcore.so, so we output the location of the library.
export LD_LIBRARY_PATH=./build

# Suppress the output of all commands.
.SILENT:

# The additional include directories we want to search.
IncludeDirectories=-Icode -Ithird_party/acutest/include

# The compile flags we want to use through all our g++ calls.
CompileFlags=-g -Wall -Wno-write-strings -DISE_DEBUG=1 $(IncludeDirectories)

# The default target that builds all the executables.
all: build_lib build_tests build_test_application | setup

# This command runs the test application.
run: build/test_application
	./build/test_application

#  This command runs the unit tests.
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

ise_keyword_table: code/ise_keyword_table.cpp | setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_bk_tree: code/ise_bk_tree.cpp | setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise: code/ise.cpp | setup
	g++ $(CompileFlags) -c code/$@.cpp -o build/$@.o

ise_tests: tests/ise_tests.cpp | setup
	g++ $(CompileFlags) -c tests/$@.cpp -o build/$@.o

ise_test_application: tests/ise_test_application.cpp | setup
	g++ $(CompileFlags) -c tests/$@.cpp -o build/$@.o

# This target builds the core library.
build_lib: ise_match ise_keyword_list ise_keyword_table ise_bk_tree ise | setup
	g++ $(CompileFlags) -shared -fPIC build/ise_match.o build/ise_keyword_list.o build/ise_keyword_table.o build/ise_bk_tree.o build/ise.o -o build/libcore.so

# This target builds the unit tests application.
build_tests: ise_tests | setup
	g++ $(CompileFlags) build/ise_tests.o -Lbuild -lcore -o build/tests

# This target builds the test application.
build_test_application: ise_test_application | setup
	g++ $(CompileFlags) build/ise_test_application.o -Lbuild -lcore -o build/test_application
