#include <stdio.h>      /* printf, sprintf */
#include <stdbool.h>    /* bool */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <ctype.h>      /* isdigit */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>  

#include "requests.h"   /* custom header for HTTP requests */
#include "helpers.h"
#include "parson.h"     /* JSON parsing library */

#define HOST "34.246.184.49:8080"
#define IP "34.246.184.49"
#define PORT 8080
#define REGISTER_ACCESS "/api/v1/tema/auth/register"
#define LOGIN_ACCESS "/api/v1/tema/auth/login"
#define LIBRARY_ACCESS "/api/v1/tema/library/access"
#define BOOKS_ACCESS "/api/v1/tema/library/books"
#define LOGOUT_ACCESS "/api/v1/tema/auth/logout"
#define PAYLOAD_TYPE "application/json"

/**
 * @brief Handles user registration by collecting username and password,
 *        creating a JSON object, and sending it to the server.
 *
 * @param sockfd The socket file descriptor.
 */
void registration(int sockfd) {
    char *username = malloc(sizeof(char) * LINELEN);
    char *password = malloc(sizeof(char) * LINELEN);

    if (username == NULL || password == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    /* Consume newline character (from previous input) */
    fgets(username, LINELEN - 1, stdin);

    printf("username=");
    fgets(username, LINELEN - 1, stdin);
    username[strlen(username) - 1] = '\0';

    printf("password=");
    fgets(password, LINELEN - 1, stdin);
    password[strlen(password) - 1] = '\0';

    if (strchr(username, ' ')) {
        printf("Error: Username cannot contain spaces. Please try again.\n");
        free(username);
        free(password);
        return;
    }

    if (strchr(password, ' ')) {
        printf("Error: Password cannot contain spaces. Please try again.\n");
        free(username);
        free(password);
        return;
    }

    /* Create JSON object with username and password */
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    json_object_set_string(obj, "username", username);
    json_object_set_string(obj, "password", password);

    /* Serialize JSON object to string */
    char *json_string = json_serialize_to_string_pretty(val);
    char *message = compute_post_request(HOST, REGISTER_ACCESS, PAYLOAD_TYPE, json_string, NULL);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        printf("Error: Username already taken. Please try again.\n");
    } else {
        printf("User registered successfully.\n");
    }

    free(username);
    free(password);
    json_value_free(val);
    free(message);
    free(response);
}

/**
 * @brief Handles user login by collecting username and password,
 *        creating a JSON object, and sending it to the server.
 *
 * @param sockfd The socket file descriptor.
 * @return The session cookie if login is successful, NULL otherwise.
 */
char *login(int sockfd) {
    char *username = malloc(sizeof(char) * LINELEN);
    char *password = malloc(sizeof(char) * LINELEN);
    char *cookie = NULL;

    if (username == NULL || password == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    /* Consume newline character (from previous input) */
    fgets(username, LINELEN - 1, stdin);

    printf("username=");
    fgets(username, LINELEN - 1, stdin);
    username[strlen(username) - 1] = '\0';

    printf("password=");
    fgets(password, LINELEN - 1, stdin);
    password[strlen(password) - 1] = '\0';

    if (strchr(username, ' ')) {
        printf("Error: Username cannot contain spaces. Please try again.\n");
        free(username);
        free(password);
        return NULL;
    }

    if (strchr(password, ' ')) {
        printf("Error: Password cannot contain spaces. Please try again.\n");
        free(username);
        free(password);
        return NULL;
    }

    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    json_object_set_string(obj, "username", username);
    json_object_set_string(obj, "password", password);

    char *json_string = json_serialize_to_string_pretty(val);
    char *message = compute_post_request(HOST, LOGIN_ACCESS, PAYLOAD_TYPE, json_string, NULL);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *error = strstr(response, "error");

    if (error) {
        if (strstr(error, "Credentials")) {
            printf("Error: Invalid username or password. Please try again.\n");
        } else {
            printf("Error: No account with this username. Please register first.\n");
        }
    } else {
        printf("User logged in successfully.\n");

        /* Extract session cookie from response */
        cookie = strstr(response, "Set-Cookie: ");
        if (cookie) {
            cookie = strstr(cookie, "connect.sid=");
            cookie = strtok(cookie, ";");
            /* Duplicate the cookie string to manage memory correctly */
            cookie = strdup(cookie); 
        }
    }

    free(username);
    free(password);
    json_value_free(val);
    free(message);
    free(response);

    return cookie;
}

/**
 * @brief Enters the library by sending a GET request with the session cookie.
 *
 * @param sockfd The socket file descriptor.
 * @param cookie The session cookie.
 * @return The JWT token if successful, NULL otherwise.
 */
char *enter_library(int sockfd, char *cookie) {
    char *message = compute_get_request(HOST, LIBRARY_ACCESS, cookie, NULL);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    /* Extract token from response */
    char *token = strstr(response, "token");
    if (token) {
        token = strtok(token, "\"");
        token = strtok(NULL, "\"");
        token = strtok(NULL, "\"");
    }

    free(message);
    free(response);

    if (token) {
        /* Duplicate the token string to manage memory correctly */
        char *jwt = malloc(strlen(token) + 1);
        if (jwt) {
            strcpy(jwt, token);
            printf("User entered the library successfully.\n");
            return jwt;
        }
    }
    printf("Error: Failed to enter the library.\n");
    return NULL;
}

/**
 * @brief Retrieves the list of books from the library.
 *
 * @param sockfd The socket file descriptor.
 * @param jwt The JWT token.
 */
void get_books(int sockfd, char *jwt) {
    char *message = compute_get_request(HOST, BOOKS_ACCESS, NULL, jwt);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        printf("Error: Failed to get books\n");
    } else {
        char *books = strstr(response, "[");
        if (books) {
            puts(books);
        }
    }

    free(message);
    free(response);
}

