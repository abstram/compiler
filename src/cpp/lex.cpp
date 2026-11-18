module;
#include <cctype>
#include <expected>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

export module compiler.lex;

export namespace lex {
std::unordered_set<std::string> two_char_symbols = {
    "==", "!=", "+=", "-=", "*=", "/=", "%=",
    "::", "->", "--", "&&", "||", ">=", "<="};
// our parsing system won't allow for <- symbol but -> is fine

enum class type { TEXT, INT, FLOAT, SYMBOL, NEWLINE };
enum class error { NUMBER_BEFORE_TEXT };

struct token {
  type type;
  std::string value;
  int line;

  bool operator==(const token &o) const {
    return (type == o.type && value == o.value);
  }
};

std::string token_to_string(const token &t) {
  std::string result = "VALUE: " + t.value + "\nTYPE: ";

  switch (t.type) {
  case type::TEXT:
    result += "text";
    break;

  case type::INT:
    result += "int";
    break;

  case type::FLOAT:
    result += "float";
    break;

  case type::SYMBOL:
    result += "symbol";
    break;

  case type::NEWLINE:
    result += "newline";
    break;
  }

  return result;
}

void print_tokens(const std::vector<token> &vt) {
  for (int i = 0; i < vt.size(); i++) {
    std::cout << token_to_string(vt[i]) << '\n';
    if (i < (vt.size() - 1)) {
      std::cout << '\n';
    }
  }
}

token init_token(const char c, int line_number) {
  token result;
  if (std::isalpha(c) || c == '_') {
    std::string result_value = "";
    result_value += c;

    result = token{type::TEXT, result_value, line_number};
  } else if (std::isdigit(c)) {
    std::string result_value = "";
    result_value += c;

    result = token{type::INT, result_value, line_number};
  } else if (std::ispunct(c)) {
    std::string result_value = "";
    result_value += c;

    result = token{type::SYMBOL, result_value, line_number};
  }

  return result;
}

std::expected<void, error> lex(std::vector<token> &source_tokens,
                               const std::string &line, int line_number) {
  token current;
  bool current_init = false;

  for (int i = 0; i < line.length(); i++) {
    if (line[i] == '#') {
      if (i == 0)
        return {};

      source_tokens.emplace_back(type::NEWLINE, "", line_number);
      return {};
    }

    if (!current_init) {
      if (std::isspace(line[i])) {
        continue;
      }

      current = init_token(line[i], line_number);
      current_init = true;
      continue;
    }

    // from here, doing i-- once is fully safe
    switch (current.type) {
    case type::SYMBOL:
      if (current.value == "." && std::isdigit(line[i])) {
        current.type = type::FLOAT;
        // no break here

      } else if (current.value.length() >= 2 ||
                 !two_char_symbols.contains(current.value + line[i])) {
        source_tokens.push_back(current);
        current_init = false;
        i--;
        break;
      }

      current.value += line[i];
      break;

    case type::INT:
      if (line[i] == '.') {
        current.type = type::FLOAT;

      } else if (std::isalpha(line[i]) || line[i] == '_') {
        return std::unexpected(error::NUMBER_BEFORE_TEXT);

      } else if (!isdigit(line[i])) {
        source_tokens.push_back(current);
        current_init = false;
        i--;
        break;
      }

      current.value += line[i];
      break;

    case type::FLOAT:
      if (std::isalpha(line[i]) || line[i] == '_') {
        return std::unexpected(error::NUMBER_BEFORE_TEXT);
      } else if (!std::isdigit(line[i])) {
        source_tokens.push_back(current);
        current_init = false;
        i--;
        break;
      }

      current.value += line[i];
      break;

    case type::TEXT:
      if (!std::isalnum(line[i])) {
        source_tokens.push_back(current);
        current_init = false;
        i--;
        break;
      }

      current.value += line[i];
      break;

    case type::NEWLINE:
      // this will never happen
      break;
    }
  }

  if (current_init)
    source_tokens.emplace_back(current);
  if (line.length() != 0)
    source_tokens.emplace_back(type::NEWLINE, "", line_number);
  return {};
}

} // namespace lex
