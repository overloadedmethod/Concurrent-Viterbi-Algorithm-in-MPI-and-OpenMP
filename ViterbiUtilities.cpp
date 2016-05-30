/*
 * ViterbiUtilities.cpp
 *
 *  Created on: Jun 7, 2015
 *      Author: hekpotexhapb
 */

#include "ViterbiUtilities.hpp"


template<typename T>
double emissionFunction(const std::vector<std::vector<T>> &emissionMatrix,
		const State toState, const Probability prevObservation) {
	//aj exp(-(oi - bj)2)
	double power = prevObservation - emissionMatrix[toState][1];
	return emissionMatrix[toState][0] * exp(-(power * power));
}

std::vector<std::pair<Probability, State>> backTrack(
		std::vector<std::vector<std::pair<Probability, State> > > viterbiResultMatrix,
		State startingBacktrack) {
	std::vector<std::pair<Probability, State>> result(
			viterbiResultMatrix.size());
	std::pair<Probability, State> backtrack;

	for (int i = viterbiResultMatrix.size() - 1; i > -1; i--) {
		backtrack = viterbiResultMatrix[i][startingBacktrack];
//		std::cout << " < " << backtrack.first << " , " << backtrack.second
//				<< " > ";
		result[i] = backtrack;
		startingBacktrack = backtrack.second;
	}
	std::cout << std::endl;
	return result;
}



std::pair<Probability, PrevState> findMostProbablePrevState(
		const std::vector<Probability> &prevCol,
		const std::vector<std::vector<Probability> > &transitionMatrix,
		const std::vector<std::vector<Probability> > &emissionMatrix,
		const Probability prevObservation, const State currentPossibleState) {

	int prevPossibleState = 0;
	double currentPrevStateCandidate;

	double emission = emissionFunction(emissionMatrix, currentPossibleState,
			prevObservation);
	std::pair<Probability, PrevState> mostProbablePrevState;

	//FIXED:fixed after pointing on wrong usage of critical section
#pragma omp parallel
	{
		std::pair<Probability, PrevState> mostProbablePrevStatePrivate =
				std::make_pair(0.0, 0);
#pragma omp parallel for private(currentPrevStateCandidate,mostProbablePrevStatePrivate)
		for (prevPossibleState = 0; prevPossibleState < prevCol.size();
				prevPossibleState++) {
			currentPrevStateCandidate = prevCol[prevPossibleState] * emission
					* transitionMatrix[prevPossibleState][currentPossibleState];
			//[row][col] dm,i+1 = max[dm,1*t1,m*f(1, i),   dm,2*t2,m*f(2, i),  dm,3*t3,m*f(3, i), …, dm,N*tN,m*f(N, i)]

			if (currentPrevStateCandidate > mostProbablePrevStatePrivate.first)
				mostProbablePrevStatePrivate = std::make_pair(
						currentPrevStateCandidate, prevPossibleState);
#pragma omp flush(mostProbablePrevState)
			if (mostProbablePrevStatePrivate.first > mostProbablePrevState.first) {
#pragma omp critical
				{
					if (mostProbablePrevStatePrivate.first
							> mostProbablePrevState.first)
						mostProbablePrevState = mostProbablePrevStatePrivate;
				}
			}
		}
	}
//	std::cout << "<" << mostProbablePrevState.first << ","
//			<< mostProbablePrevState.second << ">" << std::endl;
	return mostProbablePrevState;
}

std::pair<std::vector<Probability>, std::vector<PrevState>> yieldNewColRange(
		const std::vector<Probability> &prevCol, // previous column probabilities
		const std::vector<std::vector<Probability> > &transitionMatrix,
		const std::vector<std::vector<Probability> > &emissionMatrix,
		const Probability prevObservation, const Index from, const int size) {//generate range from Index of some size

	std::pair<Probability, PrevState> cellResult;
	std::pair<std::vector<Probability>, std::vector<PrevState>> newColChunk;
	newColChunk.first = std::vector<Probability>(size);
	newColChunk.second = std::vector<PrevState>(size);

#pragma omp parallel for//just loop over possibleStates matrix
	for (int currentPossibleState = 0; currentPossibleState < size;
			currentPossibleState++) {

		cellResult = findMostProbablePrevState(prevCol, transitionMatrix,//calculate for each most probable predecessor
				emissionMatrix, prevObservation, from + currentPossibleState);

		newColChunk.first[currentPossibleState] = cellResult.first;
		newColChunk.second[currentPossibleState] = cellResult.second;
	}
	return newColChunk;
}


