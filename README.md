# working-with-threads

In this project we are going to design a price predictor with two algorithms, serial and parallel .
there are two files in assets folder.
train.csv and weights.csv. the train.csv file includes informations about cars and the weight.csv file gives us the information about how much impact every feature has on the price of the cars.
in serial algorithm we read the data file and calculate everything in order and with no parallelism but in parallel algorithm, we split the input file into 4 files and all the calculation occurs in parallel using threads of the system

at last if we compare the time each algorithm takes to solve the problem, we can see that the parallel algorithm runs about 4 times faster than the serial algorithm.
