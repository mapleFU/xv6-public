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
    // int fd = open("/passwd", O_RDONLY);
    // if (fd > 0) {
    //     char buf[512];
    //     char data[512];
    //     int n;
    //     int sum = 0;
    //     while ((n = write(fd, buf, sizeof(buf))) > 0) {
            
    //         strcpy(data + sum, buf);
    //         sum += n;
    //     }
    // }
    strcpy(username, argv[1]);
    strcpy(password, argv[2]);
    su(username, password);
    exit();
}