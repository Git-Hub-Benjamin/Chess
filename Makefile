JSONLIB = ./jsonlib/cJSON.c
SOCKET = ./src/server/socket/sockethelper.cpp
SERVER_FILES = ./src/server/servermain.cpp ./src/chessfunctionality.cpp #$(JSONLIB)
CLIENT_FILES = ./src/client/clientmain.cpp ./src/client/clientlocalgame.cpp ./src/chessfunctionality.cpp ./src/client/clientonlinegame.cpp
ZERO_OPTIMIZATION = -O0

#test
test:
	clang++ main.cpp -g -o ./a.out -lncurses ./src/chessfunctionality.cpp
	#clang++ main.cpp -g -o test.out -lSDL2 -lSDL2_ttf

	./a.out

#build run
buru: game
	./build/chess.out

#build server
server:
	clang++ $(SERVER_FILES) $(SOCKET) -o ./build/serverchess.out -g $(ZERO_OPTIMIZATION)

client:
	clang++ $(CLIENT_FILES) $(SOCKET) -o ./build/chess.out -g $(ZERO_OPTIMIZATION)

# same as game
all: game
	./build/chess.out
#build game
game: server client

clean:
	rm -rf ./build/chess.out
	rm -rf ./build/serverchess.out
