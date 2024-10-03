#include "const.h"
#include "file.h"
#include "receiver.h"

#include <iostream>
#include <csignal>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdlib>
#include <array>
#include <system_error>
#include <stdexcept>

volatile std::sig_atomic_t stop_flag = 0;

void signal_handler(int signum) {
    stop_flag = 1;
    std::cout << "\nReceived signal " << signum << ", shutting down server..." << std::endl;
}

void setup_signal_handlers() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;

    // Не устанавливает SA_RESTART, чтобы гарантировать прерывание accept()
    if (sigaction(SIGTERM, &sa, nullptr) == -1) {
        throw std::system_error(errno, std::generic_category(), "sigaction SIGTERM failed");
    }
    if (sigaction(SIGHUP, &sa, nullptr) == -1) {
        throw std::system_error(errno, std::generic_category(), "sigaction SIGHUP failed");
    }
}

/**
 * @brief Создает и настраивает серверный сокет.
 *
 * @param port Номер порта, на котором сервер прослушивает.
 * @return int Дескриптор файла созданного серверного сокета.
 * @throws std::system_error Если создание сокета, setsockopt, bind или listen не удалось.
 */
int create_server_socket(int port) {
    int server_fd;

    // Создание TCP сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::system_error(errno, std::generic_category(), "Socket creation failed");
    }

    // Переиспользование сокета
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::system_error(errno, std::generic_category(), "setsockopt SO_REUSEADDR failed");
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    memset(&server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        throw std::system_error(errno, std::generic_category(), "Bind failed");
    }

    if (listen(server_fd, CONNECTIONS) < 0) {
        throw std::system_error(errno, std::generic_category(), "Listen failed");
    }

    std::cout << "Server is listening on port " << port << "..." << std::endl;
    return server_fd;
}

/**
 * @brief Принимает подключение клиента.
 *
 * @param server_fd Дескриптор файла серверного сокета.
 * @return int Дескриптор файла принятого клиентского сокета.
 * @throws std::system_error Если принятие клиентского подключения не удалось.
 */
int accept_client(int server_fd) {
    struct sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    int client_fd;

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        if (errno == EINTR) {
            // Interrupted by signal, return -1 to allow graceful shutdown
            return -1;
        }
        throw std::system_error(errno, std::generic_category(), "Accept failed");
    }

    char client_ip[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN) == nullptr) {
        throw std::system_error(errno, std::generic_category(), "inet_ntop failed");
    }
    std::cout << "Accepted connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

    return client_fd;
}

void close_client_connection(int client_fd, std::ofstream &outfile) {
    if (outfile.is_open()) {
        outfile.close();
    }
    close(client_fd);
}

/**
 * @brief Обрабатывает подключенного клиента, получая имя файла и содержимое файла.
 *
 * @param client_fd Дескриптор файла подключенного клиентского сокета.
 * @throws std::system_error Если любая операция приема не удалась.
 * @throws std::runtime_error Если запись в выходной файл не удалась.
 */
void handle_client(int client_fd) {
    std::ofstream outfile;
    try {
        uint32_t filename_length = receive_filename_length(client_fd);

        std::string filename = receive_filename(client_fd, filename_length);

        std::string output_filename = create_output_filename(filename);

        open_output_file(output_filename, outfile);

        receive_and_write_file(client_fd, outfile);

        close_client_connection(client_fd, outfile);
    } catch (const std::exception &e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
        close_client_connection(client_fd, outfile);
    }
}

void run_server(int server_fd) {
    while (!stop_flag) {
        try {
            int client_fd = accept_client(server_fd);
            if (client_fd >= 0) {
                handle_client(client_fd);
            }
            // If client_fd < 0 and stop_flag is set, выход
        } catch (const std::system_error &e) {
            std::cerr << "System error: " << e.what() << std::endl;
            if (stop_flag) {
                // Interrupted by a signal, proceed to shutdown
                break;
            }
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    std::cout << "Server shutting down..." << std::endl;
}

int main() {
    try {
        setup_signal_handlers();

        int server_fd = create_server_socket(SERVER_PORT);

        // Server loop
        run_server(server_fd);

        close(server_fd);
        std::cout << "Server shut down gracefully." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}