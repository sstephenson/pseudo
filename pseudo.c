#include "pseudo.h"
const char *program_name;

int main(int argc, char **argv) {
  int o, wait = 0;
  char *message = "";
  program_name = basename(argv[0]);

  static struct option options[] = {
    { "help",    no_argument,       NULL, 'h' },
    { "message", required_argument, NULL, 'm' },
    { "version", no_argument,       NULL, 'v' },
    { "wait",    no_argument,       NULL, 'w' },
    { NULL,      0,                 NULL, 0   }
  };

  while ((o = getopt_long(argc, argv, "hm:vw", options, NULL)) != -1) {
    switch (o) {
    case 'h':
      version(stdout);
      usage(stdout);
      return 0;
    case 'm':
      message = optarg;
      break;
    case 'v':
      version(stdout);
      return 0;
    case 'w':
      wait = 1;
      break;
    default:
      usage(stderr);
      return 1;
    }
  }

  if (!verify_sudo_session()) {
    if (!authenticate_sudo_session(message)) {
      return 1;
    }
  }

  if (wait) {
    int count = 0;
    while (getppid() != 1 && !(count == 0 && !validate_sudo_session())) {
      count = (count + 1) % 30;
      sleep(1);
    }
    return 1;

  } else {
    if (!validate_sudo_session()) {
      return 1;
    }
  }

  return 0;
}

void version(FILE *file) {
  fprintf(file, "Pseudo %s\n", PSEUDO_VERSION);
}

void usage(FILE *file) {
  fprintf(file, "Usage: %s [ -m | --message <message>] [-w | --wait]\n", program_name);
}

int verify_sudo_session() {
  return system("/bin/test -z \"$(/usr/bin/sudo -n /usr/bin/true 2>&1)\"") == 0;
}

int authenticate_sudo_session(char *message) {
  char buf[512];
  char *user = get_user_name();
  char *command = "mkdir -p /var/db/sudo/%s && touch /var/db/sudo/%s";
  snprintf(buf, sizeof(buf), command, user, user);
  return run_command_with_privileges(message, buf) == 0;
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
