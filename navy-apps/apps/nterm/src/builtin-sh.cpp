#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
  char* argv[16] = {NULL};
  int argc = 1;
  const char* start = cmd;
  int len = 0;
  while (*cmd)
  {
    if(*cmd == ' ' || *cmd == '\n')
    {
      if(len == 0)
        continue;
      argv[argc] = (char*)malloc(len + 1);
      memcpy(argv[argc], start, len);
      argv[argc][len] = '\0';
      argc++;
      len = 0;
      cmd++;
      start = cmd;
      if(argc == 16)
      {
        printf("Too many arguments\n");
        return;
      }
      continue;
    }
    cmd++;
    len++;
  }
  argv[argc] = NULL;

  int i = 0;
  while (argv[i] != NULL)
  {
    printf("argv[%d] = %s\n", i, argv[i]);
    i++;
  }

  char* envp[] = {"/bin", "/usr/bin"};
  execve(argv[0], argv, envp);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
