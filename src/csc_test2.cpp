/*
 *  <Short Description>
 *  Copyright (C) 2025  Brett Terpstra
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>

typedef enum TokType_T {
    OPED, OPET, LAST
} TokType;

typedef struct Token_T{
    TokType type;
    void* data;
} Token;

void addTok(Token **tokArr, size_t newSize, Token *tok){
    Token *tmp = static_cast<Token*>(realloc(*tokArr, newSize * sizeof(char *)));
    if(!tmp){
        printf("EROORRRRRRRRRRRRRR!");
        abort();
    }
    *tokArr = tmp;
    tmp = NULL;

    tokArr[newSize-1] = static_cast<Token_T*>(malloc(sizeof(Token)));

    *tokArr[newSize-1] = *tok;
}

int ma3in() {
    Token *tokArr = NULL;
    double dataD = 1;
    char dataT = '+';

    Token tok = {OPET, &dataT};
    addTok(&tokArr, 1, &tok);

    if(tok.type == OPED){
        printf("your token is: %f", *(double*)tok.data);
    }
    if(tok.type == OPET){
        printf("your token is: %c", *(char*)tok.data);
    }

    return 0;
}