#include <iostream>
#include "lsr_classes.hpp"
extern LSRBlock* programBlock;
extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();
    std::cout << programBlock << std::endl;
    return 0;
}
