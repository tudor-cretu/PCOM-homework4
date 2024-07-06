#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *cookies, char *token) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));

  // Write the method name, URL, request params (if any) and protocol
  // type
  sprintf(line, "GET %s HTTP/1.1", url);
  compute_message(message, line);

  // Add the host
  sprintf(line, "Host: %s", host);
  compute_message(message, line);

  // Add cookies
  if (cookies) {
    sprintf(line, "Cookie: %s", cookies);
    compute_message(message, line);
  }

  // Add token
  if (token) {
    sprintf(line, "Authorization: Bearer %s", token);
    compute_message(message, line);
  }

  // Add final new line
  compute_message(message, "");
  free(line);

  return message;
}


char *compute_post_request(char *host, char *url, char* content_type, 
                                            char *target, char *jwt) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // writes the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // adds the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    //adds the target in authorization
    if (jwt != NULL) {
        sprintf(line, "Authorization: Bearer %s", jwt);
        compute_message(message, line);
    }

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    // adds payload length
    int length = strlen(target);
    sprintf(line, "Content-Length: %d", length);
    compute_message(message, line);

    compute_message(message, "");   

    // adds payload
    memset(line, 0, LINELEN);
    strcat(message, target);


    free(line);
    return message;
}

char *compute_delete_request(const char *host, const char *url, 
                                    char *cookies, char *token) {
	char *message = (char *)calloc(BUFLEN, sizeof(char));
	char *line = (char *)calloc(LINELEN, sizeof(char));

	sprintf(line, "DELETE %s HTTP/1.1", url);

	compute_message(message, line);

	/* Add the host */
	sprintf(line, "Host: %s", host);
	compute_message(message, line);

	/* Add headers and/or cookies, according to the protocol format */
	if (cookies) {
		memset(line, 0, LINELEN);
		sprintf(line, "Cookie: %s", cookies);

		compute_message(message, line);
	}

	if (token) {
		sprintf(line, "Authorization: Bearer %s", token);
		compute_message(message, line);
	}

	/* Add final new line */
	compute_message(message, "");
	free(line);

	return message;
}
