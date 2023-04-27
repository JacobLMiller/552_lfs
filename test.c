#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* extractSubstring(char* inputString,char *remainder) {
    char* start = strchr(inputString, '/');
    if (start == NULL) {
        return NULL;
    }
    char* end = strchr(start + 1, '/'); 
    if (end == NULL) {
        end = inputString + strlen(inputString); 
    }
    size_t length = end - start - 1; 
    size_t remlen = strlen(inputString) - length;
    char* substring = malloc(length + 1); 
    if (substring == NULL) {
        return NULL; 
    }
    memcpy(substring, start + 1, length);
    memcpy(remainder,end,remlen);
    substring[length] = '\0'; 
    return substring;
}

int main() {
    char str[] = "/foo/bar/baz";
    char* token;
    char* rest = str;
 
    while ((token = strtok_r(rest, "/", &rest)))
        printf("%s\n", token);
}
