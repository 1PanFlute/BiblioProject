#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "format.h"

#define LONG_DESCR_LENGTH 100
#define SHORT_DESCR_LENGTH 20

// Funzione che elimina gli spazi iniziali e finali di una stringa.
char* trimWhitespace(char* str) {
    int i = 0;
    int j = strlen(str) - 1;
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
        i++;
    }
    while (str[j] == ' ' || str[j] == '\t' || str[j] == '\n') {
        j--;
    }
    str[j + 1] = '\0';
    return str + i;
}

// Funzione che esegue il tokenize delle stringhe relative ai volumi del file_recordli
// Li immagazzina all'interno di un array di volumi.
int numBooks=0;
Book parseBookRecord(char* line) {
    Book book;
    memset(&book, 0, sizeof(Book));

    char* token = strtok(line, ";");
    while (token != NULL) {
        char* colon = strchr(token, ':');
        if (colon != NULL) {
            *colon = '\0';
            char* value = trimWhitespace(colon + 1);
            if (strcmp(token, "autore") == 0) {
                strcpy(book.autore, value);
            } else if (strstr(token, "titolo") != NULL) {
                strcpy(book.titolo, value);
            } else if (strstr(token, "editore") != NULL) {
                strcpy(book.editore, value);
            } else if (strstr(token, "anno") != NULL) {
                strcpy(book.anno, value);
            } else if (strstr(token, "nota") != NULL) {
                strcpy(book.nota, value);
            } else if (strstr(token, "prestito") != NULL) {
                strcpy(book.prestito, value);
            } else if (strstr(token, "volume") != NULL) {
                strcpy(book.volume, value);
            } else if (strstr(token, "descrizione") != NULL) {
                strcpy(book.descrizione_fisica, value);
            } else if (strstr(token, "pubblicazione") != NULL) {
                strcpy(book.luogo_pubblicazione, value);
            } else if (strstr(token, "scaffale") != NULL || strstr(token, "collocazione") != NULL) {
                strcpy(book.collocazione, value);
            } else if (strstr(token, "autore")){
                size_t length = strlen(book.autore);
                book.autore[length] = ',';
                book.autore[length+1] = ' ';
                book.autore[length+2] = '\0';
                strcat(book.autore, value);
            }
        }

        token = strtok(NULL, ";");
    }
    numBooks++;
    return book;
}
