#include "resource.h"

Topic preface[] = {
    {"Preface", NULL, APP_ASSETS_PATH("preface/preface.txt")},
    {"Preface to the first edition", NULL, APP_ASSETS_PATH("preface/preface_to_the_first_edition.txt")}
};

Topic chapter1[] = { 
    {"1.0 A Tutorial Introduction", NULL, APP_ASSETS_PATH("chapter1/0-chapter1.txt")}, 
    {"1.1 Getting Started", NULL, APP_ASSETS_PATH("chapter1/1-chapter1.1.txt")}, 
    {"1.2 Variables and Arithmetic Expressions", NULL, APP_ASSETS_PATH("chapter1/2-chapter1.2.txt")},
    {"1.3 The For Statement", NULL, APP_ASSETS_PATH("chapter1/3-chapter1.3.txt")}, 
    {"1.4 Symbolic Constants", NULL, APP_ASSETS_PATH("chapter1/4-chapter1.4.txt")}, 
    {"1.5 Character Input and Output", NULL, APP_ASSETS_PATH("chapter1/5-chapter1.5.txt")}, 
    {"1.6 Arrays", NULL, APP_ASSETS_PATH("chapter1/6-chapter1.6.txt")}, 
    {"1.7 Functions", NULL, APP_ASSETS_PATH("chapter1/7-chapter1.7.txt")}, 
    {"1.8 Arguments—Call by Value", NULL, APP_ASSETS_PATH("chapter1/8-chapter1.8.txt")}, 
    {"1.9 Character Arrays", NULL, APP_ASSETS_PATH("chapter1/9-chapter1.9.txt")}, 
    {"1.10 External Variables and Scope", NULL, APP_ASSETS_PATH("chapter1/10-chapter1.10.txt")}, 
    {"Exercises", NULL, APP_ASSETS_PATH("chapter1/exercises.txt")}, 
};

Topic chapter2[] = {
    {"2.0 Types, Operators, and Expressions", NULL, APP_ASSETS_PATH("chapter2/0-chapter2.txt")},
    {"2.1 Variable Names", NULL, APP_ASSETS_PATH("chapter2/1-chapter2.1.txt")},
    {"2.2 Data Types and Sizes", NULL, APP_ASSETS_PATH("chapter2/2-chapter2.2.txt")},
    {"2.3 Constants", NULL, APP_ASSETS_PATH("chapter2/3-chapter2.3.txt")},
    {"2.4 Declarations", NULL, APP_ASSETS_PATH("chapter2/4-chapter2.4.txt")},
    {"2.5 Arithmetic Operators", NULL, APP_ASSETS_PATH("chapter2/5-chapter2.5.txt")},
    {"2.6 Relational and Logical Operators", NULL, APP_ASSETS_PATH("chapter2/6-chapter2.6.txt")},
    {"2.7 Type Conversions", NULL, APP_ASSETS_PATH("chapter2/7-chapter2.7.txt")},
    {"2.8 INCREMENT AND DECREMENT OPERATORS 47", NULL, APP_ASSETS_PATH("chapter2/8-chapter2.8.txt")},
    {"2.9 Bitwise Operators", NULL, APP_ASSETS_PATH("chapter2/9-chapter2.9.txt")},
    {"2.10 Assignment Operators and Expressions", NULL, APP_ASSETS_PATH("chapter2/10-chapter2.10.txt")},
    {"2.11 Conditional Expressions", NULL, APP_ASSETS_PATH("chapter2/10-chapter2.11.txt")},
    {"2.12 Precedence and Order of Evaluation", NULL, APP_ASSETS_PATH("chapter2/10-chapter2.12.txt")},
    {"Exercises", NULL, APP_ASSETS_PATH("chapter2/exercises.txt")},
};

// Topic chapter3[] = {
// };

// Topic chapter4[] = {
// };

// Topic chapter5[] = {
// };

// Topic chapter6[] = {
// };

// Topic chapter7[] = {
// };

// Topic chapter8[] = {
// };

Topic license[] = {
    {"LICENSE", NULL, APP_ASSETS_PATH("LICENSE.txt")},
};

Chapter chapters[] = {
    {"Preface", preface, sizeof(preface) / sizeof(Topic)},
    {"Ch1-Introduction", chapter1, sizeof(chapter1) / sizeof(Topic)},
    {"Ch2-Types, Ops, Expr", chapter2, sizeof(chapter2) / sizeof(Topic)},
    // {"Ch3-Control Flow", chapter3, sizeof(chapter3) / sizeof(Topic)},
    // {"Ch4-Functions", chapter4, sizeof(chapter4) / sizeof(Topic)},
    // {"Ch5-Pointers, Arrays", chapter5, sizeof(chapter5) / sizeof(Topic)},
    // {"Ch6-Structures", chapter6, sizeof(chapter6) / sizeof(Topic)},
    // {"Ch7-Input, Output", chapter7, sizeof(chapter7) / sizeof(Topic)},
    // {"Ch8-UNIX Sys Interface", chapter8, sizeof(chapter8) / sizeof(Topic)},
    {"LICENSE", license, sizeof(license) / sizeof(Topic)},
};

const size_t number_of_chapters = sizeof(chapters) / sizeof(Chapter);