/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple shell
 */

#include "app.h"
#include <string.h>

void parse_request(char* buf, struct proc_request* req) {
    int idx = 0, nargs = 0;
    memset(req->argv, 0, CMD_NARGS * CMD_ARG_LEN);

    for (int i = 0; i < strlen(buf); i++)
        if (buf[i] != ' ') {
            req->argv[nargs][idx++] = buf[i];
        } else if (idx != 0) {
            idx = req->argv[nargs][idx] = 0;
            nargs++;
        }
    req->argc = idx ? nargs + 1 : nargs;
}

int main() {
    CRITICAL("Welcome to egos-2000!");
    
    char buf[256] = "cd";  /* Enter the home directory first */
    while (1) {
        struct proc_request req;
        struct proc_reply reply;

        if (strcmp(buf, "killall") == 0) {
            req.type = PROC_KILLALL;
            grass->sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
        } else {
            req.type = PROC_SPAWN;
            parse_request(buf, &req);
            grass->sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
            grass->sys_recv(NULL, (void*)&reply, sizeof(reply));

            if (reply.type != CMD_OK)
                INFO("sys_shell: command causes an error");
            else if (req.argv[req.argc - 1][0] != '&')
                grass->sys_recv(NULL, (void*)&reply, sizeof(reply));
        }

        do {
            printf("\x1B[1;32m➜ \x1B[1;36m%s\x1B[1;0m ", grass->workdir);
        } while (earth->tty_read(buf, 256) == 0);
    }
}
