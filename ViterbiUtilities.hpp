/*
 * ViterbiUtilities.hpp
 *
 *  Created on: Jun 7, 2015
 *      Author: hekpotexhapb
 */

#ifndef VITERBIUTILITIES_HPP_
#define VITERBIUTILITIES_HPP_
#include "Viterbi.hpp"

template<typename T>
double emissionFunction(const std::vector<std::vector<T> > &emissionMatrix,
		const State toState, const Probability prevObservation);


std::pair<Probability, PrevState> findMostProbablePrevState(
		const std::vector<Probability> &prevCol,
		const std::vector<std::vector<Probability> > &transitionMatrix,
		const std::vector<std::vector<Probability> > &emissionMatrix,
		const Probability prevObservation, const State currentPossibleState) ;

std::pair<std::vector<Probability>, std::vector<PrevState> > yieldNewColRange(
		const std::vector<Probability> &prevCol, // previous column probabilities
		const std::vector<std::vector<Probability> > &transitionMatrix,
		const std::vector<std::vector<Probability> > &emissionMatrix,
		const Probability prevObservation, const Index from, const int size);

std::vector<std::pair<Probability, State>> backTrack(
		std::vector<std::vector<std::pair<Probability, State> > > viterbiResultMatrix,
		State startingBacktrack);


#endif /* VITERBIUTILITIES_HPP_ */
