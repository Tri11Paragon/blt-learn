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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void resizeStrArr(char*** strArr, size_t size)
{
    char** tmp = static_cast<char**>(realloc(*strArr, size * sizeof(char*)));
    if (!tmp)
    {
        //error message here
        abort();
    }
    *strArr = tmp;
}

void addTok(char **arr, size_t num, char *tok){
    size_t tokSize = strlen(tok) + 1;
    num--;

    resizeStrArr(&arr, num);

    arr[num] = static_cast<char*>(malloc(tokSize * sizeof(char)));
    //resizeStr(&arr[num], tokSize * sizeof(char));

    for(size_t i = 0; i < tokSize; i++){
        arr[num][i] = tok[i];
    }
}

int main542343() {
    char **strArr = NULL;

    char *str = (char*) malloc(2 * sizeof(char));
    str[0] = 's';
    str[1] = '\0';

    char **tmp = (char**) realloc(strArr, 1 * sizeof(char *));
    if(!tmp){
        printf("realloc error");
        abort();
    }
    strArr = tmp;
    tmp = NULL;

    strArr[0] = (char*) malloc(2 * sizeof(char));
    strArr[0][0] = str[0];
    strArr[0][1] = str[1];

    printf("%s\n", strArr[0]);

    free(str);
    free(strArr[0]);
    free(strArr);


    return 0;
}

// int main()
// {
//     char** strArr = NULL;
//     resizeStrArr(&strArr, 1);
//     strArr[0] = static_cast<char*>(malloc(2 * sizeof(char)));
//     strArr[0][0] = 's';
//     strArr[0][1] = '\0';
//     printf("%s\n", strArr[0]);
//
//     free(strArr[0]);
//     free(strArr);
//
//     return 0;
// }
