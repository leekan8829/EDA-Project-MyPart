all:main.cpp
	g++ -static main.cpp -L/home/kk/桌面/eda/PA5 -lhmetis -m32 -o par
clean:
	rm -f par