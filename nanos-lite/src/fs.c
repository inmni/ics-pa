#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
	size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
#define NR_OCCUPIED 3
#define NR_FILES (sizeof(file_table)/sizeof(Finfo))
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, invalid_write},
#include "files.h"
};
#define file(i) file_table[(i)]
void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *filename, int flags, int mode){
		// flags and mode are ignored
		int idx = NR_OCCUPIED;
		for(; idx < NR_FILES; idx++){
				if(!strcmp(file(idx).name, filename)){
								file(idx).open_offset = file(idx).disk_offset;
								return idx;
				}
		}
		return -1;
}
#define FILEOPT(f, opt) f.opt

#define def_fs_operate(opt) \
size_t fs_##opt(int fd, void *buf, size_t len) { \
		/* If the file has its own function*/ \
		if( FILEOPT(file(fd), opt) ){ \
				len = FILEOPT(file(fd), opt)(buf, file(fd).open_offset, len); \
				file(fd).open_offset += len; \
		} \
		/* Consider overflow*/ \
		if( file(fd).open_offset + len - file(fd).disk_offset > file(fd).size ){ \
				len = file(fd).size - ( file(fd).open_offset - file(fd).disk_offset ); \
		} \
\
		file(fd).open_offset += len; \
		return ramdisk_##opt(buf, file(fd).open_offset, len); \
}

def_fs_operate(write);
def_fs_operate(read);
/* 
size_t fs_read(int fd, void *buf, size_t len) {
		// If the file has its own read function
		if( file(fd).read ){
				len = file(fd).read(buf, file(fd).open_offset, len);
				file(fd).open_offset += len;
				return len;
		}
		// Consider overflow
		if( file(fd).open_offset + len - file(fd).disk_offset > file(fd).size ){
				len = file(fd).size - ( file(fd).open_offset - file(fd).disk_offset );
		}
		
		file(fd).open_offset += len;
		return ramdisk_read(buf, file(fd).open_offset, len);
}

size_t fs_write(int fd, const void *buf, size_t len) {
		// If the file has its own write function
		if( file(fd).write ){
				len = file(fd).write(buf, file(fd).open_offset, len);
				file(fd).open_offset += len;
				return len;
		}
		// Consider overflow
		if( file(fd).open_offset + len - file(fd).disk_offset > file(fd).size ){
				len = file(fd).size - ( file(fd).open_offset - file(fd).disk_offset );
		}

		file(fd).open_offset += len;
		return ramdisk_write(buf, file(fd).write_offset, len);
}
*/
size_t fs_lseek(int fd, size_t offset, int whence){
		size_t new_offset;
		switch(whence){
				case SEEK_SET:
								new_offset = file(fd).disk_offset + offset;
								break;
				case SEEK_CUR:
								new_offset = file(fd).open_offset + offset;
								break;
				case SEEK_END:
								new_offset = file(fd).disk_offset + file(fd).size + offset;
								break;
				default:
								return -1;
		}
		if( new_offset < file(fd).disk_offset || new_offset > file(fd).disk_offset + file(fd).size ){
				return -1;
		}
		file(fd).open_offset = new_offset;
		return new_offset - file(fd).disk_offset;
}

int fs_close(int fd) {
		return (fd>=NR_OCCUPIED && fd<NR_FILES)?0:-1;
}
