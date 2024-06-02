SOCKET = ./src/socket/sockethelper.cpp
SERVER_FILES = ./src/server/servermain.cpp ./src/server/servergame.cpp $(CHESS_FUNC)
CLIENT_FILES = ./src/client/clientmain.cpp ./src/client/online-game/clientonlinegame.cpp ./src/client/client-terminal-frontend/displaymanager.cpp ./src/client/client-text-graphics/textgraphic.cpp ./src/client/online-game/connecting-to-online.cpp ./src/client/online-game/online-game-loop.cpp ./src/Chess/ChessGame/text-piece-art.cpp $(CHESS_FUNC)
DEV_FILES = ./src/dev_mode.cpp
CHESS_FUNC = ./src/chessfunctionality.cpp ./src/Chess/ChessGame/Standard_ChessGame.cpp ./src/terminal-io/terminal.cpp ./src/client-rand-string/generate.cpp
ZERO_OPTIMIZATION = -O0

#test
test:
	clang++ main.cpp -g -o ./a.out -lncurses
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

