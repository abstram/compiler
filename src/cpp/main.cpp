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

  // test lex
  // for (int i = 0; i < source_tokens.size(); i++) {
  //   std::cout << lex::token_to_string(source_tokens[i]) << '\n';
  //   if (i < (source_tokens.size() - 1)) {
  //     std::cout << '\n';
  //   }
  // }

  ast::ast_node *root = new ast::ast_node{};
  int index = 0;
  auto ast_result = ast::generate_ast(root, source_tokens, index);

  if (!ast_result) {
    switch (ast_result.error()) {
    default:
      std::cerr << "Unknown error\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
