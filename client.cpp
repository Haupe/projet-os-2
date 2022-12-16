#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <signal.h>
#include <string>
#include <iostream>

std::string lecture(int longueur, int sock);  // sert a effectuer une lecture propre sur le socket
void interact(int sock);  // sert a effectuer une discussion entre l'utilisateur et le serveur

int main(int argc, char* argv[]) {
   // Permet que write() retourne 0 en cas de réception
   // du signal SIGPIPE.
   signal(SIGPIPE, SIG_IGN);
  
   int sock =0;
   if((sock=socket(AF_INET, SOCK_STREAM, 0)) == -1){
      perror("Unable to open a socket\n");
      exit(0);
   }

   struct sockaddr_in serv_addr;
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(28772);

   // Conversion de string vers IPv4 ou IPv6 en binaire
   if (argc == 2) {
      inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
   }else{
      inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
   }


   // connect etablis la connexion avec le server
   if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
      perror("unable to connect to a server\n");
      exit(0);
   }

   interact(sock);
  
   close(sock);
   return 0;
}

void interact(int sock){
   int longueur = 512; // contient la longueur des char*
   char buffer[longueur] = "";  // contient l'entree du client
   std::string query="";  // contient l'entrée du client sans le \n
   int end_index=0; // contient l'indice du charactere ~ dans le reponse
   bool flag;  // sert a sortir de la boucle quand il le faut

   // Boucle pendant laquelle le client prends des entres de l'utilisateur, l'envoie et imprime la réponse.
   printf(">");
   while (fgets(buffer, longueur, stdin) != NULL) {  // ici on lit sur l'entree du client
      if (!strcmp(buffer, "")){ // on verifie si il est vide
         break;
      }

      query = buffer; // ces 4 lignes servent a retirer le \n pour pas de soucis 
      if (query.find("\n")){
			query.erase(query.length()-1);
		}

      write(sock, query.c_str(), strlen(buffer) + 1);  // ici on envoie l'entree de l'utilisateur au serveur
      std::string result = "";
      flag = true;
      while (flag) { // ici il faut boucler car le serveur peut envoyer un nombre variable de reponses
         result = lecture(longueur, sock);  // on lit un element du serveur
         if((end_index=result.find("~")) != std::string::npos){ 
            // si le charactere ~ est present dans le "message" du serveur, c'est la fin de la reponse
            result.erase(end_index);
            flag = false;
         }
         printf("%s \n", result.c_str());
      }
      printf(">");
   }
}

std::string lecture(int longueur, int sock){
   int ret, i=0;
   std::string result="";
   char buffer[longueur] = "";
   while (i < longueur) {
      for(int i=0; i<longueur; i++ ){buffer[i]=0;}
      ret = read(sock, buffer, longueur-1);
      result += buffer;
      i += ret;
   }
   return result.c_str();
}

