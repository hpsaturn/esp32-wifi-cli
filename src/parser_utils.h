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

static inline String ParseArgument(String message) {
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

static inline int ParseEnableDisable(String args) {
  Pair<String, String> operands = ParseCommand(args);
  String param = operands.first();
  param.toUpperCase();
  if (param.equals("ENABLE"))
    return 1;
  else if (param.equals("DISABLE"))
    return 0;
  else 
    return -1;
}

static inline bool extract_connect_parames(const char* args, char** ssid, char** password, Stream *response) {
    const char* password_prefix = " password ";

    // Find the " password " prefix
    const char* password_start = strstr(args, password_prefix);
    if (password_start == NULL) {
        response->printf("Invalid command syntax\n");
        return false;
    }

    // Calculate the length of the SSID
    size_t ssid_length = password_start - args;

    // Allocate memory for the SSID and copy it
    *ssid = (char*)malloc(ssid_length + 1);
    if (*ssid == NULL) {
        response->printf("Memory allocation failed\n");
        return false;
    }
    strncpy(*ssid, args, ssid_length);
    (*ssid)[ssid_length] = '\0';

    // Skip the " password " prefix to get the password
    password_start += strlen(password_prefix);

    // Allocate memory for the password and copy it
    *password = strdup(password_start);
    if (*password == NULL) {
        response->printf("Memory allocation failed\n");
        free(*ssid);
        return false;
    }
    return true;
}