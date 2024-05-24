JSONLIB = ./jsonlib/cJSON.c
SOCKET = ./src/server/socket/sockethelper.cpp
SERVER_FILES = ./src/server/servermain.cpp  $(CHESS_FUNC)
CLIENT_FILES = ./src/client/clientmain.cpp ./src/client/clientlocalgame.cpp ./src/client/clientonlinegame.cpp $(CHESS_FUNC)
DEV_FILES = ./src/dev_mode.cpp
CHESS_FUNC = ./src/chessfunctionality.cpp ./src/terminal-io/terminal.cpp ./src/client-rand-string/generate.cpp
ZERO_OPTIMIZATION = -O0

#test
test:
	clang++ main.cpp -g -o ./a.out 
	#clang++ main.cpp -g -o test.out -lSDL2 -lSDL2_ttf

	./a.out

#build run
buru: game
	./build/chess.out

#build server
tserver:
	clang++ $(DEV_FILES) $(SERVER_FILES) $(SOCKET) -o ./build/serverchess.out -g $(ZERO_OPTIMIZATION) -D SERVER_TERMINAL
	clang++ ./src/server/server-terminal-communication/fifowriter.cpp -o ./build/server-terminal.out

#build server without terminal
ntserver:
	clang++ $(DEV_FILES) $(SERVER_FILES) $(SOCKET) -o ./build/serverchess.out -g $(ZERO_OPTIMIZATION)
	
terminal:
	clang++ ./src/server/server-terminal-communication/fifowriter.cpp ./src/terminal-io/terminal.cpp -o ./build/server-terminal.out


client:
	clang++ $(DEV_FILES) $(CLIENT_FILES) $(SOCKET) -o ./build/chess.out -g $(ZERO_OPTIMIZATION)

# same as game
all: game
	
#build game (took out server for now)
game: client tserver

clean:
	rm -rf ./build/chess.out
	rm -rf ./build/serverchess.out

