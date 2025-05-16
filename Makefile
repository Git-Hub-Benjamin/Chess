SOCKET = ./src/socket/sockethelper.cpp
SERVER_FILES = ./src/server/ServerMain.cpp ./src/server/OnlineChessGame.cpp ./src/server/StandardServerOnlineChessGame.cpp ./src/Chess/ChessGame/text-piece-art.cpp ./src/server/WChessServer.cpp $(CHESS_FUNC)
CLIENT_FILES = ./src/client/online-game/clientonlinegame.cpp \
               ./src/client/client-terminal-frontend/displaymanager.cpp \
               ./src/client/client-text-graphics/textgraphic.cpp \
               ./src/client/online-game/connecting-to-online.cpp \
               ./src/client/online-game/StandardOnlineChessGame.cpp \
               ./src/client/tui/clientoption.cpp \
               ./src/client/tui/tuimain.cpp

CHESS_FILES = ./ src/Chess/ChessGame/text-piece-art.cpp \
              ./src/Chess/ChessGame/StandardLocalChessGame.cpp \
              ./src/Chess/ChessGame/StandardChessGame.cpp

COMMON_FILES = ./src/chessfunctionality.cpp \
               ./src/terminal-io/terminal.cpp \
               ./src/client-rand-string/generate.cpp
DEV_FILES = ./src/dev_mode.cpp
CHESS_FUNC = ./src/chessfunctionality.cpp ./src/Chess/ChessGame/StandardChessGame.cpp ./src/terminal-io/terminal.cpp ./src/client-rand-string/generate.cpp
ZERO_OPTIMIZATION = -O0

WINDOWS_DEF = -D _WIN32
LINUX_DEF = -D ___linux____
MAC_DEF = -D __APPLE__
CLIENT_COMPILE_ONLINE = -D COMPILE_ONLINE

#test
test:
	g++ -I./src main.cpp
	./a.out

#build run
buru: game
	./build/chess.out

OS := $(shell uname)

OS := $(shell uname)

ifeq ($(OS),Linux)
	#build server
	tserver: terminal
		clang++ $(DEV_FILES) $(SERVER_FILES) $(SOCKET) -o ./build/serverchess.out -g $(ZERO_OPTIMIZATION) -D SERVER_TERMINAL

	#build server without terminal
	ntserver:
		clang++ $(DEV_FILES) $(SERVER_FILES) $(SOCKET) -o ./build/serverchess.out -g $(ZERO_OPTIMIZATION)
		
	terminal:
		clang++ ./src/server/server-terminal-communication/fifowriter.cpp ./src/terminal-io/terminal.cpp -o ./build/server-terminal.out
else
    tserver:
		@echo "This command can only be run on Linux"
    ntserver:
		@echo "This command can only be run on Linux"
endif


wintclient:
	clang++ -I./src $(DEV_FILES) $(CLIENT_FILES) $(CHESS_FILES) $(COMMON_FILES) $(SOCKET) -o ./build/tchess.exe -g $(ZERO_OPTIMIZATION) $(WINDOWS_DEF) -lws2_32 -lmingw32

wintcliento:
	clang++ -I./src $(DEV_FILES) $(CLIENT_FILES) $(CHESS_FILES) $(COMMON_FILES) $(SOCKET) -o ./build/tchess.out -g $(ZERO_OPTIMIZATION) -D _WIN32 -lws2_32 -lmingw32


lintclient:
	clang++ $(DEV_FILES) $(CLIENT_FILES) ./src/client/tui/tuimain.cpp  $(SOCKET) -o ./build/tchess.out -g $(ZERO_OPTIMIZATION) -D __linux__

gclient:
	clang++ $(DEV_FILES) $(CLIENT_FILES) ./src/client/gui/guimain.cpp  $(SOCKET) -o ./build/gchess.out -g $(ZERO_OPTIMIZATION)

# same as game
all: game
	
#build game (took out server for now)
game: wintclient gclient tserver

windowsclient:
	g++ -I./src ./src/client/tui/tuimain.cpp ./src/Chess/ChessGame/text-piece-art.cpp ./src/client/client-text-graphics/textgraphic.cpp ./src/Chess/ChessGame/StandardChessGame.cpp ./src/dev_mode.cpp ./src/Chess/ChessGame/StandardLocalChessGame.cpp ./src/terminal-io/terminal.cpp ./src/client/tui/clientoption.cpp ./src/chess-helper-functionality.cpp -o ./build/tchess.exe $(WINDOWS_DEF) -g
	
clean:
	rm -rf ./build/chess.out
	rm -rf ./build/serverchess.out

