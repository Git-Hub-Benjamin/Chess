SOCKET = ./src/socket/sockethelper.cpp
SERVER_FILES = ./src/server/ServerMain.cpp ./src/server/OnlineChessGame.cpp ./src/server/StandardServerOnlineChessGame.cpp ./src/Chess/ChessGame/text-piece-art.cpp ./src/server/WChessServer.cpp $(CHESS_FUNC)
CLIENT_FILES = ./src/client/online-game/clientonlinegame.cpp ./src/client/client-terminal-frontend/displaymanager.cpp ./src/client/client-text-graphics/textgraphic.cpp ./src/client/online-game/connecting-to-online.cpp ./src/Chess/ChessGame/text-piece-art.cpp ./src/Chess/ChessGame/StandardLocalChessGame.cpp ./src/client/online-game/StandardOnlineChessGame.cpp ./src/client/tui/clientoption.cpp $(CHESS_FUNC)
DEV_FILES = ./src/dev_mode.cpp
CHESS_FUNC = ./src/chessfunctionality.cpp ./src/Chess/ChessGame/StandardChessGame.cpp ./src/terminal-io/terminal.cpp ./src/client-rand-string/generate.cpp
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
tserver: terminal
	clang++ $(DEV_FILES) $(SERVER_FILES) $(SOCKET) -o ./build/serverchess.out -g $(ZERO_OPTIMIZATION) -D SERVER_TERMINAL


#build server without terminal
ntserver:
	clang++ $(DEV_FILES) $(SERVER_FILES) $(SOCKET) -o ./build/serverchess.out -g $(ZERO_OPTIMIZATION)
	
terminal:
	clang++ ./src/server/server-terminal-communication/fifowriter.cpp ./src/terminal-io/terminal.cpp -o ./build/server-terminal.out

tclient:
	clang++ $(DEV_FILES) $(CLIENT_FILES) ./src/client/tui/tuimain.cpp  $(SOCKET) -o ./build/tchess.out -g $(ZERO_OPTIMIZATION)

gclient:
	clang++ $(DEV_FILES) $(CLIENT_FILES) ./src/client/gui/guimain.cpp  $(SOCKET) -o ./build/gchess.out -g $(ZERO_OPTIMIZATION)

# same as game
all: game
	
#build game (took out server for now)
game: tclient gclient tserver
	
clean:
	rm -rf ./build/chess.out
	rm -rf ./build/serverchess.out

