#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <stdlib.h>

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
  	char buf[32];
		if(!NDL_PollEvent(buf, 32))return 0;// Error
		printf("Get Event %s\n", buf);
		char *prefix = strtok(buf, " ");
		char *name = strtok(NULL, " ");
		uint8_t code = atoi(strtok(NULL, " "));
		if(prefix[0]=='k'){
				event->key.keysym.sym = code;
				switch(prefix[1]){
						case 'd': event->key.type = SDL_KEYDOWN;break;
						case 'u': event->key.type = SDL_KEYUP;	break;
						default: return 0;// keyboard but no action.
				}
				event->type = event->key.type;
		}
		return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
