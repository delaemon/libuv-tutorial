#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

#define FIB_UNTIL 100

uv_loop_t *loop;

int _fib(int n) {
    if (n < 2) {
        return n;
    }
    return _fib(n - 1) + _fib(n - 2);
}

void fib(uv_work_t *req) {
    int n = *(int *) req->data;
    if (random() % 2) {
        sleep(1);
    } else {
        sleep(3);
    }
    long fib = _fib(n);
    fprintf(stderr, "%dth fibonacci is %lu\n", n, fib);
}

void after_fib(uv_work_t *req, int status) {
    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int *)req->data);
}

int main() {
    loop = uv_default_loop();

    int data[FIB_UNTIL];
    uv_work_t req[FIB_UNTIL];
    for (int i = 0; i < FIB_UNTIL; i++) {
        data[i] = i;
        req[i].data = (void *)&data[i];
        uv_queue_work(loop, &req[i], fib, after_fib);
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}
