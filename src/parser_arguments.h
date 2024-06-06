#include <Arduino.h>

template <typename T, typename U>
class Pair {
 private:
  T first_;
  U second_;

 public:
  Pair(T first, U second) : first_(first), second_(second) {}
  T first() { return this->first_; }
  U second() { return this->second_; }
};

static Pair<String, String> ParseCommand(String message) {
  String keyword = "";
  for (auto& car : message) {
    if (car == ' ') break;
    keyword += car;
  }
  if (keyword != "") message.remove(0, keyword.length());
  keyword.trim();
  message.trim();

  return Pair<String, String>(keyword, message);
}

static String ParseArgument(String message) {
  String keyword = "";
  for (auto& car : message) {
    if (car == '"') break;
    keyword += car;
  }
  if (keyword != "") message.remove(0, keyword.length());
  message.trim();
  int msg_len = message.length();
  if (msg_len > 0) {
    message.remove(0, 1);
    message.remove(msg_len - 2);
  }

  return message;
}