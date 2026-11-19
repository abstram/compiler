module;
#include <array>
#include <expected>
#include <span>
#include <string>
#include <variant>
#include <vector>

export module compiler.ast;
import compiler.lex;

export namespace ast {
enum class error_code { UNEXPECTED_TOKEN, UNEXPECTED_EOF };
struct error {
  error_code code;
  int line;
};

// TODO: unnecessary heap allocs all over this file
// maybe get rid of some of them

struct requires_definition {
  std::vector<std::string> includes;
  std::vector<std::string> imports;
  std::vector<std::string> links;
  std::vector<std::string>
      enables; // ill parse this but idk if it'll be a valid feature
};

struct variable_definition {
  std::string name;
  std::string register_name;
};

struct assign {
  std::string name;
};

struct add_assign {
  std::string name;
};

struct sub_assign {
  std::string name;
};

struct mul_assign {
  std::string name;
};

struct div_assign {
  std::string name;
};

struct mod_assign {
  std::string name;
};

struct memory_access {}; // memory[inside], outside is none

struct memory_assign {};
struct if_definition {};
struct while_definition {};
// those 3 ^ point to this handler on inside:
struct handler {};

struct const_definition {
  std::string name;
};

struct add {};
struct sub {};
struct mul {};
struct div {};
struct mod {};
struct and_op {};
struct or_op {};

struct interpret {}; // ^ symbol (UNARY OPERATOR)
struct negative {};  // also unary
struct positive {};  // also unary
struct not_op {};    // also unary

// these 3 next ones have no children (no inside and no outside)
struct value {
  std::string value;
};

struct variable {
  std::string name;
};

struct function_call {
  std::string name;
  std::vector<std::string> paren_params;
  std::vector<std::string> brack_params;
};

struct inline_assembly {
  std::vector<lex::token> code;
};

struct struct_init { // inside doesn't go to anything, only outside
  std::string name;
  std::string struct_name;
  std::string
      var_name; // this is memory[var_name] in e :: engine at memory[...]
                // you can't do operations in the ... in there ^^^^^^^
};

// i changed my mind, no nested structs

struct access { // . symbol for like struct_name.struct_member
  std::string struct_name;
  std::string member_name;
};

struct drop { // -- symbol, no inside
  std::vector<std::string> names;
};

struct drop_all { // -* symbol, no inside
};

struct to_cast {
  std::string name;
  std::string new_type;
};

struct as_cast {
  std::string name;
  std::string new_type;
};

struct param {
  std::string name;
  std::vector<std::string> types;
};

enum class opt_level { NONE, WINDOWED_STOKE, FULL_STOKE };

struct function_definition {
  std::string name;

  std::vector<param> paren_params;
  std::vector<param> brack_params;
  param result;

