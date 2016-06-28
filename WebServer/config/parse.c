#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

void web4c_Breakup_chars(const char *msg ,int len)
{
        char *pos ,*end ,*cur;
        char show[100] =  {0};

        cur = pos = msg;
        end = msg + len;
        while(cur != end){
                if(*cur == '\n'){
                        bzero(show ,sizeof(show)-1);
			strncpy(show ,pos ,cur-pos+1);
                        pos = cur;
                       	puts(show); 
                }
                cur++;
        }
}

void web4c_config_parse(const char *path)
{
	int config_fd = 0;
	struct stat st;
	char *tmpbuf = NULL;
	int index = 0;

	config_fd = open(path ,O_RDWR ,644);
	if(config_fd < 0){
		perror("open failed");
		return;
	}
	
	memset(&st ,0x00 ,sizeof(st));
	 fstat(config_fd, &st); 
	/*void *mmap(void *start,size_t length,int prot,int flags,int fd,off_t offsize);*/
	tmpbuf = (char *)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, config_fd, 0); 
//	web4c_Breakup_chars(tmpbuf ,st.st_size);
	while(index < st.st_size){
		printf("%c" ,*tmpbuf++);
		index++;
	}
	close(config_fd);
}
int main(void)
{
	web4c_config_parse("web4c.config");
	return 0;	
}
