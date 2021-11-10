.SILENT:
.PHONY: clean

OutputDirectory = build
CommonCompilerFlags = -DISE_DEBUG -g -Wno-write-strings -Icode

first: code/ise.cpp tests/ise_tests.cpp | SetupOutputDirectory
	g++ $(CommonCompilerFlags) -Ithird_party/acutest/include tests/ise_tests.cpp -o $(OutputDirectory)/tests

tests: $(OutputDirectory)/tests
	./$(OutputDirectory)/tests

SetupOutputDirectory:
	mkdir -p $(OutputDirectory)

clean:
	rm -rf $(OutputDirectory)
