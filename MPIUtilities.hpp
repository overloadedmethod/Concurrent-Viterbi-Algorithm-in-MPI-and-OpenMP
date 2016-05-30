/*
 * MPIUtilities.hpp
 *
 *  Created on: Jun 7, 2015
 *      Author: hekpotexhapb
 */
///////////////////////////////////////////////////////////
///code moved here because of linker issues - it have some problem with template functions
///////////////////////////////////////////////////////////
#pragma once
#ifndef MPIUTILITIES_HPP_
#define MPIUTILITIES_HPP_

#include "Viterbi.hpp"


//simply takes some vector and broadcast it among others
template<typename T, typename K>
std::vector<T> broadcastVector(std::vector<K> &buffer, const int senderRank,
		const std::vector<T> sendedVector, MPI_Datatype MPI_TYPE) {
	int unpackSize;
	int packPosition = 0;
	int callerRank;
	std::vector<T> result = sendedVector;

	MPI_Comm_size(MPI_COMM_WORLD, &unpackSize);

	//less than 2 nodes?nothing to send
	if (unpackSize < 2)
		return sendedVector;

	if (sendedVector.size() > 0) {
		int packedVectorSize = sendedVector.size();
		//pack vector size
		MPI_Pack(&packedVectorSize, 1, MPI_INT, &buffer.front(),
				buffer.size() * sizeof(K), &packPosition,
				MPI_COMM_WORLD);
		//pack vector itself
		MPI_Pack(&sendedVector.front(), packedVectorSize, MPI_TYPE,
				&buffer.front(), buffer.size() * sizeof(K), &packPosition,
				MPI_COMM_WORLD);

	}

	MPI_Bcast(&buffer.front(), buffer.size() * sizeof(K), MPI_PACKED,
			senderRank, MPI_COMM_WORLD);

	MPI_Comm_rank(MPI_COMM_WORLD, &callerRank);
	if (callerRank != senderRank) {
		packPosition = 0;

		MPI_Unpack(&buffer.front(), buffer.size() * sizeof(K), &packPosition,
				&unpackSize, 1, MPI_INT, MPI_COMM_WORLD);

		result = std::vector<T>(unpackSize);

		MPI_Unpack(&buffer.front(), buffer.size() * sizeof(K), &packPosition,
				&result.front(), unpackSize, MPI_TYPE, MPI_COMM_WORLD);
	}

	return result;
}

//same about matrix
template<typename T, typename K>
std::vector<std::vector<T>> broadcastMatrix(std::vector<K> &buffer,
		const std::vector<std::vector<T>> &matrix, const int senderRank,
		MPI_Datatype MPI_TYPE) {

	int rows = matrix.size();
	int callerRank;

	MPI_Comm_size(MPI_COMM_WORLD, &callerRank);

	if (callerRank < 2)
		return matrix;

	//send how many rows will be sent when each of them is separate vector
	MPI_Bcast(&rows, 1, MPI_INT, senderRank, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Comm_rank(MPI_COMM_WORLD, &callerRank);

	std::vector<std::vector<T>> result(rows);

	for (int i = 0; i < rows; i++) {
		result[i] = broadcastVector(buffer, senderRank,
				(callerRank == senderRank) ? matrix[i] : std::vector<T>(0),//if you are not sending something just put it empty
				MPI_TYPE);
	}
	return result;
}


//each node sends some chunks and recieves vector constructed by others
template<typename T, typename K>
std::vector<T> synchronizeAndAssemblyChunks(std::vector<K> &sendBuffer,
		std::vector<K> &recvBuffer, const int tag, const int senderRank,
		std::vector<T> chunk, MPI_Datatype MPI_TYPE) {
	int packPosition = 0;
	int ranksSize;
	int chunkSize = chunk.size();

	MPI_Comm_size(MPI_COMM_WORLD, &ranksSize);

	std::vector<T> synchronizedVector(chunkSize * ranksSize);

	if (ranksSize < 2) {
		return chunk;
	}
	//pack my current chunk
	MPI_Pack(&chunkSize, 1, MPI_INT, &sendBuffer.front(),
			sendBuffer.size() * sizeof(K), &packPosition,
			MPI_COMM_WORLD);
	MPI_Pack(&chunk.front(), chunkSize, MPI_TYPE, &sendBuffer.front(),
			sendBuffer.size() * sizeof(K), &packPosition,
			MPI_COMM_WORLD);
	//share among others
	for (int i = 0, unpackPosition = 0, unpackSize; i < ranksSize;
			i++, unpackPosition = 0) {
		if (i == senderRank) {
			//if i share with myself - just copy my chunk to previous column synchronized vector
			std::copy(chunk.begin(), chunk.end(),
					synchronizedVector.begin() + senderRank * chunk.size());

		} else {
			unpackPosition = 0;
			MPI_Sendrecv(&sendBuffer.front(), packPosition * sizeof(K),
			MPI_PACKED, i, tag, &recvBuffer.front(), packPosition * sizeof(K),
			MPI_PACKED, i, tag,
			MPI_COMM_WORLD, MPI_SUCCESS);
			//unpack to synchronized vector
			MPI_Unpack(&recvBuffer.front(), recvBuffer.size() * sizeof(K),
					&unpackPosition, &unpackSize, 1, MPI_INT,
					MPI_COMM_WORLD);

			MPI_Unpack(&recvBuffer.front(), recvBuffer.size() * sizeof(K),
					&unpackPosition,
					&synchronizedVector.front() + i * chunkSize, unpackSize,
					MPI_TYPE, MPI_COMM_WORLD);
		}
	}
	return synchronizedVector;
}

#endif /* MPIUTILITIES_HPP_ */
