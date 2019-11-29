#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>
//------------------------------------------------------------------------------------------------------
double one_thread(const std::vector<int>&, const std::vector<int>&);
double parallel_threads(const std::vector<int>&, const std::vector<int>&);
//------------------------------------------------------------------------------------------------------
int main()
{
	size_t n = 3000000;
	setlocale(LC_ALL, "rus");
	std::vector<int> a(n, 1), b(n, 2);
	double thread1 = one_thread(a, b);
	double thread2 = parallel_threads(a, b);
	std::cout << "Параллельные потоки работают быстрее в " << thread1 / thread2 << " раз" << std::endl;
	std::cout << "-----------------------------------------------------------------------" << std::endl;
	return 0;
}
//------------------------------------------------------------------------------------------------------
double parallel_threads(const std::vector<int>& vect1, const std::vector<int>& vect2)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	int value = 0;
#pragma omp parallel for reduction(+:ans) schedule(static)
	for (int i = 0; i < vect1.size(); i++)
	{
		value += vect1[i] * vect2[i];
	}
	auto end_time = std::chrono::high_resolution_clock::now();
	double dur_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count() * 1e-9;
	std::cout << std::endl << "Time (parallel threads): " << dur_time << "sec" << std::endl;
	std::cout << std::endl;
	return dur_time;
}
//------------------------------------------------------------------------------------------------------
double one_thread(const std::vector<int>& vect1, const std::vector<int>& vect2)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	std::vector<int> result(vect1.size(), 0);
	int value = 0;
	for (int i = 0; i < vect1.size(); i++)
	{
		value += vect1[i] * vect2[i];
	}
	auto end_time = std::chrono::high_resolution_clock::now();
	double dur_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count() * 1e-9;
	std::cout << std::endl << "Time (one thread): " << dur_time << "sec" << std::endl;
	return dur_time;
}
//------------------------------------------------------------------------------------------------------