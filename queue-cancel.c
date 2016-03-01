#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

#define FIB_UNTIL 10

uv_loop_t *loop;
uv_work_t fib_reqs[FIB_UNTIL];

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

void signal_handler(uv_signal_t *req, int signum) {
    printf("Signal received!\n");
    for (int i = 0; i < FIB_UNTIL; i++) {
        uv_cancel((uv_req_t*) &fib_reqs[i]);
    }
    uv_signal_stop(req);
}

void after_fib(uv_work_t *req, int status) {
    if(status == UV_ECANCELED) {
        fprintf(stderr, "Calculation of %d cancelled.\n", *(int *) req->data);
    }
}

int main() {
    loop = uv_default_loop();
    int data[FIB_UNTIL];
    for (int i = 0; i < FIB_UNTIL; i++) {
        data[i] = i;
        fib_reqs[i].data = (void *) &data[i];
        uv_queue_work(loop, &fib_reqs[i], fib, after_fib);
    }

    uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);

    return uv_run(loop, UV_RUN_DEFAULT);
}
