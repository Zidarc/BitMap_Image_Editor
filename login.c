#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "login.h"

#define FILEPATH "users.txt"
#define XOR_KEY 7 

// Encrypt using XOR
int encrypt(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i] ^ XOR_KEY;
        i++;
    }
    dest[i] = '\0';
    return 0;
}


int decrypt(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i] ^ XOR_KEY;
        i++;
    }
    dest[i] = '\0';
    return 0;
}

// Signup (register)
int signup(const char *username, const char *password) {

    FILE *fp = fopen(FILEPATH, "a+");
    if (!fp) {
        return LOGIN_FILE_ERROR;
    }

    char fileUser[100], filePass[100];

    // Move to beginning to read
    rewind(fp);

    // Check for duplicate username
    while (fscanf(fp, "%s %s", fileUser, filePass) == 2) {
        if (strcmp(fileUser, username) == 0) {
            fclose(fp);
            return LOGIN_USERNAME_EXISTS;
        }
    }

    // Encrypt password before storing
    char encPass[100];
    encrypt(encPass, password);

    fprintf(fp, "%s %s\n", username, encPass);

    fclose(fp);
    return LOGIN_OK;
}

// Login (check credentials)
int login(const char *username, const char *password) {

    FILE *fp = fopen(FILEPATH, "r");
    if (!fp) {
        return LOGIN_FILE_ERROR;
    }

    char line[256], fileUser[100], filePass[100];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%s %s", fileUser, filePass) == 2) {
            
            if (strcmp(fileUser, username) == 0) {

                char decPass[100];
                decrypt(decPass, filePass);

                if (strcmp(decPass, password) == 0) {
                    found = 1;
                    break;
                }
            }
        }
    }

    fclose(fp);

    if (found)
        return LOGIN_OK;
    else
        return LOGIN_INVALID_CREDENTIALS;
}
