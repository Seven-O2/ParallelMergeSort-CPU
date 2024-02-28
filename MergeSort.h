#pragma once
#include <vector>
#include <cmath>

// set to powers of 2
constexpr auto NO_OF_THREADS = 4;

class MergeSort
{
	static void mMerge(std::vector<int>& a, uint64_t beg, uint64_t m, uint64_t end);
	static void mSort(std::vector<int>& a, uint64_t beg, uint64_t end);

	void seqMSort(std::vector<int>& a);
	void parMSort(std::vector<int>& a);
	void ompMSort(std::vector<int>& a);

	double measuredSort(std::vector<int>& a, void(MergeSort::*sortFunc)(std::vector<int>& a));

public:

	double seqMergeSort(std::vector<int>& a);
	double parMergeSort(std::vector<int>& a);
	double ompMergeSort(std::vector<int>& a);
	bool proove(std::vector<int>& a);
	std::vector<int> createRandomData(uint64_t size);
};
