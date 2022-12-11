#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <signal.h>
#include <string>
#include <iostream>

std::string lecture(int longueur, char* buffer, int sock);

int main(void) {
  // Permet que write() retourne 0 en cas de réception
  // du signal SIGPIPE.
  signal(SIGPIPE, SIG_IGN);
  
  int sock = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(28772);

  // Conversion de string vers IPv4 ou IPv6 en binaire
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

  connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  printf("Your're connected to the database !\nPlease enter a query :\n>");
  
   char buffer[256] = "";
   std::string query="";
   int longueur, i, ret;
   while (fgets(buffer, 256, stdin) != NULL) {
      longueur = strlen(buffer) + 1;
      query = buffer;
      printf("Envoi...\n");
      if(static_cast<std::string>(query).find("\n")){
         
			query.erase(query.length()-1);
		}
      //printf("here\n");
      write(sock, query.c_str(), strlen(buffer) + 1);
      i = 0;
      std::string result = "";
      for(int i=0; i<256; i++ ){buffer[i]=0;}
      int counter = 0;
      std::string save = "not first time";
      while (result.find("~") == std::string::npos) {
         /* if(result == save){ // pour etre safe
            counter ++;
            if (counter > 2)
               break;
         } */
         save = result;
         result += lecture(longueur, buffer, sock);
         //printf("%s \n", result.c_str());

      }
      printf("Recu : %s\n>", result.c_str());
   }
  
  close(sock);
  return 0;
}

std::string lecture(int longueur, char* buffer, int sock){
   int ret, i=0;
   std::string result="";
   while (i < longueur) {
      ret = read(sock, buffer, longueur-1);
      result += buffer;
      i += ret;
   }
   return result.c_str();
}