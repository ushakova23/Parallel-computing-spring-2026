#include <iostream>
#include <windows.h>
#include <intrin.h>  
#include <vector>
#include <cstdlib>   
#include <ctime>     

using namespace std;

//сортировка пузырьком
void bubbleSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    //настройки для эксперимента
    const int ARRAY_SIZE = 5000;  // сколько чисел сортируем
    const int K = 5;               // сколько раз повторяем замеры

    cout << "введите размер массива: " << ARRAY_SIZE << " элементов\n";
    cout << "введите количество замеров: " << K << "\n";

    //хранение результатов
    DWORD tick_results[K];           //GetTickCount()
    unsigned __int64 tsc_results[K]; //RDTSC
    double qpc_results[K];           //QueryPerformanceCounter

    srand(time(NULL));

    //измерения
    for (int test = 0; test < K; test++) {
        cout << "замер номер:" << test + 1 << ":\n";

        vector<int> original(ARRAY_SIZE);
        for (int i = 0; i < ARRAY_SIZE; i++) {
            original[i] = rand() % 10000;
        }

        //измерение GetTickCount()
        vector<int> arr1 = original;
        DWORD start1 = GetTickCount();
        bubbleSort(arr1);
        DWORD end1 = GetTickCount();
        tick_results[test] = end1 - start1;
        cout << "GetTickCount: " << tick_results[test] << "ms\n";

        //измерение RDTSC
        vector<int> arr2 = original;
        unsigned __int64 tsc1 = __rdtsc();
        bubbleSort(arr2);
        unsigned __int64 tsc2 = __rdtsc();
        tsc_results[test] = tsc2 - tsc1;
        cout << "RDTSC: " << tsc_results[test] << "ticks\n";

        //измерение QueryPerformanceCounter
        vector<int> arr3 = original;
        LARGE_INTEGER freq, qpc1, qpc2;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&qpc1);
        bubbleSort(arr3);
        QueryPerformanceCounter(&qpc2);

        double seconds = (qpc2.QuadPart - qpc1.QuadPart) / (double)freq.QuadPart;
        qpc_results[test] = seconds * 1000;
        cout << "QPC: " << qpc_results[test] << "ms\n";
    }

    //таблица результатов
    cout << "| Замер | GetTickCount(ms) | RDTSC(ticks)     | QPC(ms) |\n";

    for (int i = 0; i < K; i++) {
        printf("| %5d | %16lu | %17I64u | %7.3f |\n",
            i + 1,
            tick_results[i],
            tsc_results[i],
            qpc_results[i]);
    }

    //обработка для QPC 
    cout << "\n обработка для QPC\n";

    //минимальное время
    double min_time = qpc_results[0];
    for (int i = 1; i < K; i++) {
        if (qpc_results[i] < min_time) {
            min_time = qpc_results[i];
        }
    }
    cout << "минимальное время: " << min_time << "ms\n";

    //среднее время 
    double sum = 0;
    for (int i = 0; i < K; i++) {
        sum += qpc_results[i];
    }
    double avg = sum / K;
    cout << "среднее время: " << avg << "ms\n";

    //среднеквадратическое отклонение
    double sum_sq = 0;
    for (int i = 0; i < K; i++) {
        sum_sq += (qpc_results[i] - avg) * (qpc_results[i] - avg);
    }
    double sigma = sqrt(sum_sq / K);
    cout << "СКО (сигма): " << sigma << "ms\n";
    cout << "3сигма: " << 3 * sigma << "ms\n";

    //критерий трёх сигма
    cout << "\n проверка на выбросы:\n";
    vector<double> clean_results;
    for (int i = 0; i < K; i++) {
        double diff = abs(qpc_results[i] - avg);
        if (diff <= 3 * sigma) {
            clean_results.push_back(qpc_results[i]);
            cout << "замер" << i + 1 << " = " << qpc_results[i] << " ms - оставляем\n";
        }
        else {
            cout << "замер " << i + 1 << " = " << qpc_results[i] << " ms - выблос (удаляем)\n";
        }
    }

    //новое среднее без
    if (clean_results.size() > 0) {
        double clean_sum = 0;
        for (int i = 0; i < clean_results.size(); i++) {
            clean_sum += clean_results[i];
        }
        double clean_avg = clean_sum / clean_results.size();
        cout << "\n среднее время без выбросов: " << clean_avg << "ms\n";

        //доверительный интервал
        double clean_sq = 0;
        for (int i = 0; i < clean_results.size(); i++) {
            clean_sq += (clean_results[i] - clean_avg) * (clean_results[i] - clean_avg);
        }
        double clean_sigma = sqrt(clean_sq / clean_results.size());
        double delta = 1.96 * clean_sigma / sqrt(clean_results.size());

        cout << "доверительный интервал (p=0.95): " << clean_avg << "+-" << delta << "ms\n";
        cout << "интервал: [" << clean_avg - delta << "; " << clean_avg + delta << "] ms\n";
    }
    else {
        cout << "\n все значения оказались выбросами, перепроверьте условия";
    }

    cin.get();
    return 0;
}
