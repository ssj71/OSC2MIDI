//spencer jackson
//oscserver.c
//modified from the example code from the liblo documentation
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lo/lo.h"
#include "pair.h"

int done = 0;

void error(int num, const char *m, const char *path);

int msg_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);


int run_osc_server(char* port)
{
    /* start a new server on port 7770 */
    lo_server_thread st = lo_server_thread_new(port, error);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, NULL, NULL, msg_handler, NULL);

    lo_server_thread_start(st);

    while (!done) {
        usleep(1000);
        printf("hi mom!");
    }

    lo_server_thread_free(st);

    return 0;
}

void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
int msg_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{
    int i;

    printf("path: <%s>\n", path);
    for (i = 0; i < argc; i++) {
        printf("arg %d '%c' ", i, types[i]);
        lo_arg_pp((lo_type)types[i], argv[i]);
        printf("\n");
    }
    printf("\n");
    fflush(stdout);

    return 1;
}
