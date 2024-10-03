#include "const.h"
#include "receiver.h"

#include <system_error>
#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdint>
#include <vector>
#include "netinet/in.h"

uint32_t receive_filename_length(int client_fd) {
    uint32_t filename_length_network;

    ssize_t recv_len = recv_all(client_fd, &filename_length_network, sizeof(filename_length_network));
    if (recv_len != sizeof(filename_length_network)) {
        throw std::system_error(errno, std::generic_category(), "Receive filename length failed");
    }

    return ntohl(filename_length_network);
}

std::string receive_filename(int client_fd, uint32_t filename_length) {
    std::vector<char> filename_buffer(filename_length);

    ssize_t recv_len = recv_all(client_fd, filename_buffer.data(), filename_length);
    if (recv_len != static_cast<ssize_t>(filename_length)) {
        throw std::system_error(errno, std::generic_category(), "Receive filename failed");
    }
    std::string filename;
    filename.assign(filename_buffer.begin(), filename_buffer.end());

    return filename;
}

void receive_and_write_file(int client_fd, std::ofstream &outfile) {
    std::array<char, BUFFER_SIZE> buffer;
    ssize_t bytes_received;

    while ((bytes_received = recv(client_fd, buffer.data(), buffer.size(), 0)) > 0) {
        outfile.write(buffer.data(), bytes_received);
        if (!outfile) {
            throw std::runtime_error("Failed to write to output file.");
        }
    }

    if (bytes_received < 0) {
        throw std::system_error(errno, std::generic_category(), "Receive file content failed");
    }
}

ssize_t recv_all(int socket, void *buffer, size_t length) {
    ssize_t total_received = 0;
    ssize_t bytes_received;

    while (total_received < static_cast<ssize_t>(length)) {
        bytes_received = recv(socket, static_cast<char*>(buffer) + total_received, length - total_received, 0);
        if (bytes_received < 0) {
            throw std::system_error(errno, std::generic_category(), "recv_all failed");
        }
        if (bytes_received == 0) {
            break; // Connection closed gracefully
        }
        total_received += bytes_received;
    }
    return total_received;
}