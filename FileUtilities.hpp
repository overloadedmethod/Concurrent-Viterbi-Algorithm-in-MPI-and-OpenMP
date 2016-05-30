/*
 * Utilities.hpp
 *
 *  Created on: Jun 7, 2015
 *      Author: hekpotexhapb
 */
#pragma once
#ifndef FILEUTILITIES_HPP_
#define FILEUTILITIES_HPP_

#include "Viterbi.hpp"

void finalOutput(const std::string &fileName,
		const std::vector<std::pair<Probability, State> > &historyVector);

std::vector<double> extractVectorFromFile(const std::string &fileName);

std::vector<std::vector<double>> extractMatrixFromFile(const int rows,
		const int cols, const std::string &fileName);

std::vector<std::vector<double>> extractMatrixFromFile(
		const std::string &fileName);

template<typename T>
void printVector(std::vector<T> vector, std::string message);

template<typename T>
void printMatrix(std::vector<std::vector<T>> vector, std::string message);

template<typename T, typename K>
void printHistoryMatrix(std::vector<std::vector<std::pair<T, K>>>vector,
std::string message) {
	int col = 0;
	int row = 0;
	std::cout <<message
	<< std::endl;
	for (col = 0; col < vector[col].size();col++) {
		std::cout << '[';
		for (row = 0; row < vector.size()-1;row++)
		std::cout << "< " << vector[row][col].first <<" , "<<vector[row][col].second<< ">" << " , ";
		std::cout<<vector[row][col].first <<" , "<<vector[row][col].second<< ">"<<']'<<std::endl;
	}
}

std::vector<double> generateVector(int length, double from, double to);

std::vector<std::vector<double> > generateMatrix(int rows, int cols,
		double from, double to);

#endif /* FILEUTILITIES_HPP_ */