  opt_level optimization;
};

struct struct_member {
  std::string name;
  std::string type;
};

struct struct_definition { // not finished
  std::string name;
  std::vector<struct_member> members;
};

struct union_definition { // not finished
  std::string name;
  std::vector<std::string> members;
};

struct enum_definition {
  std::string name;
  std::vector<std::string> members;
};

struct none {};

using info =
    std::variant<function_definition, struct_definition, union_definition, none,
                 requires_definition, variable_definition, assign, add_assign,
                 sub_assign, mul_assign, div_assign, mod_assign, memory_assign,
                 memory_access, if_definition, while_definition, handler,
                 const_definition, add, sub, mul, div, mod, and_op, or_op,
                 interpret, negative, positive, not_op, value, variable,
                 function_call, inline_assembly, struct_init, access, drop,
                 drop_all, to_cast, as_cast, enum_definition>;

struct ast_node {
  info info;
  ast_node *inside;
  ast_node *outside;
};

bool str_in(std::span<const std::string> sv, std::string s) {
  for (int i = 0; i < sv.size(); i++) {
    if (sv[i] == s)
      return true;
  }

  return false;
}

std::expected<void, error>
validate_token(std::vector<lex::token> &source_tokens, int start, int end,
               std::span<const lex::type> valid_types,
               std::span<const std::string> valid_values) {
  if (start >= end) {
    return std::unexpected(
        error{error_code::UNEXPECTED_EOF, source_tokens[end - 1].line});
  }

  bool ok = false;
  for (const lex::type o : valid_types) {
    if (source_tokens[start].type == o) {
      ok = true;
    }
  }

  bool ok2 = false;
  if (valid_values.size() == 0)
    ok2 = true;
  else
    for (const std::string o : valid_values) {
      if (source_tokens[start].value == o) {
        ok2 = true;
      }
    }

  if (!(ok && ok2)) {
    return std::unexpected(
        error{error_code::UNEXPECTED_TOKEN, source_tokens[start].line});
  }

  return {};
}

std::expected<void, error> fill_types(std::span<const std::string> end_here,
                                      std::vector<std::string> &types,
                                      std::vector<lex::token> &source_tokens,
                                      int &start, int end) {

  while (!str_in(
      end_here,
      source_tokens[start].value)) { // , for fill params, } for result value
    start++;
    auto token_is_valid = validate_token( // e.g. gi
        source_tokens, start, end, std::array<lex::type, 1>{lex::type::TEXT},
        std::array<std::string, 0>{});
    if (!token_is_valid) {
      return std::unexpected(token_is_valid.error());
    }

    types.push_back(source_tokens[start].value);

    start++;
    token_is_valid = validate_token(source_tokens, start, end,
                                    std::array<lex::type, 1>{lex::type::SYMBOL},
                                    std::array<std::string, 0>{});
    if (!token_is_valid) {
      return std::unexpected(token_is_valid.error());
    }
  }

  return {};
}

std::expected<void, error> fill_params(const std::string &end_here,
                                       std::vector<param> &params,
                                       std::vector<lex::token> &source_tokens,
                                       int &start, int end) {
  // start at beginning parenthesis
  while (source_tokens[start].value != end_here) {
    start++;
    param temp;
    if (source_tokens[start].type == lex::type::TEXT) {
      temp.name = source_tokens[start].value;
    } else {
      return std::unexpected(
          error{error_code::UNEXPECTED_TOKEN, source_tokens[start].line});
    }

    start++;

    if (source_tokens[start] == lex::token{lex::type::SYMBOL, ":"}) {
      auto types_are_valid = fill_types(std::array<std::string, 2>{",", ")"},
                                        temp.types, source_tokens, start, end);
      if (!types_are_valid)
        return std::unexpected(types_are_valid.error());
    } else {
      return std::unexpected(
          error{error_code::UNEXPECTED_TOKEN, source_tokens[start].line});
    }

    params.push_back(temp);
    // now start should be on the comma or the parenthesis
  }

  start++;

  // start should be AFTER the closing parenthesis, aka after the end char
  return {};
}

std::expected<function_definition, error>
analyze_fn(std::vector<lex::token> &source_tokens, int &start, int end,
           int &new_end) {
  function_definition temp;

  start++;
  auto token_is_valid = validate_token(
      source_tokens, start, end, std::array<lex::type, 1>{lex::type::TEXT},
      std::array<std::string, 0>{});
  if (!token_is_valid) {
    return std::unexpected(token_is_valid.error());
  }

  temp.name = source_tokens[start].value;

  start++;
  token_is_valid = validate_token(
      source_tokens, start, end, std::array<lex::type, 1>{lex::type::SYMBOL},
      std::array<std::string, 6>{"!", "!!", "(", "[", "->", "{"});
  if (!token_is_valid) {
    return std::unexpected(token_is_valid.error());
  }

  if (source_tokens[start].value == "!") {
    temp.optimization = opt_level::WINDOWED_STOKE;
    start++;
    token_is_valid = validate_token(
        source_tokens, start, end, std::array<lex::type, 1>{lex::type::SYMBOL},
        std::array<std::string, 4>{"(", "[", "->", "{"});
    if (!token_is_valid) {
      return std::unexpected(token_is_valid.error());
    }

  } else if (source_tokens[start].value == "!!") {
    temp.optimization = opt_level::FULL_STOKE;
    start++;
    token_is_valid = validate_token(
        source_tokens, start, end, std::array<lex::type, 1>{lex::type::SYMBOL},
        std::array<std::string, 4>{"(", "[", "->", "{"});
    if (!token_is_valid) {
      return std::unexpected(token_is_valid.error());
    }
  } else {
    temp.optimization = opt_level::NONE;
  }

  // now we've checked for optimization !'s, we can check for parenthesis and
  // brack arguments, then finally for the function opening curly brace, and
  // set inside_end to the end curly brace

  if (source_tokens[start].value == "(") {
    auto params_are_valid = fill_params(")", temp.paren_params, source_tokens,
                                        start /*this should be a int&*/,
                                        end); // assumes no interfering parens
    if (!params_are_valid) {
      return std::unexpected(params_are_valid.error());
    }

    token_is_valid = validate_token(source_tokens, start, end,
                                    std::array<lex::type, 1>{lex::type::SYMBOL},
                                    std::array<std::string, 3>{"[", "->", "{"});
    if (!token_is_valid) {
      return std::unexpected(token_is_valid.error());
    }

    // now start should be right after the ending parenthesis, either on
    // bracket or arrow or curly brace
  }

  if (source_tokens[start].value == "[") {
    auto params_are_valid = fill_params("]", temp.brack_params, source_tokens,
                                        start /*this should be a int&*/, end);
    if (!params_are_valid) {
      return std::unexpected(params_are_valid.error());
    }

    token_is_valid = validate_token(source_tokens, start, end,
                                    std::array<lex::type, 1>{lex::type::SYMBOL},
                                    std::array<std::string, 2>{"->", "{"});
    if (!token_is_valid) {
      return std::unexpected(token_is_valid.error());
    }
  }

  if (source_tokens[start].value == "->") {
    start++;
    token_is_valid = validate_token(source_tokens, start, end,
                                    std::array<lex::type, 1>{lex::type::TEXT},
                                    std::array<std::string, 0>{});
    if (!token_is_valid) {
      return std::unexpected(token_is_valid.error());
    }

    temp.result.name = source_tokens[start].value;

    start++;
    token_is_valid = validate_token(source_tokens, start, end,
                                    std::array<lex::type, 1>{lex::type::SYMBOL},
                                    std::array<std::string, 1>{":"});
    if (!token_is_valid) {
      return std::unexpected(token_is_valid.error());
    }

    auto types_are_valid =
        fill_types(std::array<std::string, 1>{"{"}, temp.result.types,
                   source_tokens, start, end);
    if (!types_are_valid) {
      return std::unexpected(types_are_valid.error());
    }
  }

  // now it should be on the brace entering the function
  start++;
  // now we have to figure out how to find new_end here
  int open_brackets = 1;
  for (int i = 0; i < end; i++) {
    if (source_tokens[i] == lex::token{lex::type::SYMBOL, "}"}) {
      open_brackets--;
      if (open_brackets == 0) {
        new_end = i;
        break;
      }
    } else if (source_tokens[i] == lex::token{lex::type::SYMBOL, "{"}) {
      open_brackets++;
    }
  }

  if (open_brackets != 0) {
    return std::unexpected(
        error{error_code::UNEXPECTED_EOF, source_tokens[start - 1].line});
    // so that it's the line of the unclosed brace that gets the error
  }

  if (start == end) {
    return std::unexpected(
        error{error_code::UNEXPECTED_TOKEN, source_tokens[end].line});
  }

  return temp;
}

std::expected<info, error> analyze(std::vector<lex::token> &source_tokens,
                                   int &start, int end, int &new_end) {
  info result;
  if (source_tokens[start].type == lex::type::NEWLINE) {
    start++; // we know newline can't be eof
  }

  // TODO: implement function filling out an ast definition, placing index
  // right after it, and moving new_end to the end of the block

  auto token_is_valid = validate_token(
      source_tokens, start, end, std::array<lex::type, 1>{lex::type::TEXT},
      std::array<std::string, 0>{});
  if (!token_is_valid)
    return std::unexpected(token_is_valid.error());
  if (source_tokens[start] ==
      lex::token{lex::type::TEXT, "fn"}) { // if its a function def token
    auto analysis_result = analyze_fn(source_tokens, start, end, new_end);
    if (!analysis_result) {
      return std::unexpected(analysis_result.error());
    }

    result = *analysis_result;
  } else if () {
  }

  return result;
}

std::expected<void, error> generate_ast(ast_node *root,
                                        std::vector<lex::token> &source_tokens,
                                        int start, int end) {
  if (start >= end) {
    root->info = none{};
    return {};
  }

  int inside_end;
  auto analyze_result = analyze(source_tokens, start, end, inside_end);
  if (!analyze_result) {
    return std::unexpected(analyze_result.error());
  }

  root->info = *analyze_result;
  // now start will become the start for the inside
  root->inside = new ast_node{};
  auto result = generate_ast(root->inside, source_tokens, start, inside_end);
  if (!result) {
    return std::unexpected(result.error());
  }

  root->outside = new ast_node{};
  result = generate_ast(root->outside, source_tokens, inside_end + 1, end);
  if (!result) {
    return std::unexpected(result.error());
  }

  return {};
}

} // namespace ast
