#pragma once

#include <string>

// Открывает PTY или реальное устройство TTY.
// Если `device_path` пустая строка — создаётся пара PTY (master/slave),
// возвращается файловый дескриптор master, а в `slave_out` записывается путь к
// slave-устройству. Если указан `device_path`, функция откроет этот путь и
// вернёт соответствующий fd, а `slave_out` будет равен указанному пути. В
// случае ошибки возвращается -1.
int open_tty(const std::string &device_path, std::string &slave_out);

// Считать строку из `fd` в `out`. Удаляет символы '\r' и '\n'.
// Возвращает количество прочитанных байт (>0), 0 при EOF, или отрицательное
// число при ошибке.
ssize_t write_all(int fd, const char *buf, size_t len);
ssize_t read_line(int fd, std::string &out);
