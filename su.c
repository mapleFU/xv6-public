// Shell.

#include "types.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf(2, "args to little\n");
        exit();
    }
    char username[16], password[16];
    strcpy(username, argv[1]);
    strcpy(password, argv[2]);
    su(username, password);
    exit();
}