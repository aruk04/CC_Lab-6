#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

std::string getContainerId() {
    char hostname[256];
    gethostname(hostname, 256);
    return std::string(hostname);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);
    
    std::cout << "Backend server listening on port 8080 (hostname: " << getContainerId() << ")" << std::endl;
    
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            continue;
        }
        
        // Read the HTTP request (we don't process it, just drain it)
        char buffer[4096];
        recv(client_socket, buffer, sizeof(buffer), 0);
        
        std::string body = "<html><body><h1>Served by backend: " + getContainerId() + "</h1></body></html>";
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += body;
        
        send(client_socket, response.c_str(), response.length(), 0);
        
        shutdown(client_socket, SHUT_WR);
        close(client_socket);
    }
    
    return 0;
}
