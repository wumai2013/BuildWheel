#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
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

#define show_error() do{        \
	printf("error:<%s> code:[%d]\n" ,strerror(errno) ,errno);       \
}while(0)

void setnonblocking(int sock)
{       
	int opts;
	opts = fcntl(sock, F_GETFL);
	if(opts < 0)
	{       
		perror("fcntl(sock,GETFL)");
		exit(1);
	}
	opts = opts | O_NONBLOCK;
	if(fcntl(sock, F_SETFL, opts) < 0)
	{
		perror("fcntl(sock,SETFL,opts)");
		exit(1);
	}
}
int main(int argc, char **argv)
{
	int ret ,val;
	int srv_fd ,cli_fd = 0;
	socklen_t size = sizeof(struct sockaddr_in);
	struct sockaddr_in srv_addr ,client_addr;
	char buf[1024] = {0};

	int epfd=0, nfds=0, i=0;
	struct epoll_event ev;
	struct epoll_event events[256];

	memset(&srv_addr ,0x00 ,sizeof(struct sockaddr_in));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(80);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	srv_fd = socket(AF_INET ,SOCK_STREAM ,0);
	val = 1;
	if (setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		fprintf(stderr, "spawn-fcgi: couldn't set SO_REUSEADDR: %s\n", strerror(errno));
		close(srv_fd);
		return -1;
	}
	setnonblocking(srv_fd);
	/*
	 *          *       int bind(int sockfd, const struct sockaddr *addr,
	 *                   *                       socklen_t addrlen);
	 *                            *      */
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


	epfd = epoll_create(256);
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = srv_fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, srv_fd, &ev) < 0) {
		goto EXIT;
	}

	while (1){
		nfds=epoll_wait(epfd,events,256,-1);
		for(i=0;i<nfds;++i){
			if (events[i].data.fd == srv_fd){
				cli_fd = accept(srv_fd ,(struct sockaddr *)&client_addr ,&size);
				if(cli_fd < 0){
					show_error();
					goto EXIT;
				}
				ev.events = EPOLLIN | EPOLLET;    
				ev.data.fd = cli_fd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, cli_fd, &ev);
			}else if(events[i].events & EPOLLIN){
				if(events[i].data.fd != cli_fd){
					continue;
				}
				ret = read(events[i].data.fd ,buf ,sizeof(buf)-1);
				if(ret == EAGAIN){
					continue;
				}
				printf("buf:\n<%s>\n" ,buf);
				write(events[i].data.fd ,RESPONSE ,strlen(RESPONSE));
				if(events[i].data.fd > 0){
					close(events[i].data.fd);
				}
				epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
			}
		}
	}
EXIT:
	if(srv_fd){
		close(srv_fd);
	}
	return 0;
}
