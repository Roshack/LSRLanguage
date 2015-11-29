CC = g++

all: lsr

lsr: lsr_bison.cpp lsr_flex.cpp lsr_classes.cpp lsr_main.cpp
	g++ -o lsr lsr_bison.cpp lsr_flex.cpp lsr_classes.cpp lsr_main.cpp

lsr_bison.cpp: lsr_bison.y
	bison -d -o lsr_bison.cpp lsr_bison.y

lsr_flex.cpp: lsr_flex.l
	lex -o lsr_flex.cpp lsr_flex.l

lsr_classes.cpp: lsr_classes.hpp
	g++ -c lsr_classes.cpp

clean:
	rm -f lsr_bison.cpp
	rm -f lsr_flex.cpp
	rm -f lsr_bison.hpp
	rm -f lsr
