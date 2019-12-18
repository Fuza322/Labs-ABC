#include <stdio.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <future>
#include <atomic>
#include <chrono>
#include <queue>

typedef uint8_t byte;
using namespace std;
using namespace chrono;


class AbstractQ
{
public:
	virtual void push(byte val) = 0;
	virtual bool pop(byte& val) = 0;
};

class UnlimitedMutQ :public AbstractQ
{
public:
	queue<uint8_t> que;
	mutex mut;

	void push(uint8_t num)
	{
		mut.lock();
		que.push(num);
		mut.unlock();
	}

	bool pop(uint8_t& num)
	{
		mut.lock();
		if (que.empty())
		{
			mut.unlock();
			this_thread::sleep_for(milliseconds(15));
			mut.lock();
			if (que.empty())
			{
				mut.unlock();
				return false;
			}
		}
		num = que.front();
		que.pop();
		mut.unlock();
		return true;
	}
};

class LimitedAtomQ : public AbstractQ
{
	int size;
	atomic<byte>* queue;
	alignas(64) atomic<int> toPush = 0, toPop = 0;
public:
	LimitedAtomQ(int size)
	{
		this->size = size;
		queue = new atomic<byte>[size];
		for (int i = 0; i < size; i++)
			queue[i] = 0;
	}

	~LimitedAtomQ()
	{
		delete[] queue;
	}

	void push(byte val)
	{
		while (42)
		{
			int back = toPush.load();
			if (back == toPop + size)continue;

			byte x = queue[back % size];
			if (x) continue;

			if (toPush.compare_exchange_strong(back, toPush + 1))
			{
				if (queue[back % size].compare_exchange_strong(x, val));
				return;
			}
		}
	}

	bool pop(byte& value) override
	{
		while (1)
		{
			int start = toPop.load();
			if (start == toPush)
			{
				this_thread::sleep_for(std::chrono::milliseconds(1));
				start = toPop.load();
				if (start == toPush)
					return false;
			}
			byte x = queue[start % size];
			if (x == 0) return false;
			if (toPop.compare_exchange_strong(start, toPop + 1))
			{
				if (queue[start % size].compare_exchange_strong(x, 0))
				{
					value = x;
					return true;
				}
				return true;
			}
		}
	}
};

class LimitedMutQ :public AbstractQ
{
	int size;
	byte* q;
	alignas(64) long toPop = 0, b = 0;
	mutex mut;
	condition_variable write, read;

public:
	LimitedMutQ(int size) : size(size), q(new byte[size]) {}

	void push(uint8_t p) override
	{
		unique_lock<mutex> lock(mut);
		read.wait(lock, [this] {return b + size > toPop; });
		q[toPop++ & size - 1] = p;
		write.notify_one();
	}

	bool pop(uint8_t& back) override
	{
		unique_lock<mutex> lock(mut);
		if (write.wait_for(lock, std::chrono::milliseconds(10), [this] { return b < toPop; }))
		{
			back = q[b++ & (size - 1)];
			read.notify_one();
			return true;
		}
		else return false;

	}
};

void test(AbstractQ& q, int numofp, int numofc, int taskNum)
{
	atomic<int> sum = 0;
	int work = taskNum * numofp / numofc;

	auto Prod = [&](int i) {for (int i = 0; i < taskNum; i++)q.push(1); };
	auto Cons = [&](int i) {for (int i = 0; i < work; i++) { byte poppedValue = 0; while (!q.pop(poppedValue)); sum += poppedValue; }};
	cout << "Producers: " << numofp << " Consumers: " << numofc;

	vector<thread> threads(numofc + numofp);
	auto t1 = high_resolution_clock::now();
	for (int i = 0; i < numofp; i++) threads[i] = thread(Prod, i);
	for (int i = 0; i < numofc; i++)threads[i + numofp] = thread(Cons, i);
	for (int i = 0; i < threads.size(); i++)threads[i].join();

	cout << ". Time: " << duration_cast<milliseconds>(high_resolution_clock::now() - t1).count() << "ms. Error: " << taskNum * numofp - sum << endl;
}

int main()
{
	int cnums[3] = { 1,2,4 };
	int pnums[3] = { 1,2,4 };
	int taskNum = 4 * 1024;// *1024;

	UnlimitedMutQ q1;
	cout << "Unlimited mutex queue: " << endl;
	for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++)test(q1, pnums[i], cnums[j], taskNum);
	LimitedMutQ q2(4);
	cout << "Limited mutex queue: " << endl;
	for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++)test(q2, pnums[i], cnums[j], taskNum);
	LimitedAtomQ q3(4);
	cout << "Limited atomic queue: " << endl;
	for (int i = 0; i < 3; i++)for (int j = 0; j < 3; j++)test(q3, pnums[i], cnums[j], taskNum);

}