//#include "Viterbi.hpp"

#include "FileUtilities.hpp"
#include "MPIUtilities.hpp"
#include "ViterbiUtilities.hpp"



std::vector<double> sendBuffer(3001000);
std::vector<double> recvBuffer(3001000);

std::vector<std::pair<Probability, State>> master(const int transitionRows,
		const int transitionCols, const std::string &transitionMatrixFileName,
		const int emissionRows, const int emissionCols,
		const std::string &emissionMatrixFileName,
		const std::string &observationsFileName,
		const std::string &outputFileName) {
//unpack needed data from file
	std::vector<std::vector<Probability> > transitionMatrix =
			extractMatrixFromFile(transitionRows, transitionCols,
					transitionMatrixFileName);

	std::vector<std::vector<Probability> > emissionMatrix =
			extractMatrixFromFile(emissionRows,emissionCols,emissionMatrixFileName);

	std::vector<Probability> observations = extractVectorFromFile(
			observationsFileName);


//generators for debugging
//	std::vector<std::vector<Probability> > transitionMatrix = generateMatrix(
//			1000, 1000, 0, 1);
//
//	std::vector<std::vector<Probability> > emissionMatrix = generateMatrix(1000,
//			2, 0, 1);
//
//	std::vector<Probability> observations = generateVector(3000, 0, 1);

	//broadcasting data
	broadcastMatrix(sendBuffer, transitionMatrix, 0,MPI_DOUBLE);

	broadcastMatrix(sendBuffer, emissionMatrix, 0,MPI_DOUBLE);

	broadcastVector(sendBuffer, 0, observations, MPI_DOUBLE);

//initialize history vector
	std::vector<std::vector<std::pair<Probability, State>> > historyVector(
			observations.size());

	int ranksSize;

	std::pair<std::vector<Probability>, std::vector<State> > generatedChunk;

	MPI_Comm_size(MPI_COMM_WORLD, &ranksSize);

	std::vector<Probability> prevColProbabilities;

	int chunkSize = (int) (transitionMatrix.size() / ranksSize);

	generatedChunk.first = std::vector<Probability>(chunkSize);
//initial generated chunk
	std::fill(generatedChunk.first.begin(), generatedChunk.first.end(), 1);

	for (auto i = 0; i < observations.size(); i++) {

		//std::cout << "running iteration:" << i << std::endl;

		//sync chunks between all processes
		prevColProbabilities = synchronizeAndAssemblyChunks(sendBuffer,
				recvBuffer, i, 0, generatedChunk.first, MPI_DOUBLE);

		//generating a new chunk according to rank number
		generatedChunk = yieldNewColRange(prevColProbabilities,
				transitionMatrix, emissionMatrix, observations[i], 0,
				chunkSize);

		std::vector<int> prevColStates(prevColProbabilities.size());
		std::copy(generatedChunk.second.begin(), generatedChunk.second.end(),
				prevColStates.begin());

//receive prevStates by indecies
		for (int j = 1; j < ranksSize; j++) {
			MPI_Recv(&(prevColStates.begin() + j * chunkSize)[0], chunkSize,
			MPI_INT, j, i, MPI_COMM_WORLD, MPI_SUCCESS);

		}

		//printVector(prevColStates, "prevColStates:");

		std::vector<std::pair<Probability, PrevState>> observationCol(
				prevColStates.size());
//merge probabilities and relevant states
		for (int i = 0; i < observationCol.size(); i++)
			observationCol[i] = std::make_pair(prevColProbabilities[i],
					prevColStates[i]);
		historyVector[i] = observationCol;

	}

	std::pair<Probability, State> maximum = std::make_pair(0.0, 0);

//find maximum in last column
//FIXED:
#pragma omp parallel
	{
		std::pair<Probability, State> maximum_private = std::make_pair(0.0, 0);
#pragma omp parallel for private(maximum_private)
		for (auto i = 0; i < prevColProbabilities.size(); i++)
			if (prevColProbabilities[i] > maximum_private.first)
				maximum_private = std::make_pair(prevColProbabilities[i], i);

#pragma omp flush(maximum)
		if (maximum_private.first < maximum.first) {
#pragma omp critical
			{
				if (maximum_private.first < maximum.first)
					maximum = maximum_private;
			}
		}

	}

	printHistoryMatrix<double, int>(historyVector,"history vector at the end:");

//create most probable path
	std::vector<std::pair<Probability, State>> result = backTrack(historyVector,
			maximum.second);

	cout << "result is:" << endl;

	for (int i = 0; i < result.size(); i++)
		cout << "<\t" << result[i].first << "\t,\t" << result[i].second
				<< "\t> ";

	finalOutput(outputFileName, result);

	return result;
}

void slave(int rank) {

	std::string debugMsg;


	//init
	std::vector<std::vector<double> > transitionMatrix = broadcastMatrix(
			sendBuffer, std::vector<std::vector<double>>(0), 0,
			MPI_DOUBLE);

	std::vector<std::vector<double> > emissionMatrix = broadcastMatrix(
			sendBuffer, std::vector<std::vector<double>>(0), 0,
			MPI_DOUBLE);

	std::vector<Probability> observations = broadcastVector(recvBuffer, 0,
			std::vector<Probability>(0), MPI_DOUBLE);

	std::vector<std::pair<Probability, PrevState>> chunk;
	std::vector<std::pair<Probability, PrevState> > prevCol(0);

	int ranksSize;
	std::pair<std::vector<Probability>, std::vector<State> > generatedChunk;

	MPI_Comm_size(MPI_COMM_WORLD, &ranksSize);

	std::vector<Probability> prevColProbabilities(0);
	int chunkSize = (int) (transitionMatrix.size() / ranksSize);

	generatedChunk.first = std::vector<Probability>(chunkSize);

	//fill very first chunk by 1
	std::fill(generatedChunk.first.begin(), generatedChunk.first.end(), 1);

	for (auto i = 0; i < observations.size(); i++) {

		prevColProbabilities = synchronizeAndAssemblyChunks(sendBuffer,
				recvBuffer, i, rank, generatedChunk.first, MPI_DOUBLE);

		generatedChunk = yieldNewColRange(prevColProbabilities,
				transitionMatrix, emissionMatrix, observations[i],
				rank * chunkSize, chunkSize);

		MPI_Send(&generatedChunk.second.front(), generatedChunk.second.size(),
		MPI_INT, 0, i,
		MPI_COMM_WORLD);

	}//nothing to process - die

	return;
}

int main(int argc, char *argv[]) {
	int rank, size, i;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	std::vector<std::string> allArgs(argv, argv + argc);//console args

	if (rank == 0)//passing console args
		master(std::atoi(&(allArgs[1])[0]), std::atoi(&(allArgs[2])[0]),
				allArgs[3], std::atoi(&(allArgs[4])[0]),
				std::atoi(&(allArgs[5])[0]), allArgs[6], allArgs[7],
				allArgs[8]);
	else
		slave(rank);

	MPI_Finalize();
	return 0;
}

