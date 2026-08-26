#include <iostream>
using namespace std;

double calculateSum(double first, double second)
{
    double result = first + second;
    return result;
}

double findMaximum(double first, double second)
{
    if (first > second)
    {
        return first;
    }
    else
    {
        return second;
    }
}

int countPositive(int limit)
{
    int count = 0;

    for (int i = 0; i < limit; i++)
    {
        count += 1;
    }

    return count;
}

int main()
{
    double first = 12.5;
    double second = 7.5;
    double sum = calculateSum(first, second);
    double maximum = findMaximum(first, second);
    int positiveCount = countPositive(3);

    if (sum > maximum)
    {
        positiveCount += 1;
    }

    return 0;
}
