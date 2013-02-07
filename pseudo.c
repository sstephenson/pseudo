#include "pseudo.h"
const char *program_name;

int main(int argc, char **argv) {
  int o, wait = 0;
  char *message = "";
  program_name = basename(argv[0]);

  static struct option options[] = {
    { "help",    no_argument,       NULL, 'h' },
    { "message", required_argument, NULL, 'm' },
    { "wait",    no_argument,       NULL, 'w' },
    { NULL,      0,                 NULL, 0   }
  };

  while ((o = getopt_long(argc, argv, "hm:w", options, NULL)) != -1) {
    switch (o) {
    case 'h':
      usage();
      return 0;
    case 'm':
      message = optarg;
      break;
    case 'w':
      wait = 1;
      break;
    default:
      usage();
      return 1;
    }
  }

  if (!verify_sudo_session()) {
    if (!authenticate_sudo_session(message)) {
      return 1;
    }
  }

  if (wait) {
    while (validate_sudo_session()) {
      sleep(30);
    }
    return 1;

  } else {
    if (!validate_sudo_session()) {
      return 1;
    }
  }

  return 0;
}

void usage() {
  fprintf(stderr, "usage: %s [ -m | --message <message>] [-w | --wait]\n", program_name);
}

int verify_sudo_session() {
  return system("/usr/bin/sudo -n /usr/bin/true 2>/dev/null") == 0;
}

int authenticate_sudo_session(char *message) {
  char buf[512];
  char *user = get_user_name();
  char *command = "mkdir -p /var/db/sudo/%s && touch /var/db/sudo/%s";
  snprintf(buf, sizeof(buf), command, user, user);
  return run_command_with_privileges(message, buf);
}

int validate_sudo_session() {
  if (verify_sudo_session()) {
    return system("/usr/bin/sudo -v 2>/dev/null") == 0;
  }
  return 0;
}

int run_command_with_privileges(char *message, char *command) {
  OSStatus status;
  AuthorizationRef authorization_ref;

  status = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &authorization_ref);
  if (status != errAuthorizationSuccess) {
    return 1;
  }

  AuthorizationItem   right  = {"system.privilege.admin", 0, NULL, 0};
  AuthorizationRights rights = {1, &right};
  AuthorizationFlags  flags  = kAuthorizationFlagDefaults |
                               kAuthorizationFlagInteractionAllowed |
                               kAuthorizationFlagPreAuthorize |
                               kAuthorizationFlagExtendRights;

  AuthorizationItem items[1];
  items[0].name = "prompt";
  items[0].valueLength = strlen(message);
  items[0].value = message;
  items[0].flags = 0;

  AuthorizationEnvironment environment;
  environment.items = items;
  environment.count = 1;

  status = AuthorizationCopyRights(authorization_ref, &rights, &environment, flags, NULL);
  if (status != errAuthorizationSuccess) {
    return 1;
  }

  char *program = "/bin/sh";
  char *args[] = {"-c", command, NULL};
  FILE *file = NULL;

#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  status = AuthorizationExecuteWithPrivileges(authorization_ref, program, kAuthorizationFlagDefaults, args, &file);
#pragma clang diagnostic warning "-Wdeprecated-declarations"
  if (status != errAuthorizationSuccess) {
    return 1;
  }

  pipe_stream(file, stdout);
  status = AuthorizationFree(authorization_ref, kAuthorizationFlagDestroyRights);
  return 0;
}


void pipe_stream(FILE *src, FILE *dst) {
  char buf[512], size;
  while ((size = read(fileno(src), buf, sizeof(buf))) > 0) {
    write(fileno(dst), buf, size);
  }
}

char *get_user_name() {
  struct passwd *pw;
  uid_t uid;
  int c;

  uid = geteuid();
  pw = getpwuid(uid);

  if (!pw) {
    return NULL;
  } else {
    return pw->pw_name;
  }
}
