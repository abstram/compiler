module;
#include <expected>
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

struct param {
  std::string name;
  std::vector<std::string> types;
};

enum class opt_level { NONE, WINDOWED_STOKE, FULL_STOKE };

struct function_definition { // not finished

  std::string name;

  std::vector<param> paren_params;
  std::vector<param> brack_params;
  param result;

  opt_level optimization; // 0, 1, 2
};

struct struct_definition { // not finished

  std::string name;
};

struct union_definition { // not finished

  std::string name;
};

struct leaf {};

using info = std::variant<function_definition, struct_definition,
                          union_definition, leaf>;

struct ast_node {
  info info;
  ast_node *inside;
  ast_node *outside;
};

std::expected<info, error> analyze(std::vector<lex::token> &source_tokens,
                                   int &start, int end, int &new_end) {
  info result;

  // TODO: implement function filling out an ast definition, placing index
  // right after it, and moving new_end to the end of the block

  if (source_tokens[start] == lex::token{lex::type::TEXT, "fn"}) {
    function_definition temp;

    start++;
    if (start >= end) {
      return std::unexpected(
          error{error_code::UNEXPECTED_EOF, source_tokens[start].line});
    }

    if (source_tokens[start].type != lex::type::TEXT)
      return std::unexpected(
          error{error_code::UNEXPECTED_TOKEN, source_tokens[start].line});

    temp.name = source_tokens[start].value;

    start++;

    // TODO: abstract out these two error checks!
    if (start >= end) {
      return std::unexpected(
          error{error_code::UNEXPECTED_EOF, source_tokens[start].line});
    }

    if (source_tokens[start].type != lex::type::SYMBOL) {
      return std::unexpected(
          error{error_code::UNEXPECTED_TOKEN, source_tokens[start].line});
    }
  }

  return result;
}

std::expected<void, error> generate_ast(ast_node *root,
                                        std::vector<lex::token> &source_tokens,
                                        int start, int end) {
  if (start >= end) {
    root->info = leaf{};
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
