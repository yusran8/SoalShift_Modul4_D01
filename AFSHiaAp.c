#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#define key 17

static const char *dirpath = "/home/yusran/shift4";
const char crypt[150]="qE1~ YMUR2\"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0";

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
	char fpath[1000];
	int a=strlen(path);
	char encrypt[a];

	strcpy(encrypt, path);
	if(strcmp(encrypt,".")!=0&&strcmp(encrypt, "..")!=0){
	for(int i=0; i<strlen(encrypt); i++){
		if(encrypt[i]=='/') continue;
		else{
			int j=0, k=0;
			while(encrypt[i]!=crypt[j]) j++;
			k=(j+key)%strlen(crypt);
			encrypt[i]=crypt[k];
		}
	}
	}
	

	sprintf(fpath,"%s%s",dirpath,encrypt);
	
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
  char decrypt[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else{ 
		int a=strlen(path);
		char encrypt[a];
		strcpy(encrypt, path);
		if(strcmp(encrypt, ".")!=0&&strcmp(encrypt, "..")!=0){
		for (int i=0;i<strlen(encrypt);i++){
			if(encrypt[i]=='/') continue;
			else{
				int j=0, k=0;
				while(decrypt[i]!=crypt[j]) j++;
				k=(j+key)%strlen(crypt);
				encrypt[i]=crypt[k];
			}
		}
		}

		sprintf(fpath, "%s%s",dirpath,encrypt);
	}
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;
	
	
	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;

		strcpy(decrypt, de->d_name);
		if(strcmp(decrypt,".")!=0&&strcmp(decrypt,"..")!=0){
		for(int i=0; i<strlen(decrypt);i++){
			if(decrypt[i]=='/') continue;
			else{
				int j=0, k=0;
				while(decrypt[i]!=crypt[j]) j++;
				k=(j+strlen(crypt)-key);
				decrypt[i]=crypt[k];
			}
		}
		}
		
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
  char fpath[1000];
  int c=strlen(path);
  char encrypt[c];

	strcpy(encrypt, path);
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else{
		for(int i=0;i<c;i++){
			if(encrypt[i]=='/') continue;
			else {
				int j=0,k=0;
				while(encrypt[i]!=crypt[j])j++;
				k=(j+key)%strlen(crypt);
				encrypt[i]=crypt[k];
			}
		}
	}
	 sprintf(fpath, "%s%s",dirpath,encrypt);
	int res = 0;
  int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
