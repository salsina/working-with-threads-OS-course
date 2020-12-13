PhonePricePrediction.out : main.o
	g++ main.o -o PhonePricePrediction.out -lpthread
main.o: main.cpp
	g++ -std=c++11 main.cpp -o main.o 

.PHONY:clean
clean:
	rm *.o
	rm PhonePricePrediction.out
