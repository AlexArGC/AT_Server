#include "dictionary.hpp"
#include "pattern_matcher.hpp"
#include "tty_server.hpp"
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

int main(int argc, char **argv) {
  std::string dict = "data/expect.txt";
  // Попытаться найти словарь в нескольких относительных путях (удобно при
  // запуске из build/)
  auto try_paths = {"data/expect.txt", "../../data/expect.txt",
                    "../data/expect.txt", "./data/expect.txt"};
  bool dict_found = false;
  for (auto &p : try_paths) {
    auto entries_try = load_dictionary(p);
    if (!entries_try.empty()) {
      dict = p;
      dict_found = true;
      break;
    }
  }
  std::string device;
  if (argc > 1)
    device = argv[1];
  auto entries = load_dictionary(dict);
  if (entries.empty())
    std::cerr << "Warning: словарь пуст или не найден: " << dict << "\n";
  else
    std::cerr << "Загружен словарь: " << dict << " (" << entries.size()
              << " записей)\n";
  std::string slave;
  int fd = open_tty(device, slave);
  if (fd < 0) {
    std::cerr << "Failed to open tty (device=" << device << ")\n";
    return 1;
  }
  std::cout << "Listening on: " << slave << " (fd=" << fd << ")\n";
  std::string line;
  while (true) {
    ssize_t r = read_line(fd, line);
    if (r < 0)
      break; // неисправимая ошибка
    if (r == 0) {
      // EOF от клиента: продолжаем работу и ждём новые подключения или данные.
      continue;
    }
    if (line.empty())
      continue;
    std::cerr << "RECV: '" << line << "'\n";
    // Обработать локальную команду завершения работы.
    if (line == "AT+SHUTDOWN") {
      std::cerr << "Получена команда AT+SHUTDOWN, завершаю работу...\n";
      break;
    }
    // Найти совпадение.
    bool found = false;
    for (auto &e : entries) {
      if (pattern_match(e.pattern, line)) {
        // Отправляем ответ, добавляя CRLF в конце для корректного терминального
        // вывода
        std::string out = e.answer + "\r\n";
        std::cerr << "SEND: '" << e.answer << "' for pattern '" << e.pattern
                  << "'\n";
        write_all(fd, out.c_str(), out.size());
        found = true;
        break;
      }
    }
    if (!found) {
      std::string out = "ERROR\r\n";
      std::cerr << "SEND: 'ERROR' (no match)\n";
      write_all(fd, out.c_str(), out.size());
    }
  }
  close(fd);
  return 0;
}
