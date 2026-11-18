module;
#include <expected>
#include <string>
#include <variant>
#include <vector>

export module compiler.ast;
import compiler.lex;

export namespace ast {
enum class error {};

struct function_definition { // not finished

  std::string name;
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

info analyze(std::vector<lex::token> &source_tokens, int &start, int &new_end) {
  info result;

  // TODO: implement function filling out an ast definition, placing index
  // right after it, and moving new_end to the end of the block

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
  root->info = analyze(source_tokens, start, inside_end);
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
