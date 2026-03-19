#include <iostream>
#include <windows.h>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;


//матрица NxN
float** alloc_matrix(int N) {
    float** m = new float* [N];
    for (int i = 0; i < N; i++) {
        m[i] = new float[N];
    }
    return m;
}

void free_matrix(float** m, int N) {
    for (int i = 0; i < N; i++) {
        delete[] m[i];
    }
    delete[] m;
}

void fill_random(float** m, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            m[i][j] = rand() % 10 + 1;  
        }
    }
}

//сравнение двух матриц 
bool compare_matrices(float** A, float** B, int N, float eps = 0.001f) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (fabs(A[i][j] - B[i][j]) > eps) {
                cout << "Несовпадение в [" << i << "][" << j << "]: ";
                cout << A[i][j] << " vs " << B[i][j] << endl;
                return false;
            }
        }
    }
    return true;
}

void copy_matrix(float** src, float** dst, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            dst[i][j] = src[i][j];
        }
    }
}

//обнуление
void zero_matrix(float** m, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            m[i][j] = 0.0f;
        }
    }
}

void print_matrix(float** m, int N, const string& name) {
    cout << "Матрица " << name << ":\n";
    for (int i = 0; i < min(N, 6); i++) {
        for (int j = 0; j < min(N, 6); j++) {
            cout << setw(8) << m[i][j] << " ";
        }
        if (N > 6) cout << "...";
        cout << endl;
    }
    cout << endl;
}

//измерение времени

double get_time() {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return count.QuadPart / (double)freq.QuadPart;
}

//классическое умножение

void mul_classic(float** A, float** B, float** C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

//транспонирование матрицы В

void mul_transpose(float** A, float** B, float** C, int N, bool include_transpose_time = true) {
    float** Bt = alloc_matrix(N);

    double t1 = get_time();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            Bt[i][j] = B[j][i];
        }
    }

    double t2 = get_time();
    double transpose_time = t2 - t1;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * Bt[j][k];
            }
            C[i][j] = sum;
        }
    }

    double t3 = get_time();
    double mul_time = t3 - t2;

    free_matrix(Bt, N);
}

//буферизация столбца

void mul_buffer(float** A, float** B, float** C, int N, int M) {
    zero_matrix(C, N);
    float* col = new float[N];

    for (int j = 0; j < N; j++) {
        for (int k = 0; k < N; k++) {
            col[k] = B[k][j];
        }

        for (int i = 0; i < N; i++) {
            float sum = 0.0f;

            int k = 0;

            if (M >= 16 && N >= 16) {
                for (; k <= N - 16; k += 16) {
                    sum += A[i][k] * col[k] + A[i][k + 1] * col[k + 1] +
                        A[i][k + 2] * col[k + 2] + A[i][k + 3] * col[k + 3] +
                        A[i][k + 4] * col[k + 4] + A[i][k + 5] * col[k + 5] +
                        A[i][k + 6] * col[k + 6] + A[i][k + 7] * col[k + 7] +
                        A[i][k + 8] * col[k + 8] + A[i][k + 9] * col[k + 9] +
                        A[i][k + 10] * col[k + 10] + A[i][k + 11] * col[k + 11] +
                        A[i][k + 12] * col[k + 12] + A[i][k + 13] * col[k + 13] +
                        A[i][k + 14] * col[k + 14] + A[i][k + 15] * col[k + 15];
                }
            }
            else if (M >= 8) {
                for (; k <= N - 8; k += 8) {
                    sum += A[i][k] * col[k] + A[i][k + 1] * col[k + 1] +
                        A[i][k + 2] * col[k + 2] + A[i][k + 3] * col[k + 3] +
                        A[i][k + 4] * col[k + 4] + A[i][k + 5] * col[k + 5] +
                        A[i][k + 6] * col[k + 6] + A[i][k + 7] * col[k + 7];
                }
            }
            else if (M >= 4) {
                for (; k <= N - 4; k += 4) {
                    sum += A[i][k] * col[k] + A[i][k + 1] * col[k + 1] +
                        A[i][k + 2] * col[k + 2] + A[i][k + 3] * col[k + 3];
                }
            }
            else if (M >= 2) {
                for (; k <= N - 2; k += 2) {
                    sum += A[i][k] * col[k] + A[i][k + 1] * col[k + 1];
                }
            }

            for (; k < N; k++) {
                sum += A[i][k] * col[k];
            }

            C[i][j] = sum;
        }
    }

    delete[] col;
}

//блочное умножение

