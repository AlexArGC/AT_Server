#include "tty_server.hpp"
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <pty.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

// Держать дескриптор slave открытым при создании PTY,
// чтобы master не получал EIO, когда внешние клиенты не подключены.
static int g_slave_keep_fd = -1;

int open_tty(const std::string &device_path, std::string &slave_out) {
  if (!device_path.empty()) {
    int fd = open(device_path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
      return -1;
    slave_out = device_path;
    return fd;
  }
  int master = posix_openpt(O_RDWR | O_NOCTTY);
  if (master < 0)
    return -1;
  if (grantpt(master) != 0) {
    close(master);
    return -1;
  }
  if (unlockpt(master) != 0) {
    close(master);
    return -1;
  }
  char *name = ptsname(master);
  if (!name) {
    close(master);
    return -1;
  }
  slave_out = std::string(name);
  // Настроить параметры терминала slave в raw-режиме.
  int sfd = open(slave_out.c_str(), O_RDWR | O_NOCTTY);
  if (sfd >= 0) {
    struct termios tio;
    if (tcgetattr(sfd, &tio) == 0) {
      cfmakeraw(&tio);
      tio.c_lflag &= ~(ECHO | ECHOE | ECHONL);
      tcsetattr(sfd, TCSANOW, &tio);
    }
    // Оставляем slave fd открытым в процессе, чтобы чтения с master
    // не возвращали EIO, если внешний клиент временно не подключён.
    g_slave_keep_fd = sfd;
  }
  // Возвращаем master fd — это необходимо, чтобы пара PTY оставалась доступной.
  return master;
}

ssize_t write_all(int fd, const char *buf, size_t len) {
  size_t written = 0;
  while (written < len) {
    ssize_t w = write(fd, buf + written, len - written);
    if (w <= 0)
      return (ssize_t)w;
    written += w;
  }
  return (ssize_t)written;
}

ssize_t read_line(int fd, std::string &out) {
  out.clear();
  char c;
  // Установить блокирующее поведение для чтения (временно снять O_NONBLOCK),
  // затем читать побайтово до символа конца строки.
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
  while (true) {
    ssize_t r = read(fd, &c, 1);
    if (r <= 0) {
      int e = errno;
      if (r == 0) {
        // EOF от master означает, что в настоящий момент нет открытого
        // конца slave. Подождать и повторить попытку.
        usleep(100000);
        continue;
      }
      if (e == EINTR) {
        // Системный вызов прерван сигналом — повторить чтение.
        continue;
      }
      if (e == EAGAIN) {
        // Данных пока нет — короткая пауза и повтор.
        usleep(100000);
        continue;
      }
      if (e == EIO) {
        // Ошибка ввода/вывода на master (slave отсутствует). Подождать дольше.
        usleep(200000);
        continue;
      }
      // report unexpected read error and return
      fprintf(stderr, "read() returned %zd, errno=%d (%s)\n", r, e,
              strerror(e));
      return r;
    }
    // screen может отключить ECHO на slave-стороне PTY после подключения.
    // Возвращаем введённый символ и нормализуем конец строки для отображения.
    if (c == '\r' || c == '\n') {
      if (c == '\r')
        write_all(fd, "\r\n", 2);
      break;
    }
    write_all(fd, &c, 1);
    out.push_back(c);
    if (out.size() > 8192)
      break;
  }
  // restore flags
  fcntl(fd, F_SETFL, flags);
  return out.size();
}
