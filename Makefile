compiler:
	mkdir -p build
	clang++ -std=c++26 -x c++-module src/cpp/lex.cpp --precompile -o build/lex.pcm
	clang++ -std=c++26 -c build/lex.pcm -o build/lex.o
	clang++ -std=c++26 -c src/cpp/main.cpp -fmodule-file=compiler.lex=build/lex.pcm -o build/main.o
	clang++ build/main.o build/lex.o -o build/main

clean:
	rm -rf build/
