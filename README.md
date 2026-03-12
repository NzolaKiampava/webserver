# Webserv

*This project has been created as part of the 42 curriculum by [nkiampav](https://github.com/NzolaKiampava), [maalmeid](https://github.com/maalmeid), and [jpanzo](https://github.com/jpanzo).*

## Table of Contents

- [Description](#description)
- [Features](#features)
- [Project Structure](#project-structure)
- [Instructions](#instructions)
  - [Requirements](#requirements)
  - [Compilation](#compilation)
  - [Execution](#execution)
  - [Testing](#testing)
  - [Configuration File Format](#configuration-file-format)
  - [Graceful Shutdown](#graceful-shutdown)
- [Technical Choices](#technical-choices)
- [Team Contributions](#team-contributions)
- [Resources](#resources)
  - [Documentation & Standards](#documentation--standards)
  - [Medium Articles](#medium-articles)
  - [Testing Tools](#testing-tools)
  - [AI Usage](#ai-usage)
- [Acknowledgments](#acknowledgments)
- [License](#license)

---

## Description

**Webserv** is a custom HTTP/1.1 web server implementation written in C++98. This project provides a deep dive into the fundamentals of web server architecture, network programming, and the HTTP protocol. The server is capable of serving static websites, handling file uploads, executing CGI scripts, and managing multiple simultaneous client connections through non-blocking I/O multiplexing.

### Goal

The primary goal of this project is to understand how HTTP servers work at a fundamental level by building one from scratch. By the end of this project, we gain practical experience with:
- Socket programming and TCP/IP communication
- HTTP/1.1 protocol implementation (request parsing, response generation)
- I/O multiplexing using `poll()` for handling concurrent connections
- Non-blocking I/O operations
- CGI (Common Gateway Interface) execution
- Configuration file parsing
- Robust error handling and server resilience

This project demonstrates that URLs starting with "HTTP" represent a complex but fascinating protocol that powers the modern web.

---

## Features

- ✅ **HTTP/1.1 Protocol Support**: Implements GET, POST, and DELETE methods
- ✅ **Non-blocking I/O**: Uses `poll()` for efficient multiplexing of multiple client connections
- ✅ **Static File Serving**: Serves HTML, CSS, JavaScript, images, and other static content
- ✅ **File Upload**: Supports uploading files via POST requests
- ✅ **CGI Execution**: Runs CGI scripts (PHP, Python, etc.) and returns dynamic content
- ✅ **Configuration File**: NGINX-inspired configuration system for flexible server setup
- ✅ **Multiple Ports**: Can listen on multiple ports simultaneously
- ✅ **Custom Error Pages**: Configurable error pages (404, 403, 500, etc.)
- ✅ **Directory Listing**: Optional directory browsing when no index file exists
- ✅ **HTTP Redirections**: Supports 301/302 redirects
- ✅ **Graceful Shutdown**: Handles SIGINT and SIGTERM signals cleanly
- ✅ **Browser Compatible**: Works with modern web browsers (Chrome, Firefox, Safari)

---

## Project Structure

```
webserv/
├── Makefile                    # Build configuration
├── README.md                   # This file
├── config/                     # Configuration files
│   ├── default.conf            # Default server configuration
│   └── test.conf               # Test configuration
├── includes/                   # Header files
│   ├── Server.hpp              # Main server class
│   ├── Client.hpp              # Client connection handler
│   ├── Config.hpp              # Configuration parser
│   ├── Request.hpp             # HTTP request parser
│   ├── Response.hpp            # HTTP response generator
│   ├── CGI.hpp                 # CGI execution handler
│   └── Utils.hpp               # Utility functions
├── srcs/                       # Implementation files
│   ├── main.cpp                # Entry point
│   ├── Server.cpp              # Server implementation
│   ├── Client.cpp              # Client implementation
│   ├── Config.cpp              # Config parser implementation
│   ├── Request.cpp             # Request parser implementation
│   ├── Response.cpp            # Response generator implementation
│   ├── CGI.cpp                 # CGI handler implementation
│   └── Utils.cpp               # Utilities implementation
├── www/                        # Web content directory
│   ├── index.html              # Default homepage
│   ├── error_pages/            # Custom error pages
│   │   ├── 404.html
│   │   ├── 403.html
│   │   └── 500.html
│   └── uploads/                # Upload directory
└── cgi-bin/                    # CGI scripts
    └── test.py                 # Example Python CGI script
```

---

## Instructions

### Requirements

- **Compiler**: `c++` (with C++98 standard support)
- **Operating System**: Linux or macOS
- **Make**: GNU Make or compatible

### Compilation

To compile the project, run:

```bash
make
```

This will generate the `webserv` executable.

### Available Make Targets

```bash
make all      # Compile the project (default)
make clean    # Remove object files
make fclean   # Remove object files and executable
make re       # Recompile from scratch
```

### Execution

#### Basic Usage

Run the server with the default configuration:

```bash
./webserv
```

The server will use `config/default.conf` and typically listen on `http://localhost:8080`.

#### Custom Configuration

Run with a custom configuration file:

```bash
./webserv path/to/config.conf
```

#### Help

Display usage information:

```bash
./webserv --help
```

### Testing

#### 1. Test with a Web Browser

Open your browser and navigate to:
```
http://localhost:8080
```

#### 2. Test with telnet

```bash
telnet localhost 8080
```

Then type:
```
GET / HTTP/1.1
Host: localhost

```
(Press Enter twice after the Host line)

#### 3. Test with curl

```bash
# GET request
curl http://localhost:8080/

# POST request with data
curl -X POST -d "name=test" http://localhost:8080/upload

# Upload a file
curl -X POST -F "file=@test.txt" http://localhost:8080/upload
```

### Configuration File Format

The configuration file is inspired by NGINX syntax. Example:

```nginx
server {
    listen 8080;
    server_name localhost;
    
    root www;
    index index.html;
    
    client_max_body_size 10M;
    
    error_page 404 /error_pages/404.html;
    error_page 500 /error_pages/500.html;
    
    location / {
        allow_methods GET POST;
        autoindex on;
    }
    
    location /upload {
        allow_methods POST DELETE;
        upload_path www/uploads;
    }
    
    location /cgi-bin {
        allow_methods GET POST;
        cgi_extension .py;
        cgi_path /usr/bin/python3;
    }
}
```

### Graceful Shutdown

To stop the server gracefully:
- Press `Ctrl+C` in the terminal, or
- Send SIGTERM: `kill <pid>`

The server will close all connections and clean up resources before exiting.

---

## Technical Choices

### I/O Multiplexing: poll() vs select()

We chose `poll()` over `select()` for the following reasons:
- **No FD limit**: `select()` has a hard limit of 1024 file descriptors (FD_SETSIZE), while `poll()` has no fixed limit
- **Safer**: `select()` can corrupt the stack if FDs exceed 1024, `poll()` doesn't have this issue
- **Cleaner API**: `poll()` uses a simple array of `pollfd` structures instead of complex `fd_set` macros
- **Better portability**: While both are POSIX, `poll()` is more modern and portable

Alternative options like `epoll()` (Linux) or `kqueue()` (BSD/macOS) are faster for thousands of connections, but `poll()` provides the best balance of simplicity, portability, and performance for this project.

### Non-blocking I/O

All sockets are configured in non-blocking mode using `fcntl()` with the `O_NONBLOCK` flag. This ensures:
- The server never blocks waiting for I/O operations
- Multiple clients can be served simultaneously
- No client can monopolize server resources
- Better resilience against slowloris-style attacks

### Architecture

The project follows a modular, object-oriented design:
- **Server**: Manages the main loop, socket creation, and client lifecycle
- **Client**: Handles individual client connections, buffers, and state
- **Request**: Parses HTTP requests according to RFC 2616
- **Response**: Generates HTTP responses with appropriate headers and status codes
- **Config**: Parses and validates configuration files
- **CGI**: Executes external scripts and captures their output

### Memory Management

We avoid dynamic allocation where possible, relying on:
- Stack-allocated buffers for I/O operations
- STL containers (vectors, strings) for dynamic data
- RAII (Resource Acquisition Is Initialization) for automatic cleanup
- No raw `new`/`delete` pairs to prevent memory leaks

---

## Team Contributions

### nkiampav
- Core server implementation (socket management, `poll()` loop)
- Client connection handling and lifecycle management
- Non-blocking I/O implementation
- Signal handling and graceful shutdown

### jpanzo
- HTTP request parsing (methods, headers, body)
- HTTP response generation (status codes, headers)
- Static file serving and MIME types
- Error page handling

### maalmeid
- Configuration file parser
- CGI script execution and environment setup
- File upload handling
- URL routing and redirections

---

## Resources

### Documentation & Standards
- [RFC 2616 - HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc2616) - Official HTTP/1.1 specification
- [RFC 7230-7235](https://datatracker.ietf.org/doc/html/rfc7230) - Updated HTTP/1.1 standards
- [NGINX Documentation](https://nginx.org/en/docs/) - Configuration file inspiration
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - Socket programming tutorial
- [poll(2) man page](https://man7.org/linux/man-pages/man2/poll.2.html) - poll() system call documentation
- [CGI 1.1 Specification](https://datatracker.ietf.org/doc/html/rfc3CGI) - Common Gateway Interface

### Medium Articles
- [webserv: Building a Non-Blocking Web Server in C++98 (A 42 project)](https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7) by MannBell - Detailed walkthrough of implementing non-blocking I/O with select/poll/epoll
- [Building an HTTP Server From Scratch in C++](https://osasazamegbe.medium.com/showing-building-an-http-server-from-scratch-in-c-2da7c0db6cb7) by Osamudiamen Azamegbe - Comprehensive guide to HTTP server architecture and TCP socket programming
- [I wrote a HTTP server from scratch in C++](https://medium.com/@aryandev512/i-wrote-a-http-server-from-scratch-in-c-0a97e8252371) by Aryandev - Practical implementation covering socket programming, HTTP protocol, and server-client model
- [Building a multi-client chat server with select and epoll](https://mecha-mind.medium.com/a-non-threaded-chat-server-in-c-53dadab8e8f3) by Abhijit Mondal - Comparison of select() vs epoll() for handling multiple concurrent connections
- [60B Over 10K: Measuring select() vs poll() vs epoll() for non-blocking TCP sockets](https://medium.com/@seantywork/60b-over-10k-measuring-select-vs-poll-vs-epoll-for-non-blocking-tcp-sockets-38d64f23319f) by Taehoon Yoon - Performance benchmarks comparing different I/O multiplexing approaches

### Testing Tools
- [Siege](https://github.com/JoeDog/siege) - HTTP load testing and benchmarking
- [Apache Bench (ab)](https://httpd.apache.org/docs/2.4/programs/ab.html) - HTTP server benchmarking
- [Postman](https://www.postman.com/) - API testing and request crafting

### AI Usage

AI tools (Claude, ChatGPT) were used strategically throughout this project to:

1. **Code Generation**: 
   - Initial boilerplate for class structures
   - Helper functions for common tasks (string parsing, byte conversions)
   - Template code for poll() loop structure

2. **Debugging Assistance**:
   - Identifying potential race conditions in non-blocking I/O
   - Explaining obscure socket errors and errno values
   - Suggesting fixes for memory leaks detected by Valgrind

3. **Documentation**:
   - Generating comprehensive code comments
   - Creating this README structure
   - Explaining complex networking concepts

4. **Research**:
   - Understanding differences between select(), poll(), epoll(), kqueue()
   - HTTP header parsing edge cases
   - CGI environment variable specifications

**Important Note**: All AI-generated code was thoroughly reviewed, tested, and understood by the team before integration. We used AI as a learning accelerator, not as a replacement for understanding. During peer evaluation, we can explain every line of code and the reasoning behind our implementation choices.

---

## Acknowledgments

Special thanks to:
- The 42 Network for providing this challenging and educational project
- NGINX developers for configuration file format inspiration
- The open-source community for excellent networking documentation
- Our peers for invaluable testing, feedback, and code review

---

## License

This project is part of the 42 school curriculum and is intended for educational purposes only.

---

**Last Updated**: January 2026  
**42 School Project**: webserv  
**Grade**: [Pending Evaluation]
