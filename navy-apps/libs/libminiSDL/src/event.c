#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char key[20] = {};
  printf("waiting for event\n");
  while(!NDL_PollEvent(key, sizeof(key)));
  printf("event: %s\n", key);
  if(key[1] =='u')
    event->type = SDL_KEYUP;
  else if(key[1] =='d')
    event->type = SDL_KEYDOWN;
  for(int i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++) {
    if(strncmp(keyname[i], key + 3, strlen(key + 3)) == 0) {
      event->key.keysym.sym = i;
      break;
    }
  }
  printf("keyname[0]: %s\n", keyname[0]);
  printf("key+3: %s\n", key + 3);
  printf("event: %d\n", event->key.keysym.sym);
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
