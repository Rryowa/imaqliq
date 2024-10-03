#pragma once

#include <array>
#include <fstream>
#include <cstdint>

uint32_t receive_filename_length(int client_fd);

std::string receive_filename(int client_fd, uint32_t filename_length);

/**
 * @brief Получает содержимое файла от клиента и записывает его в выходной файл.
 *
 * @param client_fd Дескриптор клиентского сокета.
 * @param outfile Ссылка на выходной файловый поток.
 * @throws std::system_error Если операция получения не удалась.
 * @throws std::runtime_error Если запись в выходной файл не удалась.
 */
void receive_and_write_file(int client_fd, std::ofstream &outfile);

/**
 * @brief Получает все байты из сокета до достижения указанной длины.
 *
 * @param socket Дескриптор сокета.
 * @param buffer Указатель на буфер, куда будут записаны полученные данные.
 * @param length Общее количество байт для получения.
 * @throws std::system_error Если операция получения не удалась.
 */
ssize_t recv_all(int socket, void *buffer, size_t length);