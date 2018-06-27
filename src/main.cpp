#include <cstdlib>
#include <FlexLexer.h>
#include <iostream>

int main(int argc, char **argv) {
    std::cout << "I started executing\n";

    auto *lexer = new yyFlexLexer;
    while(lexer->yylex() != 0)
        ;

    exit(1);
}
