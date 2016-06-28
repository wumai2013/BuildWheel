#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <signal.h>

#define RESPONSE "HTTP/1.1 200 OK\r\n"	\
		 "Server: CTG.HAZE.C\r\n"	\
		 "Date: Fri, 24 Jun 2016 17:19:59 GMT\r\n"	\
		 "Content-Type: text/html\r\n"	\
		 "Content-Length: 10\r\n"	\
                 "Last-Modified: Fri, 24 Jun 2016 17:19:54 GMT\r\n" \
	         "Connection: keep-alive\r\n"	\
		 "Accept-Ranges: bytes\r\n"	\
		 "\r\n"		\
		 "html"

#define REQUEST "GET / HTTP/1.1\r\n"	\
		"Host: 192.168.119.150:81\r\n"	\
		"User-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64; rv:47.0) Gecko/20100101 Firefox/47.0\r\n"	\
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"	\
		"Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3\r\n"	\
		"Accept-Encoding: gzip, deflate\r\n"	\
		"Connection: keep-alive\r\n"	\
		"Pragma: no-cache\r\n"	\
		"Cache-Control: no-cache\r\n"

struct ctg_buf{
	char *argu;
	int argv;
};

struct ctg_buf com = {NULL ,0};
void scan_chars(const char *msg ,int len)
{
	char *pos ,*end ,*cur;
	char show[100] =  {0};

	cur = pos = msg;
	end = msg + len;
	while(cur != end){
		if(*cur == '\r'){
			com.argu = pos;
			com.argv = cur-pos;	
			memset(show ,0x00 ,sizeof(show));
			strncpy(show ,com.argu ,com.argv);
			pos = cur + 2;
			printf("com:%s\n" ,show);
		}
		cur++;
	}
}
#define show_error() do{	\
	printf("error:<%s> code:[%d]\n" ,strerror(errno) ,errno);	\
}while(0)

int srv_fd = 0;
void signal_handler(int sig)
{
	if(srv_fd > 0){
		close(srv_fd);
	}
}

void set_signals(void)
{
	siginterrupt(SIGINT,1);
	signal(SIGINT,signal_handler);
	siginterrupt(SIGTERM,1);
	signal(SIGTERM,signal_handler);
	signal(SIGPIPE,SIG_IGN);
}
                                                                                         
int main(void)
{	
	int ret ,val;
	int cli_fd = 0;
	socklen_t size = sizeof(struct sockaddr_in);
	struct sockaddr_in srv_addr ,client_addr;
	char buf[1024] = {0};
	extern char **environ;

	scan_chars(REQUEST ,sizeof(REQUEST)-1);
	return 0;
	//set signal
	set_signals();

	memset(&srv_addr ,0x00 ,sizeof(struct sockaddr_in));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(81);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*int socket(int domain, int type, int protocol);*/
	srv_fd = socket(AF_INET ,SOCK_STREAM ,0);
	if(srv_fd < 0){
		show_error();
		goto EXIT;
	}

	val = 1;
	if (setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		fprintf(stderr, "spawn-fcgi: couldn't set SO_REUSEADDR: %s\n", strerror(errno));
		close(srv_fd);
		return -1;
	}
	/*
	 * 	 int bind(int sockfd, const struct sockaddr *addr,
	 * 	                 socklen_t addrlen);
	 * 	*/
	ret = bind(srv_fd ,(struct sockaddr *)&srv_addr ,sizeof(struct sockaddr_in));
	if(ret < 0){
		show_error();
		goto EXIT;		
	}

	ret = listen(srv_fd ,5);
	if(ret < 0){
		show_error();
		goto EXIT;
	}

	/*int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);*/
	while(1){
		cli_fd = accept(srv_fd ,(struct sockaddr *)&srv_addr ,&size);
		if(cli_fd < 0){
			show_error();
			goto EXIT;
		}

		/*ssize_t read(int fd, void *buf, size_t count);*/
try_again:
		ret = read(cli_fd ,buf ,sizeof(buf));
		if(ret == EAGAIN){
			goto try_again;
		}else if(ret < 0){
			goto EXIT;
		}else if(ret > 0){
			printf("recv buf len<%d>:\n%s\n" ,strlen(buf),buf);
		}
		write(cli_fd ,RESPONSE ,strlen(RESPONSE));
		if(cli_fd > 0){
			close(cli_fd);
		}
	}
EXIT:
	if(srv_fd){
		close(srv_fd);
	}
	return 0;
}