/**
 * @brief Adds a new book to the library by collecting book details,
 *        creating a JSON object, and sending it to the server.
 *
 * @param sockfd The socket file descriptor.
 * @param jwt The JWT token.
 */
void add_book(int sockfd, char *jwt) {
    char *title = malloc(sizeof(char) * LINELEN);
    char *author = malloc(sizeof(char) * LINELEN);
    char *genre = malloc(sizeof(char) * LINELEN);
    char *publisher = malloc(sizeof(char) * LINELEN);
    char *page_count = malloc(sizeof(char) * LINELEN);

    if (!title || !author || !genre || !publisher || !page_count) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    /* Consume newline character (from previous input) */
    fgets(title, LINELEN - 1, stdin);

    printf("title=");
    fgets(title, LINELEN - 1, stdin);
    title[strlen(title) - 1] = '\0';

    printf("author=");
    fgets(author, LINELEN - 1, stdin);
    author[strlen(author) - 1] = '\0';

    printf("genre=");
    fgets(genre, LINELEN - 1, stdin);
    genre[strlen(genre) - 1] = '\0';

    printf("publisher=");
    fgets(publisher, LINELEN - 1, stdin);
    publisher[strlen(publisher) - 1] = '\0';

    printf("page_count=");
    fgets(page_count, LINELEN - 1, stdin);
    page_count[strlen(page_count) - 1] = '\0';

    if (strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 || strlen(publisher) == 0 || strlen(page_count) == 0) {
        printf("Error: All fields must be completed. Please try again.\n");
        free(title);
        free(author);
        free(genre);
        free(publisher);
        free(page_count);
        return;
    }

    for (int i = 0; i < strlen(page_count) - 1; i++) {
        if (!isdigit(page_count[i])) {
            printf("Error: Page count must be a number. Please try again.\n");
            free(title);
            free(author);
            free(genre);
            free(publisher);
            free(page_count);
            return;
        }
    }

    /* Create JSON object with book details */
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    json_object_set_string(obj, "title", title);
    json_object_set_string(obj, "author", author);
    json_object_set_string(obj, "genre", genre);
    json_object_set_string(obj, "publisher", publisher);
    json_object_set_string(obj, "page_count", page_count);

    /* Serialize JSON object to string */
    char *json_string = json_serialize_to_string_pretty(val);
    char *message = compute_post_request(HOST, BOOKS_ACCESS, PAYLOAD_TYPE, json_string, jwt);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        printf("Error: Failed to add book\n");
    } else {
        printf("Book added successfully.\n");
    }

    free(title);
    free(author);
    free(genre);
    free(publisher);
    free(page_count);
    json_value_free(val);
    free(message);
    free(response);
}

/**
 * @brief Retrieves the details of a specific book from the library using its ID.
 *
 * @param sockfd The socket file descriptor.
 * @param jwt The JWT token.
 */
void get_book(int sockfd, char *jwt) {
    char *id = malloc(sizeof(char) * LINELEN);

    if (!id) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    /* Consume newline character (from previous input) */
    fgets(id, LINELEN - 1, stdin);

    printf("id=");
    fgets(id, LINELEN - 1, stdin);
    id[strlen(id) - 1] = '\0';

    for (int i = 0; i < strlen(id) - 1; i++) {
        if (!isdigit(id[i])) {
            printf("Error: ID must be a number. Please try again.\n");
            free(id);
            return;
        }
    }

    /* Construct URL with book ID */
    char *url = malloc(sizeof(char) * LINELEN);
    if (!url) {
        fprintf(stderr, "Memory allocation failed\n");
        free(id);
        exit(EXIT_FAILURE);
    }
    sprintf(url, "%s/%s", BOOKS_ACCESS, id);

    char *message = compute_get_request(HOST, url, NULL, jwt);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        printf("Error: Invalid ID. Please try again.\n");
    } else {
        char *book = strstr(response, "{");
        if (book) {
            puts(book);
        }
    }

    free(id);
    free(url);
    free(message);
    free(response);
}

