#include <stdio.h>
#include "login.h"
#include "filters.h"

int main() {
    int status = signup("ali", "12345");

    if (status == LOGIN_OK) {
        printf("Signup successful!\n");
    } 
    else if (status == LOGIN_USERNAME_EXISTS) {
        printf("Error: Username already exists.\n");
    }
    else {
        printf("Error: Could not open file.\n");
    }

    return 0;
}