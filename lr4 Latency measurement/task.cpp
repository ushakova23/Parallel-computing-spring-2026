#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cstring>

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#else
#include <x86intrin.h>
#endif

using namespace std;

const int REPEAT_COUNT = 10;      // количество повторений для усреднения
const int WARMUP_COUNT = 3;       // количество проходов

// получения времени в тактах 
inline uint64_t rdtsc() {
    return __rdtsc();
}

// пследовательный обход
double sequential_sum(int* arr, int n, int repeats) {
    long long total_time = 0;
    for (int r = 0; r < repeats; r++) {
        long long sum = 0;
        uint64_t start = rdtsc();
        for (int i = 0; i < n; i++) {
            sum += arr[i];
        }
        uint64_t end = rdtsc();
        total_time += (end - start);
        if (sum == -1) cout << "";
    }
    return (double)total_time / repeats / n;
}

// случайный обход
double random_sum(int* arr, int* indices, int n, int repeats) {
    long long total_time = 0;
    for (int r = 0; r < repeats; r++) {
        long long sum = 0;
        uint64_t start = rdtsc();
        for (int i = 0; i < n; i++) {
            sum += arr[indices[i]];
        }
        uint64_t end = rdtsc();
        total_time += (end - start);
        if (sum == -1) cout << "";
    }
    return (double)total_time / repeats / n;
}

// случайный обход с дополнительным массивом индексов
double random_sum_with_index_arr(int* arr, int* index_arr, int n, int repeats) {
    long long total_time = 0;
    for (int r = 0; r < repeats; r++) {
        long long sum = 0;
        uint64_t start = rdtsc();
        for (int i = 0; i < n; i++) {
            int idx = index_arr[i];
            sum += arr[idx];
        }
        uint64_t end = rdtsc();
        total_time += (end - start);
        if (sum == -1) cout << "";
    }
    return (double)total_time / repeats / n;
}

int main() {
    system("chcp 1251 > nul");
    cout << "измерение латентности кэш-памяти" << endl;

    vector<int> sizes;

    for (int kb = 4; kb <= 2048; kb += 64) {
        sizes.push_back(kb * 1024 / 4);  
    }

    for (int mb = 2; mb <= 32; mb += 2) {
        sizes.push_back(mb * 1024 * 1024 / 4);
    }

    for (int mb = 35; mb <= 150; mb += 5) {
        sizes.push_back(mb * 1024 * 1024 / 4);
    }

    cout << "всего будет измерено " << sizes.size() << " размеров массива" << endl;

    cout << fixed << setprecision(2);
    cout << "Размер(КБ)\tРазмер(эл-тов)\tПоследоват(тактов)\tСлучайный(тактов)\tСлучайный с индексами(тактов)" << endl;

    random_device rd;
    mt19937 gen(rd());

    for (int n : sizes) {
        int size_kb = n * 4 / 1024;

        int* arr = new int[n];
        int* indices = new int[n];
        int* index_arr = new int[n];

        for (int i = 0; i < n; i++) {
            arr[i] = i;
        }

        // случайные индексы
        for (int i = 0; i < n; i++) {
            indices[i] = i;
        }
        shuffle(indices, indices + n, gen);

        for (int i = 0; i < n; i++) {
            index_arr[i] = indices[i];
        }

        for (int w = 0; w < WARMUP_COUNT; w++) {
            sequential_sum(arr, n, 1);
            random_sum(arr, indices, n, 1);
            random_sum_with_index_arr(arr, index_arr, n, 1);
        }

        // измерения
        double seq_time = sequential_sum(arr, n, REPEAT_COUNT);
        double rand_time = random_sum(arr, indices, n, REPEAT_COUNT);
        double rand_idx_time = random_sum_with_index_arr(arr, index_arr, n, REPEAT_COUNT);

        // результаты
        cout << size_kb << "\t\t" << n << "\t\t" << seq_time << "\t\t\t" << rand_time << "\t\t\t" << rand_idx_time << endl;

        delete[] arr;
        delete[] indices;
        delete[] index_arr;
    }


    return 0;
}