/**
 * @brief Deletes a specific book from the library using its ID.
 *
 * @param sockfd The socket file descriptor.
 * @param jwt The JWT token.
 */
void delete_book(int sockfd, char *jwt) {
    char *id = malloc(sizeof(char) * LINELEN);

    if (!id) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    /* Consume newline character (from previous input) */
    fgets(id, LINELEN - 1, stdin);

    printf("id=");
    fgets(id, LINELEN - 1, stdin);
    id[strlen(id) - 1] = '\0';

    for (int i = 0; i < strlen(id) - 1; i++) {
        if (!isdigit(id[i])) {
            printf("Error: ID must be a number. Please try again.\n");
            free(id);
            return;
        }
    }

    /* Construct URL with book ID */
    char *url = malloc(sizeof(char) * LINELEN);
    if (!url) {
        fprintf(stderr, "Memory allocation failed\n");
        free(id);
        exit(EXIT_FAILURE);
    }
    sprintf(url, "%s/%s", BOOKS_ACCESS, id);

    char *message = compute_delete_request(HOST, url, NULL, jwt);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        printf("Error: Invalid ID. Please try again.\n");
    } else {
        printf("Book deleted successfully.\n");
    }

    free(id);
    free(url);
    free(message);
    free(response);
}

/**
 * @brief Logs out the user by sending a GET request with the session cookie.
 *
 * @param sockfd The socket file descriptor.
 * @param cookie The session cookie.
 */
void logout(int sockfd, char *cookie) {
    /* Check if a valid session exists */
    if (!cookie) {
        printf("Error: No valid session to logout.\n");
        return;
    }

    char *message = compute_get_request(HOST, LOGOUT_ACCESS, cookie, NULL);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        printf("Error: Failed to logout.\n");
    } else {
        printf("User logged out successfully.\n");
    }

    free(message);
    free(response);
}

int main() { 
    int sockfd;

    char *command = malloc(sizeof(char) * LINELEN);
    char *cookie = NULL;
    char *jwt = NULL;

    if (!command) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    bool logged_in = false;         /* Track login status */
    bool entered_library = false;   /* Track library access status */

    while (true) {
        sockfd = open_connection(IP, PORT, AF_INET, SOCK_STREAM, 0);
        scanf("%s", command);

        if (!strcmp(command, "register")) {
            registration(sockfd);
        } else if (!strcmp(command, "exit")) {
            close(sockfd);
            printf("Succesfully exited the program.\n");
            break;
        } else if (!strcmp(command, "login")) {
            if (logged_in) {
                printf("Error: You are already logged in.\n");
                close(sockfd);
                continue;
            }
            cookie = login(sockfd);
            if (cookie) {
                logged_in = true;
            }
        } else if (!strcmp(command, "enter_library")) {
            if (!logged_in) {
                printf("Error: You must be logged in to enter the library.\n");
                close(sockfd);
                continue;
            }
            jwt = enter_library(sockfd, cookie);
            if (jwt) {
                entered_library = true;
            }
        } else if (!strcmp(command, "get_books")) {
            if (!entered_library) {
                printf("Error: You must enter the library in order to access books.\n");
                close(sockfd);
                continue;
            }
            get_books(sockfd, jwt);
        } else if (!strcmp(command, "add_book")) {
            if (!entered_library) {
                printf("Error: You must enter the library in order to add a book.\n");
                close(sockfd);
                continue;
            }
            add_book(sockfd, jwt);
        } else if (!strcmp(command, "get_book")) {
            if (!entered_library) {
                printf("Error: You must enter the library in order to access a book.\n");
                close(sockfd);
                continue;
            }
            get_book(sockfd, jwt);
        } else if (!strcmp(command, "delete_book")) {
            if (!entered_library) {
                printf("Error: You must enter the library in order to delete a book.\n");
                close(sockfd);
                continue;
            }
            delete_book(sockfd, jwt);
        } else if (!strcmp(command, "logout")) {
            if (!logged_in) {
                printf("Error: You are not logged in.\n");
                close(sockfd);
                continue;
            }
            logout(sockfd, cookie);
            logged_in = false;
            entered_library = false;
            free(cookie);
            free(jwt);
            cookie = NULL;
            jwt = NULL;
        } else {
            printf("Error: Invalid command. Please try again.\n");
        }
        /* Ensure the connection is closed after each command */
        close(sockfd);
    }

    free(command);
    if (cookie) {
        free(cookie);
    }
    if (jwt) {
        free(jwt);
    }

    return 0;
}
