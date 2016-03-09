#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

uv_loop_t *loop;
uv_process_t child_req;
uv_process_options_t options;

void cleanup_handles(uv_process_t *req, int64_t exit_status, int term_signal) {
    fprintf(stderr, "Process exited with sattus %" PRId64 ", signal %d\n", exit_status, term_signal);
    uv_close((uv_handle_t*) req->data, NULL);
    uv_close((uv_handle_t*) req, NULL);
}

void invoke_cgi_script(uv_tcp_t *client) {
    char *args[2];
    args[0] = "./cgi-tick";
    args[1] = NULL;

    options.stdio_count = 3;
    uv_stdio_container_t child_stdio[3];
    child_stdio[0].flags = UV_IGNORE;
    child_stdio[1].flags = UV_INHERIT_STREAM;
    child_stdio[1].data.stream = (uv_stream_t*)client;
    child_stdio[2].flags = UV_IGNORE;
    options.stdio = child_stdio;
    options.exit_cb = cleanup_handles;
    options.file = args[0];
    options.args = args;
    child_req.data = (void*) client;
    int r;
    if ((r = uv_spawn(loop, &child_req, &options))) {
        fprintf(stderr, "%s\n", uv_strerror(r));
        return;
    }
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status == -1) {
        // error
        return;
    }
    uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        invoke_cgi_script(client);
    } else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

void remove_sock(int sig) {
    uv_fs_t req;
    uv_fs_unlink(loop, &req, "echo.sock", NULL);
    exit(0);
}

int main() {
    loop = uv_default_loop();
    uv_pipe_t server;
    uv_pipe_init(loop, &server, 0);

    signal(SIGINT, remove_sock);
    int r;
    if ((r = uv_pipe_bind(&server, "echo.sock"))) {
        fprintf(stderr, "Bind erro %s\n", uv_err_name(r));
        return 1;
    }
    if ((r = uv_listen((uv_stream_t*)&server, 128, on_new_connection))) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return 2;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
