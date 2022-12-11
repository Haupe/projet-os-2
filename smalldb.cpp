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
#include "queries.hpp"

struct thread_fd{
	pthread_t thread;
	int fd;
};

database_t database;  // en global parce qu'il faut que les threads y aie acces, pas trouver de meilleur solution
std::vector<thread_fd> thread_list;
int server_fd;


void *client(int socket);

// les handlers des signals
void sign_signit(int sig); 
void sign_sigusr1(int sig);

int main() {  // manque la gestion des deconnexions
	// *********** creation du socket pour permettre des connexions ********************
  	server_fd = socket(AF_INET, SOCK_STREAM, 0);

  	int opt = 1;
  	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
  
  	struct sockaddr_in address;
  	address.sin_family = AF_INET;
  	address.sin_addr.s_addr = INADDR_ANY;
  	address.sin_port = htons(28772);

  	::bind(server_fd, (struct sockaddr *)&address, sizeof(address));

	// *********** mise en memoire de la database ********************
	db_load(&database, "students.bin");

	// ************ redclaration des handlers ********************
	signal(SIGINT, sign_signit);
	signal(SIGUSR1, sign_sigusr1);

	// *********** creation du set de signal a bloquer pour les threads ******
	sigset_t set;
	sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGUSR1);


  	while(1){  // boucle dans laquelle le processus principale boucle pour permettre des connexions
    	listen(server_fd, 45);
    	size_t addrlen = sizeof(address);
    	int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
		printf("client connected \n");

		thread_list.push_back(thread_fd{pthread_t{}, new_socket});

		pthread_sigmask(SIG_BLOCK, &set, NULL);
		pthread_create(&thread_list.back().thread, NULL, (void*(*)(void *)) client, (void *)new_socket);
		pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	}
  	return 0;
}

// *************** fonction executer par les threads fils *********************************

void *client(int socket){
	char query[256] = "";
	char end[256] = "~";
	int lu;
	while ((lu = read(socket, query, 256)) > 0){
		for(int i = lu; i<256; i++){query[i] = 0;}
		printf("executing :%s \n", query);
		parse_and_execute(socket, &database, query);
		write(socket, end, 256);
	}
	printf("client disconnected \n");
	close(socket);
	return nullptr;
}


// ************************ handler de signaux *********************************
void sign_signit(int sig){
	if(sig == SIGINT){
		for(auto &thread_fd: thread_list){
			pthread_join(thread_fd.thread, NULL);
			close(thread_fd.fd);
		}
		close(server_fd);
		db_save(&database);
		printf("\nfermeture de la base de donne realise, bye bye ! \n");
		exit(0);
	}
}
void sign_sigusr1(int sig){
	if(sig == SIGUSR1){
		db_save(&database);
	}
}


