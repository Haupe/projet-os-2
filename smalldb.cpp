#define PORT 28772
#define MAX_ACCEPTATION 45
#define STRING_SIZE 512

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <signal.h>
#include <pthread.h>
#include <vector>
#include <exception>

#include "db.hpp"
#include "queries.hpp"

//initialisation des structures
struct thread_fd{  // structure contenant les informations liee à un thread
	pthread_t thread;
	int fd;
};

struct QueryMutex{  // structure contenant les mutex et un compteur pour la synchronisation
	pthread_mutex_t new_access=PTHREAD_MUTEX_INITIALIZER ;
	pthread_mutex_t write_access=PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t reader_registration=PTHREAD_MUTEX_INITIALIZER;
	int readers_c = 0;
};

struct Thread_arg{ // structure contenant les arguments dont les threads ont besoin
	QueryMutex *query_mutex;
	int socket;
};

// initialisation des variables globales
database_t database;  
std::vector<thread_fd> thread_list;
int server_fd;

// definition des prototypes
void interact_with_clients(sigset_t set, sockaddr_in address, QueryMutex query_mutex);

void reading_error(int socket);
void writing_error(int socket);

int good_execution(Thread_arg *thread_arg, char* query); // fonction lancant l'execution d'une requete prenant en compte la concurrence
void *client(Thread_arg* thread_arg); // fonction permettant la discussion avec un client via le socket

// les handlers des signals
void sign_signit(int sig); 
void sign_sigusr1(int sig);

int main(int argc, char *argv[]) {  
	// *********** creation du socket pour permettre des connexions ********************
  	server_fd = socket(AF_INET, SOCK_STREAM, 0);

  	int opt = 1;
  	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
  
  	struct sockaddr_in address;
  	address.sin_family = AF_INET;
  	address.sin_addr.s_addr = INADDR_ANY;
  	address.sin_port = htons(PORT);

  	::bind(server_fd, (struct sockaddr *)&address, sizeof(address));

	// *********** mise en memoire de la database ********************
	if (argc == 2){
		db_load(&database, argv[1]);
	}else{
		db_load(&database, "students.bin");
	}

	// ************ redclaration des handlers ********************
	printf("%i\n", getpid());
	signal(SIGINT, sign_signit);
	signal(SIGUSR1, sign_sigusr1);

	// ************ initialisation des threads_mutex *************
	QueryMutex query_mutex;
	pthread_mutex_unlock(&query_mutex.new_access);	
	pthread_mutex_unlock(&query_mutex.write_access);	
	pthread_mutex_unlock(&query_mutex.reader_registration);

	// *********** creation du set de signal a bloquer pour les threads ******
	sigset_t set;
	sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGUSR1);


  	interact_with_clients( set, address, query_mutex);
  	return 0;
}


void interact_with_clients(sigset_t set, sockaddr_in address, QueryMutex query_mutex){
	while(1){  // boucle dans laquelle le processus principale boucle pour permettre des connexions

		// ici on accept et attribue a un file descriptor une connexion avec un clien
    	listen(server_fd, MAX_ACCEPTATION); 
    	size_t addrlen = sizeof(address);
    	int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
		printf("new client connected (%i)\n", new_socket);

		Thread_arg* thread_arg =new Thread_arg{&query_mutex, new_socket};  

		thread_list.push_back(thread_fd{pthread_t{}, new_socket});

		pthread_sigmask(SIG_BLOCK, &set, NULL);  // on mask certains signaux, pour s'assurer que le threads principales les recoivent
		pthread_create(&thread_list.back().thread, NULL, (void*(*)(void *)) client, (void *)thread_arg);
		pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	}
}


// ************** fonction d'erreur **************************
void reading_error(int socket){
	printf("error while reading from client %i : closing socket and thread \n", socket);
	close(socket);
}

void writing_error(int socket){
	printf("error while sending to client %i  disconnecting client \n", socket);
	close(socket);
}


// *************** fonction executer par les threads fils *********************************

int good_execution(Thread_arg *thread_arg, char* query) { 
	int result = 0;

	pthread_mutex_lock(&thread_arg->query_mutex->new_access);
	pthread_mutex_lock(&thread_arg->query_mutex->write_access);
	pthread_mutex_unlock(&thread_arg->query_mutex->new_access);
	if (strncmp("update", query, sizeof("update")-1) == 0) {
    	parse_and_execute_update(thread_arg->socket, &database, query);
		result = 1;
  	} else if (strncmp("insert", query, sizeof("insert")-1) == 0) {
    	parse_and_execute_insert(thread_arg->socket, &database, query);
		result = 1;
  	} else if (strncmp("delete", query, sizeof("delete")-1) == 0) {
    	parse_and_execute_delete(thread_arg->socket, &database, query);
		result = 1;
	}
	pthread_mutex_unlock(&thread_arg->query_mutex->write_access);
	pthread_mutex_lock(&thread_arg->query_mutex->new_access);
	pthread_mutex_lock(&thread_arg->query_mutex->reader_registration);
	if (thread_arg->query_mutex->readers_c == 0){
		pthread_mutex_lock(&thread_arg->query_mutex->write_access);
	}
	thread_arg->query_mutex->readers_c++;
	pthread_mutex_unlock(&thread_arg->query_mutex->new_access);
	pthread_mutex_unlock(&thread_arg->query_mutex->reader_registration);
	if (strncmp("select", query, sizeof("select")-1) == 0) {

		parse_and_execute_select(thread_arg->socket, &database, query);
		result = 1;
	}
	pthread_mutex_lock(&thread_arg->query_mutex->reader_registration);
	thread_arg->query_mutex->readers_c --;
	if (thread_arg->query_mutex->readers_c == 0)
		pthread_mutex_unlock(&thread_arg->query_mutex->write_access);
	pthread_mutex_unlock(&thread_arg->query_mutex->reader_registration);
	return result;
}


void *client(Thread_arg *thread_arg){
	char query[STRING_SIZE] = "";
	char end[STRING_SIZE] = "~";
	int lu=0;
	// boucle pendant laquelle le serveur reçoit des requetes du client et lui envoie le resultat
	while ((lu = read(thread_arg->socket, query, STRING_SIZE))){ 
		if (lu == -1){ // cas ou il y a eu une erreur dans le read
			reading_error(thread_arg->socket);
			return nullptr;
		}else if(lu == 2) // Si la requete est vide on sort simplement de la boucle, ça signifie que le client a fini
			break;
		for(int i = lu; i<STRING_SIZE; i++){query[i] = 0;} // ici on s'assure que le string ne contient rien d'indesirable

		if(good_execution(thread_arg, query)==0){ // le resultat de l'execution est 0, la requete n'est pas bonne
			query_fail_bad_query_type(thread_arg->socket);
		}

		if(write(thread_arg->socket, end, STRING_SIZE) == -1){ // ici on envoie au client le resultat de la requete
			writing_error(thread_arg->socket);
			return nullptr;
		}
	}
	printf("client disconnected %i (normal) : closing thread and socket \n", thread_arg->socket);
	close(thread_arg->socket);
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
	printf("Database saved \n");
}


