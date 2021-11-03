.SILENT:
.PHONY: clean

OutputDirectory = build
CommonCompilerFlags = -g -Wno-write-strings -Icode

first: code/ise.cpp tests/ise_tests.cpp | SetupOutputDirectory
	g++ $(CommonCompilerFlags) code/ise.cpp -o $(OutputDirectory)/ise
	g++ $(CommonCompilerFlags) -Ithird_party/acutest/include tests/ise_tests.cpp -o $(OutputDirectory)/tests

run: ./$(OutputDirectory)/ise
	./$(OutputDirectory)/ise

tests: $(OutputDirectory)/tests
	./$(OutputDirectory)/tests

SetupOutputDirectory:
	mkdir -p $(OutputDirectory)

clean:
	rm -rf $(OutputDirectory)
