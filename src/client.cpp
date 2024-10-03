#include "const.h"
#include "file.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <array>
#include <system_error>
#include <stdexcept>

int create_socket() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        throw std::system_error(errno, std::generic_category(), "Socket creation failed");
    }
    return sock_fd;
}

void setup_server_address(struct sockaddr_in& server_addr, const char* server_ip, int server_port) {
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    // Конвертим IPv4 из текста в бинарник
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        throw std::invalid_argument(std::string("Invalid address or Address not supported: ") + server_ip);
    }
}

void connect_to_server(int sock_fd, const struct sockaddr_in& server_addr) {
    if (connect(sock_fd, reinterpret_cast<const struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        throw std::system_error(errno, std::generic_category(), "Connection failed");
    }
}

/**
 * @brief Отправляет имя файла и длину имени, чтобы tcp понял где что.
 *
 * @param sock_fd Дескриптор файла сокета.
 * @param filename Имя файла для отправки.
 * @return void
 * @throws std::system_error Если отправка данных не удалась.
 */
void send_file_info(int sock_fd, const std::string& filename) {
    // Отправка длины имени файла
    uint32_t filename_length = htonl(static_cast<uint32_t>(filename.size()));
    ssize_t sent_bytes = send(sock_fd, &filename_length, sizeof(filename_length), 0);
    if (sent_bytes != sizeof(filename_length)) {
        throw std::system_error(errno, std::generic_category(), "Send filename length failed");
    }

    // Отправка длины имени файла
    sent_bytes = send(sock_fd, filename.c_str(), filename.size(), 0);
    if (sent_bytes != static_cast<ssize_t>(filename.size())) {
        throw std::system_error(errno, std::generic_category(), "Send filename failed");
    }
}

/**
 * @brief Отправляет содержимое файла на сервер.
 *
 * @param sock_fd Дескриптор файла сокета.
 * @param infile Ссылка на поток входного файла.
 * @return void
 * @throws std::system_error Если отправка данных не удалась.
 */
void send_file_content(int sock_fd, std::ifstream& infile) {
    // Чтение из файла и отправка данных на сервер
    std::array<char, BUFFER_SIZE> buffer;
    while (infile.read(buffer.data(), buffer.size())) {
        ssize_t sent_bytes = send(sock_fd, buffer.data(), buffer.size(), 0);
        if (sent_bytes < 0) {
            throw std::system_error(errno, std::generic_category(), "Send file content failed");
        }
    }

    // Отправка оставшихся данных
    if (infile.gcount() > 0) {
        ssize_t sent_bytes = send(sock_fd, buffer.data(), infile.gcount(), 0);
        if (sent_bytes < 0) {
            throw std::system_error(errno, std::generic_category(), "Send file content failed");
        }
    }
}

void close_socket(int sock_fd) {
    if (close(sock_fd) < 0) {
        throw std::system_error(errno, std::generic_category(), "Closing socket failed");
    }
}

/**
 * @brief Основная функция, которая организует процесс отправки файла.
 *
 * @param argc Количество аргументов.
 * @param argv Массив аргументов.
 * @return int EXIT_SUCCESS при успехе, EXIT_FAILURE в случае неудачи.
 */
int main(int argc, char* argv[]) {
    try {
        std::string file_path = read_filepath(argc, argv);

        std::ifstream infile = open_file(file_path);
        if (!infile) {
            throw std::runtime_error(std::string("Failed to open file: ") + file_path);
        }

        std::string filename = get_filename(file_path);

        struct sockaddr_in server_addr = {};
        setup_server_address(server_addr, SERVER_IP, SERVER_PORT);

        int sock_fd = create_socket();

        connect_to_server(sock_fd, server_addr);

        send_file_info(sock_fd, filename);

        send_file_content(sock_fd, infile);

        std::cout << "File sent successfully to server." << std::endl;

        close_socket(sock_fd);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}