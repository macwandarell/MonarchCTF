#include "fuser.h"


struct custom_fuser_lock_info locks_info[MAX_LOCKS];
pthread_mutex_t fuse_mutex=PTHREAD_MUTEX_INITIALIZER;
 struct fuse_operations ops={
	.getattr=my_getattr,
	.readdir=my_readdir,
	.open=my_open,
	.read=my_read,
	.release=my_release,
	.create=my_create,
	.write=my_write,
	.utimens=my_utimens,
	.flush=my_flush,
	.fsync=my_fsync,
	.unlink=my_unlink,
	.truncate=my_truncate,
	.getxattr=my_getxattr,
	.access=my_access,
	.chown=my_chown,
	.chmod=my_chmod,
	
};

 void fullpath(char fpath[512],const char *path){
	struct fs_context *ctx=fuse_get_context()->private_data;
	snprintf(fpath,512,"%s%s",ctx->real_root,path);}

int getlock(const char *directory,pid_t pid){
	struct fs_context *ctx=fuse_get_context()->private_data;
	pthread_mutex_lock(&ctx->fuse_mutex);
	for(int i=0;i<MAX_LOCKS;i++){
			if(ctx->locks_info[i].active&&strcmp(ctx->locks_info[i].directory,directory)==0&&ctx->locks_info[i].owner_pid!=pid){
	pthread_mutex_unlock(&ctx->fuse_mutex);
	return -1;}
	
else if(ctx->locks_info[i].active&&strcmp(ctx->locks_info[i].directory,directory)==0&&ctx->locks_info[i].owner_pid==pid){
	pthread_mutex_unlock(&ctx->fuse_mutex);
	return 0;}}
	
	for(int i=0;i<MAX_LOCKS;i++){
	if(!ctx->locks_info[i].active){
		strcpy(ctx->locks_info[i].directory,directory);
		ctx->locks_info[i].owner_pid=pid;
		ctx->locks_info[i].active=1;
		break;
	}}
	pthread_mutex_unlock(&ctx->fuse_mutex);
	return 0;
}

