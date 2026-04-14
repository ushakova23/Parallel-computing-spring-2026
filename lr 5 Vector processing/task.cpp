#include <iostream>
#include <windows.h>
#include <intrin.h>
#include <cstdlib>
#include <ctime>

using namespace std;

#define N 10000000

const float ALPHA = 0.3f;
const float BETA = 0.7f;

void fill_arrays(int8_t* a, int8_t* b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
}

bool compare_results(int8_t* c1, int8_t* c2, int n) {
    for (int i = 0; i < n; i++) {
        if (c1[i] != c2[i]) return false;
    }
    return true;
}

double get_time_ms() {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return count.QuadPart * 1000.0 / freq.QuadPart;
}

void scalar_blend(int8_t* a, int8_t* b, int8_t* c, int n) {
    for (int i = 0; i < n; i++) {
        float val = a[i] * ALPHA + b[i] * BETA;
        c[i] = (int8_t)(val + 0.5f);
    }
}

void avx_scalar_blend(int8_t* a, int8_t* b, int8_t* c, int n) {
    int i = 0;
    for (; i <= n - 16; i += 16) {
        __m128i va = _mm_loadu_si128((__m128i*)(a + i));
        __m128i vb = _mm_loadu_si128((__m128i*)(b + i));
        int8_t* pa = (int8_t*)&va;
        int8_t* pb = (int8_t*)&vb;
        int8_t res[16];
        for (int j = 0; j < 16; j++) {
            float val = pa[j] * ALPHA + pb[j] * BETA;
            res[j] = (int8_t)(val + 0.5f);
        }
        __m128i vc = _mm_loadu_si128((__m128i*)res);
        _mm_storeu_si128((__m128i*)(c + i), vc);
    }
    for (; i < n; i++) {
        float val = a[i] * ALPHA + b[i] * BETA;
        c[i] = (int8_t)(val + 0.5f);
    }
}

int main() {
    system("chcp 1251 > nul");

    int8_t* a = (int8_t*)_aligned_malloc(N * sizeof(int8_t), 64);
    int8_t* b = (int8_t*)_aligned_malloc(N * sizeof(int8_t), 64);
    int8_t* c_scalar = (int8_t*)_aligned_malloc(N * sizeof(int8_t), 64);
    int8_t* c_avx_scalar = (int8_t*)_aligned_malloc(N * sizeof(int8_t), 64);

    srand(12345);
    fill_arrays(a, b, N);

    double t1 = get_time_ms();
    scalar_blend(a, b, c_scalar, N);
    double t_scalar = get_time_ms() - t1;

    double t2 = get_time_ms();
    avx_scalar_blend(a, b, c_avx_scalar, N);
    double t_avx_scalar = get_time_ms() - t2;

    cout << "1. Скалярный: " << t_scalar << " ms\n";
    cout << "2. AVX скалярные команды: " << t_avx_scalar << " ms\n";

    cout << "\nПроверка корректности:\n";
    cout << "Скалярный vs AVX скалярный: " << (compare_results(c_scalar, c_avx_scalar, N) ? "OK" : "ОШИБКА") << endl;

    _aligned_free(a);
    _aligned_free(b);
    _aligned_free(c_scalar);
    _aligned_free(c_avx_scalar);

    cin.get();
    return 0;
}
