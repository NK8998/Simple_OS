#include <iostream>
#include <thread>

void inc1(int *num)
{
    *num = *num + 1;
}

void inc2(int *num)
{
    *num = *num + 1;
}

int main()
{
    int num = 0;

    std::thread t1([&num]
                   { inc1(&num); });

    std::thread t2([&num]
                   { inc2(&num); });

    if (t1.joinable())
    {
        t1.join();
    }

    if (t2.joinable())
    {
        t2.join();
    }

    std::cout << "value: " << num << std::endl;
}