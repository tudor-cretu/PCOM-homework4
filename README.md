# Client Application for Library Management

This project is a client-side application for managing a library system. It supports user registration, login, accessing the library, viewing and managing books, and logging out. The application communicates with a server using HTTP requests such as GET, POST and DELETE.

## Table of Contents
- [Features](#features)
- [Usage](#usage)
- [Files](#files)
- [Dependencies](#dependencies)
- [Implementation](#implementation)

## Features
- **User Registration**: Create a new account by providing a username and password.
- **User Login**: Authenticate with the server using your credentials.
- **Access Library**: Enter the library to view and manage books.
- **View Books**: Retrieve a list of books available in the library.
- **Add Book**: Add a new book to the library by providing book details.
- **View Book Details**: Retrieve details of a specific book by its ID.
- **Delete Book**: Remove a book from the library using its ID.
- **User Logout**: Logout from the current session.

## Usage
Use the following commands to interact with the application:

- **register**: Register a new user.
  - Prompts: 
    - `username`: Enter your desired username.
    - `password`: Enter your desired password.

- **login**: Log in with your credentials.
  - Prompts: 
    - `username`: Enter your username.
    - `password`: Enter your password.

- **enter_library**: Access the library.

- **get_books**: View a list of all books in the library.

- **add_book**: Add a new book by providing its details.
  - Prompts:
    - `title`: Enter the title of the book.
    - `author`: Enter the author of the book.
    - `genre`: Enter the genre of the book.
    - `publisher`: Enter the publisher of the book.
    - `page_count`: Enter the page count of the book.

- **get_book**: Retrieve details of a specific book by its ID.
  - Prompts:
    - `id`: Enter the ID of the book.

- **delete_book**: Delete a book using its ID.
  - Prompts:
    - `id`: Enter the ID of the book.

- **logout**: Log out from the current session.

- **exit**: Exit the application.

## Files
### client.c
Handles the main functionality of the client application, including user commands and communication with the server.

### requests.c
Contains functions for creating and sending HTTP requests:
- `compute_get_request`: Constructs a GET request.
- `compute_post_request`: Constructs a POST request.
- `compute_delete_request`: Constructs a DELETE request.

### helpers.c
Provides helper functions for error handling, connection management, and data transmission:
- `error`: Handles errors by printing a message and exiting.
- `compute_message`: Appends a line to a message.
- `open_connection`: Opens a connection to a server.
- `close_connection`: Closes a connection.
- `send_to_server`: Sends a message to the server.
- `receive_from_server`: Receives a response from the server.
- `basic_extract_json_response`: Extracts a JSON response from a string.

## Dependencies
- **parson**: A JSON library for C, used for JSON parsing and serialization.

## Implementation
The client application is implemented in C and interacts with a server using HTTP requests. The program performs several key operations such as user registration, login, accessing the library, viewing and managing books, and logging out. The implementation details are as follows:

### HTTP Communication
The client communicates with the server using standard HTTP methods (GET, POST, DELETE). The `requests.c` file contains functions to construct these requests, ensuring that the correct headers and payloads are included for each type of request. These requests are sent to the server, and responses are handled appropriately to provide feedback to the user.

### Dynamic Buffer Management
To handle data received from the server, the `buffer.c` file implements a dynamic buffer. This buffer can grow as needed to accommodate varying amounts of data, ensuring that responses of any size can be managed efficiently. Functions in `buffer.c` support initializing, adding to, and searching within the buffer.

### Helper Functions
The `helpers.c` file includes functions to manage connections, send and receive data, handle errors, and format messages. These helper functions simplify the main logic in `client.c` by abstracting common operations, making the code more modular and easier to maintain.

### JSON Handling with Parson
The Parson library is used for JSON parsing and serialization. This dependency was chosen for several reasons:
- **Ease of Use**: Parson provides a simple and intuitive API for working with JSON data, making it easy to integrate into the project.
- **Flexibility**: The library supports parsing JSON strings into objects and serializing objects back into strings, which is essential for constructing HTTP request payloads and handling server responses.
- **Lightweight**: Parson is a lightweight library with no external dependencies, making it an ideal choice for a C-based client application.

In the context of this program, Parson is used to:
- Create JSON objects for user registration and login requests.
- Serialize these objects into JSON strings to be included in HTTP request bodies.
- Parse JSON responses from the server to extract relevant data (e.g., authentication tokens, book details).

By leveraging Parson, the client application can effectively handle JSON data, ensuring smooth communication with the server and accurate data processing.

### Key Operations
- **User Registration**: Collects username and password, constructs a JSON object, and sends a POST request to the server.
- **User Login**: Collects username and password, constructs a JSON object, and sends a POST request to the server. Extracts a session cookie from the server's response.
- **Access Library**: Sends a GET request with the session cookie to enter the library and retrieve a JWT token.
- **View Books**: Sends a GET request with the JWT token to retrieve a list of books in the library.
- **Add Book**: Collects book details, constructs a JSON object, and sends a POST request with the JWT token to add a new book.
- **View Book Details**: Sends a GET request with the JWT token to retrieve details of a specific book by its ID.
- **Delete Book**: Sends a DELETE request with the JWT token to remove a book by its ID.
- **User Logout**: Sends a GET request with the session cookie to log out the user.

## Mentions

- I have used the solution of the 9th laboratory to kickstart the homework. 
- I used 2 sleep days in doing this homework. 