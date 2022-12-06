#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <signal.h>
#include <string>


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
  
  char buffer[256];
  int longueur, i, ret;
  while (fgets(buffer, 256, stdin) != NULL) {
     longueur = strlen(buffer) + 1;
     printf("Envoi...\n");
      std::string query = buffer; 
     if(static_cast<std::string>(query).find("\n")){
			query.erase(query.length()-1);
		}
     write(sock, query.c_str(), strlen(buffer) + 1);
     i = 0;
     std::string result;
     while (read(sock, buffer, longueur - i) > -1) {
        if (!strcmp(buffer, ""))
            break;
        result += buffer;
     }
     
     printf("Recu : %s\n>", result.c_str());
  }
  
  close(sock);
  return 0;
}