void mul_block(float** A, float** B, float** C, int N, int S, int M) {

    zero_matrix(C, N);

    for (int ii = 0; ii < N; ii += S) {
        for (int jj = 0; jj < N; jj += S) {
            for (int kk = 0; kk < N; kk += S) {

                int i_max = min(ii + S, N);
                int j_max = min(jj + S, N);
                int k_max = min(kk + S, N);

                for (int i = ii; i < i_max; i++) {
                    for (int j = jj; j < j_max; j++) {
                        float sum = 0.0f;

                        int k = kk;

                        if (M >= 16 && k_max - kk >= 16) {
                            for (; k <= k_max - 16; k += 16) {
                                sum += A[i][k] * B[k][j] + A[i][k + 1] * B[k + 1][j] +
                                    A[i][k + 2] * B[k + 2][j] + A[i][k + 3] * B[k + 3][j] +
                                    A[i][k + 4] * B[k + 4][j] + A[i][k + 5] * B[k + 5][j] +
                                    A[i][k + 6] * B[k + 6][j] + A[i][k + 7] * B[k + 7][j] +
                                    A[i][k + 8] * B[k + 8][j] + A[i][k + 9] * B[k + 9][j] +
                                    A[i][k + 10] * B[k + 10][j] + A[i][k + 11] * B[k + 11][j] +
                                    A[i][k + 12] * B[k + 12][j] + A[i][k + 13] * B[k + 13][j] +
                                    A[i][k + 14] * B[k + 14][j] + A[i][k + 15] * B[k + 15][j];
                            }
                        }
                        else if (M >= 8 && k_max - kk >= 8) {
                            for (; k <= k_max - 8; k += 8) {
                                sum += A[i][k] * B[k][j] + A[i][k + 1] * B[k + 1][j] +
                                    A[i][k + 2] * B[k + 2][j] + A[i][k + 3] * B[k + 3][j] +
                                    A[i][k + 4] * B[k + 4][j] + A[i][k + 5] * B[k + 5][j] +
                                    A[i][k + 6] * B[k + 6][j] + A[i][k + 7] * B[k + 7][j];
                            }
                        }
                        else if (M >= 4 && k_max - kk >= 4) {
                            for (; k <= k_max - 4; k += 4) {
                                sum += A[i][k] * B[k][j] + A[i][k + 1] * B[k + 1][j] +
                                    A[i][k + 2] * B[k + 2][j] + A[i][k + 3] * B[k + 3][j];
                            }
                        }
                        else if (M >= 2 && k_max - kk >= 2) {
                            for (; k <= k_max - 2; k += 2) {
                                sum += A[i][k] * B[k][j] + A[i][k + 1] * B[k + 1][j];
                            }
                        }

                        for (; k < k_max; k++) {
                            sum += A[i][k] * B[k][j];
                        }

                        C[i][j] += sum;
                    }
                }
            }
        }
    }
}

//проверка на правильность

bool test_correctness(int N) {
    cout << "\nпроверка (N=" << N << ")\n";

    float** A = alloc_matrix(N);
    float** B = alloc_matrix(N);
    float** C1 = alloc_matrix(N);
    float** C2 = alloc_matrix(N);
    float** C3 = alloc_matrix(N);
    float** C4 = alloc_matrix(N);

    fill_random(A, N);
    fill_random(B, N);

    mul_classic(A, B, C1, N);
    mul_transpose(A, B, C2, N, true);
    mul_buffer(A, B, C3, N, 1);
    mul_block(A, B, C4, N, 64, 1);

    bool ok1 = compare_matrices(C1, C2, N);
    bool ok2 = compare_matrices(C1, C3, N);
    bool ok3 = compare_matrices(C1, C4, N);

    cout << "Классический - транспонирование: " << (ok1 ? "OK" : "FAIL") << endl;
    cout << "Классический - буферизация: " << (ok2 ? "OK" : "FAIL") << endl;
    cout << "Классический - блочный: " << (ok3 ? "OK" : "FAIL") << endl;

    free_matrix(A, N);
    free_matrix(B, N);
    free_matrix(C1, N);
    free_matrix(C2, N);
    free_matrix(C3, N);
    free_matrix(C4, N);

    return ok1 && ok2 && ok3;
}

//измерение производительности

