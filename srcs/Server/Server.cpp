#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    int kq = kqueue();
    if (kq == -1) {
        perror("kqueue");
        exit(EXIT_FAILURE);
    }

    int fd = open("example.txt", O_RDONLY | O_NONBLOCK); // O_NONBLOCK 플래그 추가
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct kevent change;
    EV_SET(&change, fd, EVFILT_VNODE, EV_ADD | EV_ENABLE, 0, 0, NULL);

    if (kevent(kq, &change, 1, NULL, 0, NULL) == -1) {
        perror("kevent");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 파일에 데이터 쓰기
    const char *msg = "Hello, world!";
    write(fd, msg, strlen(msg));

    struct kevent event;
    while(1)
    {
        printf("done\n");
        int nev = kevent(kq, NULL, 0, &event, 1, NULL);
        printf("nev = %d\n event = %d\n", nev, event.filter);
        if (nev == -1) {
            perror("kevent");
            close(fd);
            exit(EXIT_FAILURE);
        } else if (nev > 0) {
            if (event.filter == EVFILT_READ) {
                printf("Readable event occurred on fd %d\n", fd);
                // 파일로부터 데이터 읽기
                char buffer[10000000];
                ssize_t nread = read(fd, buffer, sizeof(buffer) - 1);
                if (nread == -1) {
                    perror("read");
                    close(fd);
                    exit(EXIT_FAILURE);
                } else if (nread > 0) {
                    buffer[nread] = '\0';
                    printf("Read data: %s\n", buffer);
                }
            }
        }
    }

    close(fd);
    return 0;
}