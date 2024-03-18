#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

extern uint8_t* keystate;
static int key_num = 0;

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char key[20] = {};
  // printf("waiting for event\n");
  int ret = NDL_PollEvent(key, sizeof(key));
  if(ret == 0)
    return 0;
  // printf("event: %s\n", key);
  if(key[1] =='u') {
    key_num--;
    ev->type = SDL_KEYUP;
  }
  else if(key[1] =='d') {
    key_num++;
    ev->type = SDL_KEYDOWN;
  }
  for(int i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++) {
    if(!strncmp(keyname[i], key + 3, strlen(key + 3)) && strlen(keyname[i]) == strlen(key + 3)) {
      ev->key.keysym.sym = i;
      keystate[i] = key[1] == 'd' ? 1 : 0;
      break;
    }
  }
  // printf("event: %d, keyname[%d]: %s\n", ev->key.keysym.sym, ev->key.keysym.sym, keyname[ev->key.keysym.sym]);
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  char key[20] = {};
  // printf("waiting for event\n");
  while(!NDL_PollEvent(key, sizeof(key)));
  // printf("event: %s\n", key);
  if(key[1] =='u')
    event->type = SDL_KEYUP;
  else if(key[1] =='d')
    event->type = SDL_KEYDOWN;
  for(int i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++) {
    if(strncmp(keyname[i], key + 3, strlen(key + 3)) == 0 && strlen(keyname[i]) == strlen(key + 3)) {
      event->key.keysym.sym = i;
      break;
    }
  }
  // printf("event: %d\n", event->key.keysym.sym);
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if(numkeys)
    *numkeys = key_num;
  return keystate;
}
