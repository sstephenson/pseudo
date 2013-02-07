#include <getopt.h>
#include <libgen.h>
#include <pwd.h>
#include <Security/Authorization.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PSEUDO_VERSION "1.0.0-pre"

void version(FILE *);
void usage(FILE *);
int verify_sudo_session();
int authenticate_sudo_session(char *);
int validate_sudo_session();
int run_command_with_privileges(char *, char *);
void pipe_stream(FILE *, FILE *);
char *get_user_name();
