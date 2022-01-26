<<<<<<< HEAD
CommonCompileFlags=-Wall -Wno-write-strings -Wno-unused-function -fPIC -pthread -Icode -DISE_MULTI_THREADED=1

# TODO(philip): Disable exceptions, rtti, etc
# TODO(philip): Reimplement run/tests.

DebugCompileFlags=-g $(CommonCompileFlags)
ReleaseCompileFlags=-g -O2 $(CommonCompileFlags)

build_all: build_library build_tests build_example | setup

tests: build_library build_tests | setup
	./build/tests_debug
	./build/tests_release

profile: build_library build_example | setup
	valgrind --tool=callgrind ./build/example_release

clean:
	rm -rf build

setup:
	mkdir -p build

build_library: code/ise.cpp | setup
	g++ $(DebugCompileFlags) -shared code/ise.cpp -o build/libcore_debug.so
	g++ $(ReleaseCompileFlags) -shared code/ise.cpp -o build/libcore_release.so

build_tests: tests/tests.cpp | setup
	g++ $(DebugCompileFlags) -Ithird_party/acutest/include tests/tests.cpp -o build/tests_debug
	g++ $(ReleaseCompileFlags) -Ithird_party/acutest/include tests/tests.cpp -o build/tests_release

build_example: example/example.cpp | setup
	g++ $(DebugCompileFlags) example/example.cpp -Lbuild -lcore_debug -o build/example_debug -Wl,-rpath=build
	g++ $(ReleaseCompileFlags) example/example.cpp -Lbuild -lcore_release -o build/example_release -Wl,-rpath=build
=======
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
>>>>>>> wip
