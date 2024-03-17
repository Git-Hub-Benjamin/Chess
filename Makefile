br: game
	./build/chess.out

game:
	# g++ ChessMain.cpp chesslib.cpp -o ./chess.out -g -O0
	clang++ ChessMain.cpp chesslib.cpp -o ./chess.out -g -O0

clean:
	rm -rf ./build/chess.out

