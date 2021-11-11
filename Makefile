.SILENT:
.PHONY: clean

OutputDirectory = build
CompilerFlags = -DISE_DEBUG=1 -g -Wno-write-strings -Ithird_party/acutest/include -Icode

compile: tests/ise_tests.cpp | SetupOutputDirectory
	g++ $(CompilerFlags) tests/ise_tests.cpp -o $(OutputDirectory)/tests

tests: $(OutputDirectory)/tests
	./$(OutputDirectory)/tests

SetupOutputDirectory:
	mkdir -p $(OutputDirectory)

clean:
	rm -rf $(OutputDirectory)
