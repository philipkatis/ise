![tests](https://github.com/philipkatis/ise/actions/workflows/main.yml/badge.svg)

# Inverted Search Engine - K23A University Project

This repository has the code for the Inverted Search Engine project. It is comprised of the
core codebase as well as a few unit tests.

## Instructions

1. Download the repository:

To download the repository you can use `git clone --recursive https://github.com/philipkatis/ise`.
This ensures that both the repository and all of the required git submodules are downloaded together.

In case you used `git clone https://github.com/philipkatis/ise` you can use `git submodule update --init`
to download all the required submodules.

2. Compile:

In order to compile the application, you can use `make`. This creates an output directory and places all
of the executables in it.

3. Run the tests:

You can run the available unit tests using the command `make tests`.

## Third Party Code

The project uses [acutest](https://github.com/mity/acutest) as the unit testing library. It is a very
simple, header-only C library. It is included in the project as a git submodule.
