#include <iostream>
#include <windows.h>
#include <intrin.h>
#include <cstdlib>
#include <ctime>

using namespace std;

const int WIDTH = 1024;
const int HEIGHT = 1024;
const int IMG_SIZE = WIDTH * HEIGHT;

unsigned char* src;
unsigned char* dst_scalar;
unsigned char* dst_simd;

double get_time_ms() {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return count.QuadPart * 1000.0 / freq.QuadPart;
}

void init_image() {
    for (int i = 0; i < IMG_SIZE; i++) {
        src[i] = rand() % 256;
    }
}

void laplace_scalar() {
    for (int y = 1; y < HEIGHT - 1; y++) {
        for (int x = 1; x < WIDTH - 1; x++) {
            int idx = y * WIDTH + x;
            int h = src[idx - WIDTH - 1] + src[idx - WIDTH] + src[idx - WIDTH + 1] +
                    src[idx - 1] - 8 * src[idx] + src[idx + 1] +
                    src[idx + WIDTH - 1] + src[idx + WIDTH] + src[idx + WIDTH + 1];
            if (h < 0) h = 0;
            if (h > 255) h = 255;
            dst_scalar[idx] = (unsigned char)h;
        }
    }
}

void laplace_sse() {
    const __m128i eight = _mm_set1_epi16(8);
    const __m128i zero = _mm_setzero_si128();
    const __m128i max255 = _mm_set1_epi16(255);

    for (int y = 1; y < HEIGHT - 1; y++) {
        const unsigned char* top = src + (y - 1) * WIDTH;
        const unsigned char* mid = src + y * WIDTH;
        const unsigned char* bot = src + (y + 1) * WIDTH;
        unsigned char* out = dst_simd + y * WIDTH;

        int x = 1;
        for (; x <= WIDTH - 9; x += 8) {
            __m128i t0 = _mm_loadl_epi64((__m128i*)(top + x - 1));
            __m128i t1 = _mm_loadl_epi64((__m128i*)(top + x));
            __m128i t2 = _mm_loadl_epi64((__m128i*)(top + x + 1));
            __m128i m0 = _mm_loadl_epi64((__m128i*)(mid + x - 1));
            __m128i m1 = _mm_loadl_epi64((__m128i*)(mid + x));
            __m128i m2 = _mm_loadl_epi64((__m128i*)(mid + x + 1));
            __m128i b0 = _mm_loadl_epi64((__m128i*)(bot + x - 1));
            __m128i b1 = _mm_loadl_epi64((__m128i*)(bot + x));
            __m128i b2 = _mm_loadl_epi64((__m128i*)(bot + x + 1));

            t0 = _mm_cvtepu8_epi16(t0);
            t1 = _mm_cvtepu8_epi16(t1);
            t2 = _mm_cvtepu8_epi16(t2);
            m0 = _mm_cvtepu8_epi16(m0);
            m1 = _mm_cvtepu8_epi16(m1);
            m2 = _mm_cvtepu8_epi16(m2);
            b0 = _mm_cvtepu8_epi16(b0);
            b1 = _mm_cvtepu8_epi16(b1);
            b2 = _mm_cvtepu8_epi16(b2);

            __m128i sum = t0;
            sum = _mm_add_epi16(sum, t1);
            sum = _mm_add_epi16(sum, t2);
            sum = _mm_add_epi16(sum, m0);
            sum = _mm_add_epi16(sum, m2);
            sum = _mm_add_epi16(sum, b0);
            sum = _mm_add_epi16(sum, b1);
            sum = _mm_add_epi16(sum, b2);

            __m128i center = _mm_mullo_epi16(m1, eight);
            sum = _mm_sub_epi16(sum, center);

            __m128i neg = _mm_sub_epi16(zero, sum);
            sum = _mm_max_epi16(sum, neg);
            sum = _mm_min_epi16(sum, max255);

            __m128i packed = _mm_packus_epi16(sum, sum);
            _mm_storel_epi64((__m128i*)(out + x), packed);
        }

        for (; x < WIDTH - 1; x++) {
            int h = top[x - 1] + top[x] + top[x + 1] +
                    mid[x - 1] - 8 * mid[x] + mid[x + 1] +
                    bot[x - 1] + bot[x] + bot[x + 1];
            if (h < 0) h = 0;
            if (h > 255) h = 255;
            out[x] = (unsigned char)h;
        }
    }
}

int main() {
    src = new unsigned char[IMG_SIZE];
    dst_scalar = new unsigned char[IMG_SIZE];
    dst_simd = new unsigned char[IMG_SIZE];

    init_image();

    laplace_scalar();
    laplace_sse();

    double t1 = get_time_ms();
    laplace_scalar();
    double t_scalar = get_time_ms() - t1;

    double t2 = get_time_ms();
    laplace_sse();
    double t_simd = get_time_ms() - t2;

    cout << "Scalar: " << t_scalar << " ms\n";
    cout << "SSE: " << t_simd << " ms\n";
    cout << "Speedup: x" << (t_scalar / t_simd) << "\n";

    delete[] src;
    delete[] dst_scalar;
    delete[] dst_simd;

    cin.get();
    return 0;
}
