#include <cstdio>
#include <sys/signal.h>
#include <unistd.h>

int main() {
    char b[12] = {0};
    auto b_address = &b;
    write(STDOUT_FILENO, &b_address, sizeof(void*));
    fflush(stdout);
    raise(SIGTRAP);

    printf("%s", b);
}