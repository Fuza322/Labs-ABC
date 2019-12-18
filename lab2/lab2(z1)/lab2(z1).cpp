#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

using namespace std;
using namespace chrono;

const int numTasks = 1024 * 1024;
atomic<unsigned int> counterAtomic = 0;
unsigned int counterMutex = 0;
mutex m;

void reset(int8_t* arr) {
	counterAtomic = 0;
	for (size_t i = 0; i < numTasks; i++) arr[i] = 0;
	counterMutex = 0;
}

void SolveAtomic(int8_t* array, bool sleep) {
	unsigned int counter = 0;
	while (true) {
		counter = counterAtomic++;
		if (counter >= numTasks) break;
		array[counter]++;
		if (sleep)this_thread::sleep_for(chrono::nanoseconds(10));
	}
}

void SolveMutex(int8_t* array, bool sleep) {
	unsigned int counter = 0;
	while (true) {
		m.lock();
		counter = counterMutex++;
		m.unlock();
		if (counter >= numTasks) break;
		array[counter]++;
		if (sleep)this_thread::sleep_for(nanoseconds(10));
	}
}

int main()
{
	unsigned int NumThreads[] = { 4, 8, 16, 32 };
	thread* thrs = new thread[32];
	int8_t* arr = new int8_t[numTasks];
	reset(arr);


	for (int q = 0; q < 4; q++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		for (thread* thr = thrs; thr < thrs + NumThreads[q]; thr++)
		{
			*thr = thread(SolveMutex, arr, true);
			thr->join();
		}
		cout << "Mutex with slepp on " << NumThreads[q] << "thread(s) takes " << duration_cast<milliseconds>(high_resolution_clock::now() - t1).count() << "ms to complete\n";
		reset(arr);
	}
	cout << "*********************************************************" << endl;
	for (int q = 0; q < 4; q++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		for (thread* thr = thrs; thr < thrs + NumThreads[q]; thr++)
		{
			*thr = thread(SolveAtomic, arr, false);
			thr->join();
		}
		cout << "Atomic with slepp on " << NumThreads[q] << "thread(s) takes " << duration_cast<milliseconds>(high_resolution_clock::now() - t1).count() << "ms to complete\n";
		reset(arr);
	}
	cout << "*********************************************************" << endl;
	for (int q = 0; q < 4; q++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		for (thread* thr = thrs; thr < thrs + NumThreads[q]; thr++)
		{
			*thr = thread(SolveMutex, arr, false);
			thr->join();
		}
		cout << "Mutex without slepp on " << NumThreads[q] << "thread(s) takes " << duration_cast<milliseconds>(high_resolution_clock::now() - t1).count() << "ms to complete\n";
		reset(arr);
	}
	cout << "*********************************************************" << endl;
	for (int q = 0; q < 4; q++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		for (thread* thr = thrs; thr < thrs + NumThreads[q]; thr++)
		{
			*thr = thread(SolveAtomic, arr, false);
			thr->join();
		}
		cout << "Atomic without slepp on " << NumThreads[q] << "thread(s) takes " << duration_cast<milliseconds>(high_resolution_clock::now() - t1).count() << "ms to complete\n";
		reset(arr);
	}
}