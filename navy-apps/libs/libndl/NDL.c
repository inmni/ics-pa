#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static struct timeval tv;
static uint64_t startTime = 0;
uint32_t NDL_GetTicks() {
  	gettimeofday(&tv, NULL);
		return tv.tv_usec/1000+tv.tv_sec*1000-startTime;
}

int NDL_PollEvent(char *buf, int len) {
		return read(evtdev, buf, len)>0;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = open("/proc/dispinfo", 0, 0);
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
	if(*w > screen_w || *h> screen_h){
		printf("Error canvas size\n");
	}
	if(*w==0 && *h==0){
		*w = screen_w; *h = screen_h;
	}
	canvas_w = *w;	canvas_h = *h;
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
	x += (screen_w - canvas_w)>>1;
	y += (screen_h - canvas_h)>>1;
	lseek(fbdev, (x + y * screen_w)<<2, SEEK_SET);
	while(h > 0){
			write(fbdev, pixels, w<<2);
			pixels += w;	h--;
			lseek(fbdev, (screen_w-canvas_w) << 2, SEEK_CUR);
	}
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
	gettimeofday(&tv, NULL);
	startTime = tv.tv_sec*1000-tv.tv_usec/1000;
 	
	evtdev = open("/dev/events", 0, 0);
	fbdev = open("/dev/fb", 0, 0);

	int fd = open("/proc/dispinfo", 0, 0);
	char buf[64];
	
	read(fd, buf, 64);
	char *key1 = strtok(buf, ":");
	char *val1 = strtok(NULL, "\n");
	char *key2 = strtok(NULL, ":");
	char *val2 = strtok(NULL, "\n");
	if(strcmp(key1, "WIDTH")==0){screen_w = atoi(val1);}
	else if(strcmp(key1, "HEIGHT")==0){screen_h = atoi(val1);}
	else{printf("valid /proc/dispinfo file\n"); return 1;}

	if(strcmp(key2, "WIDTH")==0){screen_w = atoi(val2);}
	else if(strcmp(key2, "HEIGHT")==0){screen_h = atoi(val2);}
	else{printf("valid /proc/dispinfo file\n"); return 1;}
	
	return 0;
}

void NDL_Quit() {
}
