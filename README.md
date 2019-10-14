# mpp
A basic MIPS assembly preprocessor that processes macros. It is meant to be used in conjunction with SPIM, since it does not support macros by itself. The purpose of this project is double: while I do need an assembly preprocessor for my MIPS coursework, I chose to use C (and avoid the use of regex) both to get some practice in the language and because I enjoy using it.

Right now it is a functional prototype, but requires a lot of refactoring. To compile, just clone the directory, move into it and run 'make'. No dependencies apart from gcc and make itself are used.

## TODO list
* Testing
	* Test for memory leaks
	* Add test cases, and automatically test all (i.e. using a bash script)
* Documentation
	* Document the FSMs used for parsing and replacing
	* Add some short documentation to make the program design clearer
* Functionality
	* Read input from stdin
	* Commandline arguments to set output verbosity (debug/verbose/default/quiet)
* Code quality
	* Several functions need refactoring
