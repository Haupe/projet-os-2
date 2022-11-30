#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <signal.h>
#include <pthread.h>
#include <vector>

#include "db.hpp"

database_t database;


void *client(int socket);

int main() {
	// *********** creation du socket pour permettre des connexions ********************
  	int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  	int opt = 1;
  	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
  
  	struct sockaddr_in address;
  	address.sin_family = AF_INET;
  	address.sin_addr.s_addr = INADDR_ANY;
  	address.sin_port = htons(8080);

  	bind(server_fd, (struct sockaddr *)&address, sizeof(address));

	// *********** mise en memoire de la database ********************
	db_load(&database, "students.zip");

	// *********** creation du set de signal a bloquer pour les threads ******
	sigset_t set;
	sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGUSR1);


	std::vector<pthread_t> thread_list;
  	while(1){  // boucle dans laquelle le processus principale boucle pour permettre des connexions
    	listen(server_fd, 3);
    	size_t addrlen = sizeof(address);
    	int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
		thread_list.push_back(pthread_t{});
		pthread_sigmask(SIG_BLOCK, &set, NULL);
		pthread_create(&thread_list.back(), NULL, (void*(*)(void *)) client, (void *)new_socket);
		pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	}
  	return 0;
}


void *client(int socket){
	return nullptr;
}


