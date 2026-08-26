#include <iostream>
#include <vector>
#include <limits>
using namespace std;

double calculateSum(const vector<double>& numbers)
{
double sum = 0;
for (double value : numbers)
{
sum += value;
}
return sum;
}
double findMinimum(const vector<double>& numbers)
{
double minimum = numbers[0];
for (size_t i = 1; i < numbers.size(); i++)
{
if (numbers[i] < minimum)
{
minimum = numbers[i];
}
}
return minimum;
}
double findMaximum(const vector<double>& numbers)
{
double maximum = numbers[0];
for (size_t i = 1; i < numbers.size(); i++)
{
if (numbers[i] > maximum)
{
maximum = numbers[i];
}
}
return maximum;
}
int countPositive(const vector<double>& numbers)
{
int counter = 0;
for (double value : numbers)
{
if (value > 0)
{
counter++;
}
}
return counter;
}
int main()
{
setlocale(LC_ALL, "ru");
int count;
cout << "=== АНАЛИЗ ЧИСЛОВОЙ ПОСЛЕДОВАТЕЛЬНОСТИ ===" << endl;
do
{
cout << "Введите количество чисел: ";
if (!(cin >> count))
{
cout << "Ошибка: необходимо ввести целое число!" << endl;
cin.clear();
cin.ignore(numeric_limits<streamsize>::max(), '\n');
count = 0;
}
else if (count <= 0)
{
cout << "Количество элементов должно быть больше нуля." << endl;
}
} while (count <= 0);
vector<double> numbers(count);
cout << "\nВведите числа:" << endl;
for (int i = 0; i < count; i++)
{
cout << "Элемент " << i + 1 << ": ";
cin >> numbers[i];
}
double sum = calculateSum(numbers);
double average = sum / count;
double minimum = findMinimum(numbers);
double maximum = findMaximum(numbers);
int positiveCount = countPositive(numbers);
cout << "\n=== РЕЗУЛЬТАТ ===" << endl;
cout << "Сумма: " << sum << endl;
cout << "Среднее значение: " << average << endl;
cout << "Минимальное значение: " << minimum << endl;
cout << "Максимальное значение: " << maximum << endl;
cout << "Количество положительных чисел: " << positiveCount << endl;
if (average > 0)
{
cout << "Среднее значение последовательности положительное." << endl;
}
else if (average < 0)
{
cout << "Среднее значение последовательности отрицательное." << endl;
}
else
{
cout << "Среднее значение равно нулю." << endl;
}
return 0;
}
