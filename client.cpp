#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <signal.h>
#include <string>
#include <iostream>

std::string lecture(int longueur, int sock);

int main(int argc, char* argv[]) {
  // Permet que write() retourne 0 en cas de réception
  // du signal SIGPIPE.
  signal(SIGPIPE, SIG_IGN);
  
  int sock = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(28772);

  // Conversion de string vers IPv4 ou IPv6 en binaire
   if (argc == 2) {
      inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
   }else{
      inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
   }

  connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  
   char buffer[512] = "";
   std::string query="";
   int end_index=0;
   int longueur;
   bool flag;

   while (fgets(buffer, 512, stdin) != NULL) {
      if (!strcmp(buffer, "")){
         break;
      }
      longueur = 512;
      query = buffer;
      if(static_cast<std::string>(query).find("\n")){
         
			query.erase(query.length()-1);
		}
      write(sock, query.c_str(), strlen(buffer) + 1);
      std::string result = "";
      flag = true;
      while (flag) {
         result = lecture(longueur, sock);
         if((end_index=result.find("~")) != std::string::npos){
            result.erase(end_index);
            flag = false;
         }
         printf("%s \n", result.c_str());
      }
      printf("\n>");
   }
  
  close(sock);
  return 0;
}

std::string lecture(int longueur, int sock){
   int ret, i=0;
   std::string result="";
   char buffer[512] = "";
   while (i < longueur) {
      for(int i=0; i<512; i++ ){buffer[i]=0;}
      ret = read(sock, buffer, longueur-1);
      result += buffer;
      i += ret;
   }
   return result.c_str();
}