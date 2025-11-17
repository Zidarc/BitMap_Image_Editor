
#define LOGIN_H
#define LOGIN_OK 0
#define LOGIN_USERNAME_EXISTS 1
#define LOGIN_FILE_ERROR 2
#define LOGIN_INVALID_CREDENTIALS 3

int signup(const char *username, const char *password);
int login(const char *username, const char *password);
int encrypt(char *dest, const char *src);
int decrypt(char *dest, const char *src);