double measure_time(void(*func)(float**, float**, float**, int, int, int),
    float** A, float** B, float** C, int N, int param1, int param2,
    const string& name, int repeats = 3) {

    double total = 0.0;

    for (int r = 0; r < repeats; r++) {
        zero_matrix(C, N);

        double t1 = get_time();
        func(A, B, C, N, param1, param2);
        double t2 = get_time();

        total += (t2 - t1);
    }

    double avg_time = total / repeats;
    double ops = 2.0 * N * N * N; 
    double gflops = ops / avg_time / 1e9;

    cout << fixed << setprecision(3);
    cout << name << ": время = " << avg_time * 1000 << " мс, ";
    cout << "производительность = " << gflops << " GFLOPS\n";

    return gflops;
}

void wrap_classic(float** A, float** B, float** C, int N, int, int) {
    mul_classic(A, B, C, N);
}

void wrap_transpose(float** A, float** B, float** C, int N, int, int) {
    mul_transpose(A, B, C, N, true);
}

void wrap_buffer(float** A, float** B, float** C, int N, int M, int) {
    mul_buffer(A, B, C, N, M);
}

void wrap_block(float** A, float** B, float** C, int N, int S, int M) {
    mul_block(A, B, C, N, S, M);
}


//эксперимент 1 

void experiment_debug_vs_release(int N) {
    cout << "\nэксперимент 1 (N=" << N << ")\n";
    cout << "Запускаес в Debug и Release режимах\n";
}


//эксперимерт 2

void experiment_transpose(int N) {
    cout << "\nэксперимент 2 транспонирование (N=" << N << ")\n";

    float** A = alloc_matrix(N);
    float** B = alloc_matrix(N);
    float** C = alloc_matrix(N);

    fill_random(A, N);
    fill_random(B, N);

    //классический
    double t1 = get_time();
    mul_classic(A, B, C, N);
    double t2 = get_time();
    double classic_time = t2 - t1;
    double classic_ops = 2.0 * N * N * N / classic_time / 1e9;

    //с транспонированием 
    double t3 = get_time();
    mul_transpose(A, B, C, N, true);
    double t4 = get_time();
    double trans_time_with = t4 - t3;
    double trans_ops_with = 2.0 * N * N * N / trans_time_with / 1e9;

    //с транспонированием (без учета времени)
    float** Bt = alloc_matrix(N);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            Bt[i][j] = B[j][i];

    double t5 = get_time();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * Bt[j][k];
            }
            C[i][j] = sum;
        }
    }
    double t6 = get_time();
    double trans_time_without = t6 - t5;
    double trans_ops_without = 2.0 * N * N * N / trans_time_without / 1e9;

    free_matrix(Bt, N);

    cout << fixed << setprecision(3);
    cout << "классическое умножение: " << classic_time * 1000 << " мс, " << classic_ops << " GFLOPS\n";
    cout << "с транспонированием (с учетом): " << trans_time_with * 1000 << " мс, " << trans_ops_with << " GFLOPS\n";
    cout << "с транспонированием (без учета): " << trans_time_without * 1000 << " мс, " << trans_ops_without << " GFLOPS\n";
    cout << "ускорение (с учетом): x" << classic_time / trans_time_with << endl;
    cout << "ускорение (без учета): x" << classic_time / trans_time_without << endl;

    free_matrix(A, N);
    free_matrix(B, N);
    free_matrix(C, N);
}

//эксперимент 3 буферизация

void experiment_buffer_unroll(int N) {
    cout << "\nэксперимент 3 буферизация(N=" << N << ")\n";

    float** A = alloc_matrix(N);
    float** B = alloc_matrix(N);
    float** C = alloc_matrix(N);

    fill_random(A, N);
    fill_random(B, N);

    vector<int> M_values = { 1, 2, 4, 8, 16 };
    vector<double> gflops;

    cout << "M\tВремя(мс)\tGFLOPS\n";

    for (int M : M_values) {
        zero_matrix(C, N);

        double t1 = get_time();
        mul_buffer(A, B, C, N, M);
        double t2 = get_time();

        double time_ms = (t2 - t1) * 1000;
        double ops = 2.0 * N * N * N;
        double gflop = ops / (t2 - t1) / 1e9;

        cout << M << "\t" << fixed << setprecision(3) << time_ms << "\t\t" << gflop << endl;
        gflops.push_back(gflop);
    }

    int best_M = M_values[0];
    double best_gflops = gflops[0];
    for (size_t i = 1; i < M_values.size(); i++) {
        if (gflops[i] > best_gflops) {
            best_gflops = gflops[i];
            best_M = M_values[i];
        }
    }

    cout << "\nоптимальная степень раскрутки M* = " << best_M << endl;
    cout << "максимальная производительность: " << best_gflops << " GFLOPS\n";

    free_matrix(A, N);
    free_matrix(B, N);
    free_matrix(C, N);
}

//эксперимент 4 блочный метод

