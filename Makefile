SOCKET = ./src/socket/sockethelper.cpp

SERVER_FILES = ./src/server/ServerMain.cpp \
               ./src/server/OnlineChessGame.cpp \
               ./src/server/StandardServerOnlineChessGame.cpp \
               ./src/Util/Terminal/TextPieceArt.cpp \
               ./src/server/WChessServer.cpp \

CLIENT_FILES = ./src/Client/online-game/clientonlinegame.cpp \
               ./src/Client/client-terminal-frontend/displaymanager.cpp \
               ./src/Client/online-game/connecting-to-online.cpp \
               ./src/Client/online-game/StandardOnlineChessGame.cpp \
               ./src/Client/Options/Options.cpp 

LOCAL_FILES = ./src/Client/Tui/WChessTerminalMain.cpp \
			  ./src/Client/ConfigFile/ConfigFile.cpp \
              ./src/Client/Options/Options.cpp \
              ./src/Util/Terminal/TextGraphics.cpp \
              ./src/Util/Terminal/Terminal.cpp 

# Updated paths for Chess-related source files
CHESS_FILES = ./src/Util/Terminal/TextPieceArt.cpp \
			  ./src/Chess/LegacyArray/LStandardLocalChessGame.cpp \
              ./src/Chess/GameLogic/StandardLocalChessGame.cpp \
              ./src/Chess/GameLogic/StandardChessGame.cpp \
              ./src/Chess/Utils/ChessHelperFunctions.cpp 

COMMON_FILES = ./src/Util/client-rand-string/RandomString.cpp 

DEV_FILES = ./src/Chess/Dev/dev_mode.cpp # Path seems unchanged

# flags
ZERO_OPTIMIZATION = -O0

#  macros
LEGACY_GAMEBOARD = -D LEGACY_ARRAY_GAMEBOARD
WINDOWS_DEF = -D _WIN32
LINUX_DEF = -D ___linux____
MAC_DEF = -D __APPLE__
CLIENT_COMPILE_ONLINE = -D COMPILE_ONLINE

#test
test:
	g++ -I./src main.cpp ./src/Util/Terminal/Terminal.cpp ./src/Util/Terminal/TextPieceArt.cpp -D _WIN32 -g

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
	@echo "Building w/o legacy gameboard"
	g++ -I./src $(DEV_FILES) $(LOCAL_FILES) $(CHESS_FILES) $(COMMON_FILES) -o ./build/tchess.exe -g $(ZERO_OPTIMIZATION) $(WINDOWS_DEF)

wintcliento:
	clang++ -I./src $(DEV_FILES) $(CLIENT_FILES) $(CHESS_FILES) $(COMMON_FILES) $(SOCKET) -o ./build/tchess.out -g $(ZERO_OPTIMIZATION) -D _WIN32 -lws2_32 -lmingw32


lintclient:
	clang++ $(DEV_FILES) $(CLIENT_FILES) ./src/client/Tui/WChessTerminalMain.cpp  $(SOCKET) -o ./build/tchess.out -g $(ZERO_OPTIMIZATION) -D __linux__

gclient:
	clang++ $(DEV_FILES) $(CLIENT_FILES) ./src/client/gui/guimain.cpp  $(SOCKET) -o ./build/gchess.out -g $(ZERO_OPTIMIZATION)

# same as game
all: game
	
#build game (took out server for now)
game: wintclient gclient tserver

windowsclient:
	g++ -I./src ./src/client/Tui/WChessTerminalMain.cpp ./src/Chess/ChessGame/text-piece-art.cpp ./src/client/client-text-graphics/textgraphic.cpp ./src/Chess/ChessGame/StandardChessGame.cpp ./src/dev_mode.cpp ./src/Chess/ChessGame/StandardLocalChessGame.cpp ./src/terminal-io/terminal.cpp ./src/client/Tui/clientoption.cpp ./src/chess-helper-functionality.cpp -o ./build/tchess.exe $(WINDOWS_DEF) -g
	
clean:
	rm -rf ./build/chess.out
	rm -rf ./build/serverchess.out

