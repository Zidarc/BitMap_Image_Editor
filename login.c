#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "login.h"

#define FILEPATH "users.txt"
#define XOR_KEY 7 

// Encrypt password: XOR + hex encoding
int encrypt(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        unsigned char c = src[i] ^ XOR_KEY;
        sprintf(dest + i*2, "%02X", c); // 2 hex digits per char
        i++;
    }
    dest[i*2] = '\0'; // null terminate
    
}

// Decrypt password: decode hex + XOR
int decrypt(char *dest, const char *src) {
    int i = 0;
    while (src[i*2] != '\0') {
        unsigned int byte;
        sscanf(src + i*2, "%02X", &byte);
        dest[i] = (char)(byte ^ XOR_KEY);
        i++;
    }
    dest[i] = '\0'; // null terminate
}

// Signup (register)
int signup(const char *username, const char *password) {

    FILE *fp = fopen(FILEPATH, "a+");
    if (!fp) return LOGIN_FILE_ERROR;

    char fileUser[100], filePass[256]; // hex encoding is bigger

    rewind(fp);

    // Check for duplicate username
    while (fscanf(fp, "%99s %255s", fileUser, filePass) == 2) {
        if (strcmp(fileUser, username) == 0) {
            fclose(fp);
            return LOGIN_USERNAME_EXISTS;
        }
    }

    char encPass[256];
    encrypt(encPass, password);

    fprintf(fp, "%s %s\n", username, encPass);
    fclose(fp);

    return LOGIN_OK;
}

// Login (check credentials)
int login(const char *username, const char *password) {

    FILE *fp = fopen(FILEPATH, "r");
    if (!fp) return LOGIN_FILE_ERROR;

    char line[512], fileUser[100], filePass[256];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

        if (sscanf(line, "%99s %255s", fileUser, filePass) == 2) {
            if (strcmp(fileUser, username) == 0) {
                char decPass[128]; // original password length
                decrypt(decPass, filePass);
                if (strcmp(decPass, password) == 0) {
                    found = 1;
                    break;
                }
            }
        }
    }

    fclose(fp);
    return found ? LOGIN_OK : LOGIN_INVALID_CREDENTIALS;
}
