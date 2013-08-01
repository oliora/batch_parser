Batch Parser
============

MS batch file format parser written with Boost.Spirit. Created mainly as side effect of Boost.Spirit education.
Tool parses a batch file and extracts a list of near all present commands and their arguments.
The only useful purpose of this tool is to detect whether some file is a batch file :).

**WIP!!!**


Prerequisites
-------------

- [CMake](http://www.cmake.org) version 2.8 or above
- [Boost](http://www.boost.org) version 1.41 or above (Note that only 1.51 was tested)
- Your favourite dev tool (IDE or *make* or something) supporting at least C++03 (Note that only Visual Studio 2008 and Xcode 4.6 was tested)

How to build
------------

1. Setup Boost location with `set BOOST_ROOT=<root_of_boost>`

2. Generate solution/project for your favourite dev tool:

  ```
  cd <batch_parser_dir>
  mkdir workspace
  cd workspace
  cmake ..
  ```
  
  For now you should have the solution/project generated under *workspace* subdirectory.
  
  *Note about `-G` option of CMake to choose dev tool to generate solution/project for.*

3. Build generated solution/project either with you favourite compiler or with CMake `cmake --build . [--config Release|Debug]`.
