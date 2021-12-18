![tests](https://github.com/philipkatis/ise/actions/workflows/main.yml/badge.svg)

# Inverted Search Engine - K23A University Project

This repository has the code for the Inverted Search Engine project. It is comprised of the
core codebase as well as a few unit tests and an example application.

***

## Instructions

<ins>**1. Download the repository:**</ins>

To download the repository you can use `git clone --recursive https://github.com/philipkatis/ise`.
This ensures that both the repository and all of the required git submodules are downloaded together.

In case you used `git clone https://github.com/philipkatis/ise` you can use `git submodule update --init`
to download all the required submodules.

<ins>**2. Compile:**</ins>

In order to compile the application, you can use `make`. This creates an output directory and places all
of the executables in it.

<ins>**3. Run the tests:**</ins>

You can run the available unit tests using the command `make tests`.

<ins>**4. Run the example:**</ins>

You can run the available example test using the command `make run`.
There is also the ability to run a valgrind memory test to ensure no memory leaks are present,
using `make valgrind`.

***

## Third Party Code

The example code is provided by the sigmod 2013 sample. It was lightly modified to work with the
ISE code.

The project uses [acutest](https://github.com/mity/acutest) as the unit testing library. It is a very
simple, header-only C library. It is included in the project as a git submodule.
