/*
 * FileUtilities.cpp
 *
 *  Created on: Jun 7, 2015
 *      Author: hekpotexhapb
 */

#include "FileUtilities.hpp"

void finalOutput(const std::string &fileName,
		const std::vector<std::pair<Probability, State> > &historyVector) {
	std::ofstream outputFile(fileName.c_str());
	auto first = true;
	for (std::pair<Probability, State> i : historyVector) {
		outputFile << "<" << i.first << " , " << i.second << "> ";
	}
}

std::vector<double> extractVectorFromFile(const std::string &fileName) {
	std::regex delim("\\s+");

	std::ifstream file(fileName);
	std::string line;
	std::vector<std::string> delimited;
	std::vector<double> row;

	std::vector<double> result;
	while (getline(file, line)) {
		std::sregex_token_iterator
				first { line.begin(), line.end(), delim, -1 }, last;
		delimited = {first,last};
		row = std::vector<double>(delimited.size());

		for (int i = 0; i < delimited.size(); i++)
			result.push_back(std::atof(&(delimited[i])[0]));

	}
	return result;
}

std::vector<std::vector<double>> extractMatrixFromFile(const int rows,
		const int cols, const std::string &fileName) {
	std::ifstream file(fileName);
	std::vector<std::vector<double>> result(rows);
	std::vector<double> row(cols);
	std::string num;

	for (int i = 0; i < rows; i++) {

		for (int j = 0; j < cols; j++) {
			file >> num;
			row[i] = std::atof(&num[0]);
		}
		result[i] = row;
	}
	return result;
}

std::vector<std::vector<double>> extractMatrixFromFile(
		const std::string &fileName) {

	std::regex delim("\\s+");
//std::cout << fileName << std::endl;
	std::ifstream file(fileName);
	std::string line;
	std::vector<std::string> delimited;
	std::vector<double> row;

	std::vector<std::vector<double>> result;
	while (getline(file, line)) {
		std::sregex_token_iterator
				first { line.begin(), line.end(), delim, -1 }, last;
		delimited = {first,last};
		row = std::vector<double>(delimited.size());
#pragma omp parallel for
		for (int i = 0; i < delimited.size(); i++)
			row[i] = std::atof(&(delimited[i])[0]);
		result.push_back(row);
	}
	return result;
}

template<typename T>
void printVector(std::vector<T> vector, std::string message) {
	int i = 0;
	std::cout << message << std::endl << '[';
	for (i = 0; i < vector.size() - 1; i++) {
		std::cout << vector[i] << "\t , ";
	}
	std::cout << vector[i] << ']' << std::endl;
}

template<typename T>
void printMatrix(std::vector<std::vector<T>> vector, std::string message) {
	int i = 0;
	std::cout << message << std::endl;
	for (int j = 0; j < vector.size(); j++) {
		std::cout << '[';
		for (i = 0; i < vector[j].size() - 1; i++)
			std::cout << vector[j][i] << "\t , ";
		std::cout << vector[j][i] << ']' << std::endl;
	}
}



std::vector<double> generateVector(int length, double from, double to) {
//	std::mt19937 rng(std::time(nullptr));
//	std::uniform_real_distribution<double> uniform(from, to);

	std::vector<double> result(length);

	for (int j = 0; j < length; j++)
		//result[j] = uniform(rng);
		result[j] = from
				+ static_cast<float>(rand())
						/ (static_cast<float>(RAND_MAX / (to - from)));
	return result;
}

std::vector<std::vector<double> > generateMatrix(int rows, int cols,
		double from, double to) {

	std::vector<std::vector<double> > result(rows);
#pragma omp parallel for
	for (int i = 0; i < result.size(); i++) {
		result[i] = generateVector(cols, from, to);
	}
	return result;
}


