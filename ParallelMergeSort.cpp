#include <iostream>
#include "MergeSort.h"
#include <random>
#include <chrono>

int main() {

	MergeSort m;
	const uint64_t size = ((32 * 1024 * 1024) / NO_OF_THREADS) * NO_OF_THREADS;

	auto seqData = m.createRandomData(size);
	std::vector<int> parData(seqData);

	double seqTime = m.seqMergeSort(seqData);
	if (!m.proove(seqData)) {
		std::cout << "seq sort did not sort..." << std::endl;
		exit(-42);
	}

	double parTime = m.parMergeSort(parData);
	if (!m.proove(parData)) {
		std::cout << "par sort did not sort..." << std::endl;
		exit(-42);
	}	

	double ompTime = m.ompMergeSort(seqData);
	if (!m.proove(seqData)) {
		std::cout << "seq sort did not sort..." << std::endl;
		exit(-42);
	}

	std::cout << "sorted seq in " << seqTime / 1000000 << "ms" << std::endl;
	std::cout << "sorted par in " << parTime / 1000000 << "ms" << std::endl;
	std::cout << "sorted omp in " << ompTime / 1000000 << "ms" << std::endl;
	std::cout << "speedup (seq/par) is " << seqTime / parTime << std::endl;
	std::cout << "speedup (par/omp) is " << parTime / ompTime << std::endl;
	std::cout << "speedup (seq/omp) is " << seqTime / ompTime << std::endl;
}
