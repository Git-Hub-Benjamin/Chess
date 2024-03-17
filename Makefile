br: game
	./build/chess.out

game:
	g++ ChessMain.cpp chesslib.cpp -o ./build/chess.out -g

clean:
	rm -rf ./build/chess.out

