#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "login.h"
#define FILEPATH "users.txt"

int signup(const char *username, const char *password) {

    FILE *fp = fopen(FILEPATH, "a+");  
    if (!fp) {
        return LOGIN_FILE_ERROR; 
    }

    char fileUser[100], filePass[100];

    // Go to start to read existing data
    rewind(fp);

    // Check if username already exists
    while (fscanf(fp, "%s %s", fileUser, filePass) == 2) {
        if (strcmp(fileUser, username) == 0) {
            fclose(fp);
            return LOGIN_USERNAME_EXISTS; 
        }
    }

    // Append new username password at end
    fprintf(fp, "%s %s\n", username, password);

    fclose(fp);
    return LOGIN_OK;
}

int login(const char *username, const char *password) {
    FILE *fp = fopen(FILEPATH, "r");   // read only
    if (!fp) return LOGIN_FILE_ERROR;

    char line[256], fileUser[100], filePass[100];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%s %s", fileUser, filePass) == 2) {
            if (strcmp(fileUser, username) == 0 &&
                strcmp(filePass, password) == 0) {
                found = 1;
                break;
            }
        }
    }

    fclose(fp);

    if (found) return LOGIN_OK;
    else return LOGIN_INVALID_CREDENTIALS;
}