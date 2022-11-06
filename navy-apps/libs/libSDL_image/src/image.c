#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  	FILE *file = fopen(filename, "rb");
		if(file == NULL){
				printf("File %s No Found\n", filename);
				return NULL;
		}
		
		fseek(file, 0, SEEK_END);
		uint32_t file_size = ftell(file);
		
		fseek(file, 0, SEEK_SET);
		
		char *buf = (char *)malloc(file_size);
		fread(buf, file_size, 1, file);
		SDL_Surface *s = STBIMG_LoadFromMemory(buf, file_size);
		
		fclose(file);
		free(buf);

		return s;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
