module;
#include <cctype>
#include <expected>
#include <string>
#include <unordered_set>
#include <vector>

export module compiler.lex;

export namespace lex {
// TODO: expand this to include all two char symbols
std::unordered_set<std::string> two_char_symbols = {"==", "!=", "+="};

enum class type { TEXT, INT, FLOAT, SYMBOL, NEWLINE };
enum class error { NUMBER_BEFORE_TEXT };

struct token {
  type type;
  std::string value;
};

std::string token_to_string(token &t) {
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

token init_token(const char c) {
  token result;
  if (std::isalpha(c)) {
    std::string result_value = "";
    result_value += c;

    result = token{type::TEXT, result_value};
  } else if (std::isdigit(c)) {
    std::string result_value = "";
    result_value += c;

    result = token{type::INT, result_value};
  } else if (std::ispunct(c)) {
    std::string result_value = "";
    result_value += c;

    result = token{type::SYMBOL, result_value};
  }

  return result;
}

std::expected<void, error> lex(std::vector<token> &source_tokens,
                               const std::string &line) {
  token current;
  bool current_init = false;

  for (int i = 0; i < line.length(); i++) {
    if (line[i] == '#') {
      if (i == 0)
        return {};

      source_tokens.emplace_back(type::NEWLINE, "");
      return {};
    }

    if (!current_init) {
      if (std::isspace(line[i])) {
        continue;
      }

      current = init_token(line[i]);
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

      } else if (std::isalpha(line[i])) {
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
      if (std::isalpha(line[i])) {
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
  source_tokens.emplace_back(type::NEWLINE, "");
  return {};
}

} // namespace lex
