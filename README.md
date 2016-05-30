MPI+OpenMP Implementation of Viterbi Algorithm 

Files:
	Main.cpp – main file includes master , slave and main functions
	FileUtilities – module consist of miscellaneous function for pretty printing , debuging and file IO
	MPIUtilities – functions for synchronization of data via MPI interface
	ViterbiUtilities – Viterbi algorithm functions
	Viterbi.hpp – external libraries and typedef's used in code 
	

Implementation of algorithm is able to run as sequential algorithm and concurrent scalable algorithm as well.
Implementation has two main node types: 
Master node(only one per run) , 
Slave node (can vary from non to any amount that can divide calculated column by equally sized chunks).
Master recreates 2d matricies and vectors according to the argument number and sends them to other nodes if they are exists.
Calculated column divided to chunks with the size (column size)/(ranks amount) 
so it's user's responsibility to set number of running processes wisely.
Each process calculates responsble to calculate part of column according to it's rank and chunk size.
After calculation: all chunks shared between all processes and assembled to a new column vector
that will be used to calculate next iteration.
To reduce network activity only probability chunks are shared between all processes. 
Most probable previous states chunks sended to Master node only.


Each iteration Master adds to a viterbi matrix of a probabilities and previous states a new calculated column.
On the end of columns calculations master will backtrack most probable path.

Key functions of this implementation are: 
  synchronizeAndAssemblyChunks – function that receive process's chunk and returns a synchronized vector
  of all other chunks + sended one.
	YieldNewColRange – function that will yield a new chunk according to specified range and previous column.
	FindMostProbablePrevState – the heart of algorithm that will calculate a most probable previous state for given possible transition.
	FindMostProbablePrevState – the heart of algorithm that will calculate a most probable previous state for given possible transition.

Complexity of this implementation is:

	O(observations*(colSize/numOfProcs)*(colSize/numOfThreads))

Other issues:

	Algorithm was implemented and tested In eclipse Luna Service Release 2 	(4.4.2) in ArchLinux kernel: linux 3.19.3-3
