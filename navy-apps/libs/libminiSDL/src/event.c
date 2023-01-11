#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};
static const uint32_t nr_key = 83;
static uint8_t key_state[83] = {0};
int ParseEvent(uint8_t* key, uint8_t* type){
  char buf[64];
	if(NDL_PollEvent(buf, 64)<1){
		return 0;// Error
	}
	char *prefix = strtok(buf, " ");
	char *name = strtok(NULL, " ");
	uint8_t code = atoi(strtok(NULL, " "));
	if(prefix[0]=='k'){
		*key = code;
		switch(prefix[1]){
			case 'd': *type = SDL_KEYDOWN;	key_state[code] = 1; return 1;
			case 'u': *type = SDL_KEYUP;	  key_state[code] = 0; return 1;
			default: return 0;// keyboard but no action.
		}
//				printf("event type: %d key code: %d\n", event->type, (int32_t)event->key.keysym.sym);
	}
	return 0;
}
int SDL_PushEvent(SDL_Event *ev) {
	printf("SDL_PushEvent to implement!\n");
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
		if(ev==NULL) return 0;
		uint8_t key = 0, type = 0;
		int ret = ParseEvent(&key, &type);
    if(ret){
		  ev->type = type;
		  ev->key.keysym.sym = key;
    }
		return ret;
}

int SDL_WaitEvent(SDL_Event *event) {
		if(event==NULL)	return 0;
		uint8_t key = 0, type = 0;
		while(ParseEvent(&key, &type)==0);
		event->type = type;
		event->key.keysym.sym = key;
		return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
	return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
	//SDL_Event ev;
	//SDL_PollEvent(&ev);
	if(numkeys){
		*numkeys = 0;
		for(int i=0; i<nr_key; i++){
			*numkeys+=key_state[i];
		}
	}
	return key_state;
}
