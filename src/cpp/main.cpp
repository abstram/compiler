#include <fstream>
#include <iostream>
#include <string>
#include <vector>
import compiler.lex;
import compiler.ast;

int main(int argc, char **argv) {
  if (argc < 1) {
    std::cerr << "Failed: requires file argument\n";
    return EXIT_FAILURE;
  }

  std::ifstream source(argv[1]);
  std::string line;
  std::vector<lex::token> source_tokens;
  int line_number = 0;

  while (std::getline(source, line)) {
    line_number++;
    auto lex_result = lex::lex(source_tokens, line, line_number);
    if (!lex_result) {
      switch (lex_result.error()) {
      case lex::error::NUMBER_BEFORE_TEXT:
        std::cerr << "Number before text lex error on line " << line_number
                  << ".\n";
        return EXIT_FAILURE;

      default:
        std::cerr << "Unknown error\n";
        return EXIT_FAILURE;
      }
    }
  }

  source_tokens.pop_back(); // getting rid of last newline

  // to test lex: lex::print_tokens(source_tokens);

  ast::ast_node *root = new ast::ast_node{};
  int start = 0;
  while (source_tokens[start].type == lex::type::NEWLINE)
    start++;

  int end = source_tokens.size();
  auto ast_result = ast::generate_ast(root, source_tokens, start, end);

  if (!ast_result) {
    switch (ast_result.error().code) {
      // TODO: handle AST errors
    default:
      std::cerr << "Unknown error\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
