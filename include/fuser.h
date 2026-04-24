#ifndef FUSER_H
#define FUSER_H


#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#define MAX_LOCKS 200


struct custom_fuser_lock_info{
	char directory[256];
	pid_t owner_pid;
	int active;
};


struct fuse_thread_args {
    char mountpoint[256];
    char real_root[256];
};

struct fs_context{
	char real_root[256];
	struct custom_fuser_lock_info locks_info[MAX_LOCKS];
	pthread_mutex_t fuse_mutex;
};


void* start_fuse(void *args);

 void fullpath(char fpath[512],const char *path);
int getlock(const char *directory,pid_t pid);	
int rellock(const char *directory,pid_t pid);
 int my_getattr(const char *path,struct stat *stbuf,struct fuse_file_info *fi);
 int my_readdir(const char*path,void *buf,fuse_fill_dir_t filler,off_t offset,struct fuse_file_info *fi, enum fuse_readdir_flags flags);
 int my_open(const char *path,struct fuse_file_info *fi);
 int my_read(const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi);
 int my_release(const char *path,struct fuse_file_info *fi);
 int my_create(const char *path,mode_t mode,struct fuse_file_info *fi);
 int my_write(const char *path,const char *buf,size_t size,off_t offset,struct fuse_file_info *fi);
 int my_utimens(const char *path,const struct timespec times[2],struct fuse_file_info *fi);	
 int my_flush(const char *path,struct fuse_file_info *fi);
 int my_fsync(const char* path,int isdatasync,struct fuse_file_info *fi);
 int my_unlink(const char* path);
 int my_truncate(const char* path,off_t size,struct fuse_file_info *fi);
 int my_getxattr(const char* path,const char *name,char *value, size_t size);
 int my_chmod(const char* path,mode_t mode,struct fuse_file_info *fi);  
 int my_chown(const char* path,uid_t uid,gid_t gid,struct fuse_file_info *fi);
 int my_access(const char* path,int mask);



#endif
