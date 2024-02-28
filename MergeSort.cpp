#include "MergeSort.h"
#include <chrono>
#include <thread>
#include <cassert>
#include <random>
#include <omp.h>

void MergeSort::mMerge(std::vector<int>& a, uint64_t beg, uint64_t m, uint64_t end)
{
	std::vector<int> b(end - beg, 0);
	uint64_t i = 0, j = beg, k = m;
	while (j < m && k < end)
		if (a[j] <= a[k])
			b[i++] = a[j++];
		else
			b[i++] = a[k++];

	while (j < m)
		b[i++] = a[j++];

	while(i>0) {
		--i;
		a[beg + i] = b[i];
	}
}

void MergeSort::mSort(std::vector<int>& a, uint64_t beg, uint64_t end)
{
	if (end - beg > 1) {
		uint64_t m = (beg + end) >> 1;
		mSort(a, beg, m);
		mSort(a, m, end);
		mMerge(a, beg, m, end);
	}
}

void MergeSort::seqMSort(std::vector<int>& a)
{
	mSort(a, 0, a.size());
}

void MergeSort::parMSort(std::vector<int>& a)
{
	assert(a.size() % NO_OF_THREADS == 0);

	uint64_t parallelChunks = a.size() / NO_OF_THREADS;
	std::thread threadpool[NO_OF_THREADS];

	// Split sort array into NO_OF_THREADS parts and sort them parallel
	for (size_t i = 0; i < NO_OF_THREADS; i++) {
		threadpool[i] = std::thread(mSort, std::ref(a), i * parallelChunks, (i + 1) * parallelChunks);
	}

	for (size_t i = 0; i < NO_OF_THREADS; i++) {
		threadpool[i].join();
	}

	// Merge parallel sorted chunks
	size_t nFinalizingMerges = NO_OF_THREADS / 2;
	while (nFinalizingMerges > 0) {
		uint64_t mergeFrame = a.size() / nFinalizingMerges;
		for (size_t i = 0; i < nFinalizingMerges; i++) {
			uint64_t beg = i * mergeFrame;
			threadpool[i] = std::thread(mMerge, std::ref(a), beg, beg + mergeFrame / 2, beg + mergeFrame);
		}
		for (size_t i = 0; i < nFinalizingMerges; i++) {
			threadpool[i].join();
		}
		nFinalizingMerges /= 2;
	}
}

void MergeSort::ompMSort(std::vector<int>& a) {
	size_t parallelChunks = a.size() / NO_OF_THREADS;
	
	// Split sort array into NO_OF_THREADS parts and sort them with OMP
	#pragma omp parallel for num_threads(NO_OF_THREADS)
	for (size_t i = 0; i < NO_OF_THREADS; i++) {
		mSort(a, i * parallelChunks, (i + 1) * parallelChunks);
	}
	
	// Merge parallel sorted chunks
	size_t nFinalizingMerges = NO_OF_THREADS / 2;
	while (nFinalizingMerges > 0) {
		uint64_t mergeFrame = a.size() / nFinalizingMerges;
		#pragma omp parallel for num_threads(NO_OF_THREADS)
		for (size_t i = 0; i < nFinalizingMerges; i++) {
			uint64_t beg = i * mergeFrame;
			mMerge(a, beg, beg + mergeFrame / 2, beg + mergeFrame);
		}

		nFinalizingMerges /= 2;
	}
}

double MergeSort::measuredSort(std::vector<int>& a, void(MergeSort::*sortFunc)(std::vector<int>& a))
{
	auto start = std::chrono::high_resolution_clock::now();

	(this->*sortFunc)(a);

	auto stop = std::chrono::high_resolution_clock::now();

	return (double)(stop - start).count();
}


double MergeSort::seqMergeSort(std::vector<int>& a)
{
	return measuredSort(a, &MergeSort::seqMSort);
}

double MergeSort::parMergeSort(std::vector<int>& a)
{
	return measuredSort(a, &MergeSort::parMSort);
}

double MergeSort::ompMergeSort(std::vector<int>& a)
{
	return measuredSort(a, &MergeSort::ompMSort);
}

bool MergeSort::proove(std::vector<int>& a)
{
	for (uint64_t i = 0; i < a.size() - 1; ++i) {
		if (a[i] > a[i + 1])
			return false;
	}

	return true;
}

std::vector<int> MergeSort::createRandomData(uint64_t size) {

	std::vector<int> data(size, 0);

	// random gen
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(0, (int)size);
	generator.seed((unsigned)std::chrono::system_clock::now().time_since_epoch().count());

	for (int i = 0; i < size; ++i) {
		data[i] = distribution(generator);
	}

	return data;
}
