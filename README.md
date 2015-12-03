# LSRLanguage
A garbage collected programming language that no one should ever use!

Software is provided as-is and no guarantees as to anything are made! It may even break your system, it's not intended to but I suppose it could!

All installation and usage is for linux.

# Installation
First get Flex and Bison, can be done with:

$ sudo apt-get install flex bison

Also you need g++

Clone this repo.

$ cd LSRLanguage

$ ./buildLSR

# Running
Since it's an interpreter it just reads from stdin, this can be ugly as any print statements you write will print thie result out immediately, to pass in files for example the test.lsr file you just do

./lsr < test.lsr
