compiler:
	mkdir -p build
	clang++ -std=c++26 -x c++-module src/cpp/lex.cpp --precompile -o build/lex.pcm
	clang++ -std=c++26 -c build/lex.pcm -o build/lex.o
	clang++ -std=c++26 -x c++-module src/cpp/ast.cpp --precompile -fmodule-file=compiler.lex=build/lex.pcm -o build/ast.pcm
	clang++ -std=c++26 -c build/ast.pcm -fmodule-file=compiler.lex=build/lex.pcm -o build/ast.o
	clang++ -std=c++26 -c src/cpp/main.cpp -fmodule-file=compiler.lex=build/lex.pcm -fmodule-file=compiler.ast=build/ast.pcm -o build/main.o
	clang++ build/main.o build/lex.o build/ast.o -o build/main

clean:
	rm -rf build/
