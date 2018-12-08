# SFML (and other) libraries that need to be linked with
LIBS=-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lstdc++

# Create the executable file
game: src/main.o
	clang++ src/*.cpp -Isrc -Isrc/json -o bin/game_linux  -std=c++11 $(LIBS)