int rellock(const char *directory,pid_t pid){
	struct fs_context *ctx=fuse_get_context()->private_data;
	pthread_mutex_lock(&ctx->fuse_mutex);
	for(int i=0;i<MAX_LOCKS;i++){
		if(ctx->locks_info[i].active && strcmp(ctx->locks_info[i].directory,directory)==0){
		ctx->locks_info[i].active=0;
		break;}}
	pthread_mutex_unlock(&ctx->fuse_mutex);
	return 0;
}
	
	
 int my_getattr(const char *path,struct stat *stbuf,struct fuse_file_info *fi){
	struct fs_context *ctx=fuse_get_context()->private_data;
	pid_t pid=fuse_get_context()->pid;
	(void) fi;
	

	fflush(stdout);
	memset(stbuf,0,sizeof(struct stat));
	
	//root
	if(strcmp(path,"/")==0){
	stbuf->st_mode=S_IFDIR |0755;
	stbuf->st_nlink=2;
	return 0;}
	
	char fpath[512];
	fullpath(fpath,path);
	printf("[DEBUG] path=%s real=%s\n",path,fpath);
	if(lstat(fpath,stbuf)==-1){return -errno;}
	return 0;
	}


 int my_readdir(const char*path,void *buf,fuse_fill_dir_t filler,off_t offset,struct fuse_file_info *fi, enum fuse_readdir_flags flags){
	printf("[DEBUG] readdir path: %s\n",path);
	pid_t pid=fuse_get_context()->pid;
	(void) offset;
	(void) fi;
	(void) flags;
	fflush(stdout);
	struct fs_context *ctx=fuse_get_context()->private_data;
	char fpath[512];
	fullpath(fpath,path);
	printf("[DEBUG] mapping %s->%s\n",path,fpath);
	fflush(stdout);
	DIR *dp=opendir(fpath);
	if(!dp){return -errno;}
	struct dirent *de;
	filler(buf,".",NULL,0,0);
	filler(buf,"..",NULL,0,0);
	while((de=readdir(dp))!=NULL){
		struct stat st;
		memset(&st,0,sizeof(st));
		st.st_ino=de->d_ino;
		st.st_mode=de->d_type<<12;
		filler(buf,de->d_name,&st,0,0);}
	closedir(dp);
	return 0;}

 int my_open(const char *path,struct fuse_file_info *fi){
	struct fs_context *ctx=fuse_get_context()->private_data;
	char fpath[512];
	pid_t pid=fuse_get_context()->pid;
	fullpath(fpath,path);
	if(getlock(path,pid)==-1){return -EACCES;}
	printf("[DEBUG] open map: %s->%s\n",path,fpath);
	fflush(stdout);
	int fd=open(fpath,fi->flags);
	if(fd==-1){return -errno;}
	fi->fh=fd;
	return 0;}

 int my_read(const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi){
	(void) path;
	int fd=fi->fh;
	pid_t pid=fuse_get_context()->pid;
	printf("[DEBUG] read: fd=%d offset=%ld size=%zu\n",fd,offset,size);
	fflush(stdout);
	int res=pread(fd,buf,size,offset);
	if(res==-1){return -errno;}
	return res;}

 int my_release(const char *path,struct fuse_file_info *fi){
	(void) path;
	printf("[DEBUG] fd=%ld\n",fi->fh);
	pid_t pid=fuse_get_context()->pid;
	fflush(stdout);
	rellock(path,pid);
	close(fi->fh);
	return 0;}
	
 int my_create(const char *path,mode_t mode,struct fuse_file_info *fi){
	struct fs_context *ctx=fuse_get_context()->private_data;
	char fpath[512];
	fullpath(fpath,path);
	pid_t pid=fuse_get_context()->pid;
	if(getlock(path, pid)==-1){return -EACCES;}
	printf("[DEBUG] create mapping %s->%s\n",path,fpath);
	fflush(stdout);
	int fd=open(fpath,O_WRONLY|O_CREAT|O_TRUNC,mode);
	if(fd==-1){rellock(path,pid);return -errno;}
	fi->fh=fd;
	return 0;}

 int my_write(const char *path,const char *buf,size_t size,off_t offset,struct fuse_file_info *fi){
	(void) path;
	int fd=fi->fh;
	pid_t pid=fuse_get_context()->pid;
	printf("[DEBUG] write : fd=%d size=%zu offset=%ld\n",fd,size,offset);
	fflush(stdout);
	int res=pwrite(fd,buf,size,offset);
	if(res==-1){
	return -errno;}
	return res;
}

 int my_utimens(const char *path,const struct timespec times[2],struct fuse_file_info *fi){
	(void) fi;
	pid_t pid=fuse_get_context()->pid;
	struct fs_context *ctx = fuse_get_context()->private_data;
	char fpath[512];
	fullpath(fpath,path);
	printf("[DEBUG] utimens: %s->%s\n",path,fpath);
	fflush(stdout);
	int res=utimensat(0,fpath,times,0);
	if(res==-1){return -errno;}
	return 0;}
	
 int my_flush(const char *path,struct fuse_file_info *fi){
    (void) path;
    (void) fi;
    return 0;
}

 int my_fsync(const char* path,int isdatasync,struct fuse_file_info *fi){
	(void) path;
	int res;
	if(isdatasync){res=fdatasync(fi->fh);}
	else{res=fsync(fi->fh);}
	if(res==-1){return -errno;}
	return 0;}


 int my_unlink(const char* path){
	char fpath[512];
	fullpath(fpath,path);
	int res=unlink(fpath);
	if(res==-1){return -errno;}
	return 0;}

 int my_truncate(const char* path,off_t size,struct fuse_file_info *fi){
	char fpath[512];
	fullpath(fpath,path);
	
	int res;
	if(fi!=NULL){res=ftruncate(fi->fh,size);}
	else{res=truncate(fpath,size);}
	if(res==-1){return -errno;}
	return 0;}


 int my_getxattr(const char* path,const char *name,char *value, size_t size){
    (void) path;
    (void) name;
    (void) value;
    (void) size;
    return -ENODATA;}

 int my_chmod(const char* path,mode_t mode,struct fuse_file_info *fi){
    char fpath[512];
    fullpath(fpath,path);
    int res=chmod(fpath,mode);
    if (res == -1){return -errno;}
    return 0;}
   
 int my_chown(const char* path,uid_t uid,gid_t gid,struct fuse_file_info *fi){
	char fpath[512];
	fullpath(fpath,path);
	int res=lchown(fpath,uid,gid);
	if(res==-1){return -errno;}
	return 0;}

 int my_access(const char* path,int mask){
	char fpath[512];
	fullpath(fpath,path);
	int res=access(fpath,mask);
	if(res==-1){return -errno;}
	return 0;}


void* start_fuse(void* args){
	struct fuse_thread_args* fargs=(struct fuse_thread_args*)args;
	struct fs_context *ctx=malloc(sizeof(struct fs_context));
	strncpy(ctx->real_root,fargs->real_root,256);
	char *argv[]={"fuse_exe","-f","-o","allow_other",fargs->mountpoint,NULL};
	int argc=5;
	fuse_main(argc,argv,&ops,ctx);
	free(fargs);
	return NULL;
}