void experiment_block(int N) {
    cout << "\nэксперимент 4 блочный метод (N=" << N << ")\n";

    float** A = alloc_matrix(N);
    float** B = alloc_matrix(N);
    float** C = alloc_matrix(N);

    fill_random(A, N);
    fill_random(B, N);

    vector<int> S_values = { 16, 32, 64, 128, 256, 512 };
    vector<int> M_values = { 1, 2, 4, 8 };

    cout << "\nЗависимость от размера блока S (M=4)\n";
    cout << "S\tВремя(мс)\tGFLOPS\n";

    double best_s_gflops = 0;
    int best_S = 0;

    for (int S : S_values) {
        if (S > N) continue;

        zero_matrix(C, N);

        double t1 = get_time();
        mul_block(A, B, C, N, S, 4);
        double t2 = get_time();

        double time_ms = (t2 - t1) * 1000;
        double ops = 2.0 * N * N * N;
        double gflop = ops / (t2 - t1) / 1e9;

        cout << S << "\t" << fixed << setprecision(3) << time_ms << "\t\t" << gflop << endl;

        if (gflop > best_s_gflops) {
            best_s_gflops = gflop;
            best_S = S;
        }
    }

    cout << "\nоптимальный размер блока S* = " << best_S << endl;

    cout << "\nзависимость от раскрутки M (S=" << best_S << ")\n";
    cout << "M\tВремя(мс)\tGFLOPS\n";

    double best_m_gflops = 0;
    int best_M = 0;

    for (int M : M_values) {
        zero_matrix(C, N);

        double t1 = get_time();
        mul_block(A, B, C, N, best_S, M);
        double t2 = get_time();

        double time_ms = (t2 - t1) * 1000;
        double ops = 2.0 * N * N * N;
        double gflop = ops / (t2 - t1) / 1e9;

        cout << M << "\t" << fixed << setprecision(3) << time_ms << "\t\t" << gflop << endl;

        if (gflop > best_m_gflops) {
            best_m_gflops = gflop;
            best_M = M;
        }
    }

    cout << "\nоптимальная раскрутка M* = " << best_M << endl;
    cout << "максимальная производительность блочного метода: " << best_m_gflops << " GFLOPS\n";
    cout << "оптимальные параметры: S*=" << best_S << ", M*=" << best_M << endl;

    free_matrix(A, N);
    free_matrix(B, N);
    free_matrix(C, N);
}

//эксперимент 5 сравнение всех методов

void experiment_compare_all() {
    cout << "\nэксперимент 5 сравнение всех методов\n";

    vector<int> N_values = { 64, 128, 256, 512, 1024 }; 

    cout << "\nN\tКлассич\tТрансп\tБуфер\tБлочный\n";
    cout << "\t(GFLOPS)\t(GFLOPS)\t(GFLOPS)\t(GFLOPS)\n";

    for (int N : N_values) {
        cout << N << "\t";

        float** A = alloc_matrix(N);
        float** B = alloc_matrix(N);
        float** C = alloc_matrix(N);

        fill_random(A, N);
        fill_random(B, N);

        //классический
        double t1 = get_time();
        mul_classic(A, B, C, N);
        double t2 = get_time();
        double classic_gflops = 2.0 * N * N * N / (t2 - t1) / 1e9;
        cout << fixed << setprecision(2) << classic_gflops << "\t\t";

        //транспонирование
        double t3 = get_time();
        mul_transpose(A, B, C, N, true);
        double t4 = get_time();
        double trans_gflops = 2.0 * N * N * N / (t4 - t3) / 1e9;
        cout << trans_gflops << "\t\t";

        //буферизация
        double t5 = get_time();
        mul_buffer(A, B, C, N, 4);
        double t6 = get_time();
        double buffer_gflops = 2.0 * N * N * N / (t6 - t5) / 1e9;
        cout << buffer_gflops << "\t\t";

        //блочный
        double t7 = get_time();
        mul_block(A, B, C, N, 64, 4);
        double t8 = get_time();
        double block_gflops = 2.0 * N * N * N / (t8 - t7) / 1e9;
        cout << block_gflops << endl;

        free_matrix(A, N);
        free_matrix(B, N);
        free_matrix(C, N);
    }
}


int main() {
    setlocale(LC_ALL, "Russian");

    test_correctness(8);

    //основные эксперименты
    int N = 512;  

    experiment_debug_vs_release(N);
    experiment_transpose(N);
    experiment_buffer_unroll(N);
    experiment_block(N);
    experiment_compare_all();

    cin.get();

    return 0;
}
