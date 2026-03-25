#include <iostream>
#include <windows.h>
#include <intrin.h>
#include <string>
#include <vector>

using namespace std;

//хранение информации о кэше
struct CacheInfo {
    int level;          
    int size_kb;      
    int ways;           
    int line_size;      
    int threads_share;  
    string type;       
};

int extract_bits(int value, int start_bit, int bit_count) {
    return (value >> start_bit) & ((1 << bit_count) - 1);
}

bool check_bit(int value, int bit) {
    return (value >> bit) & 1;
}

//получение информации о процессоре через CPUID
void get_cpu_info() {
    int cpuInfo[4];  


    __cpuid(cpuInfo, 0);
    int max_basic_func = cpuInfo[0]; 

    char vendor[13];
    memcpy(vendor, &cpuInfo[1], 4);      
    memcpy(vendor + 4, &cpuInfo[3], 4);  
    memcpy(vendor + 8, &cpuInfo[2], 4);  
    vendor[12] = '\0';

    cout << "Информация о процессоре\n";
    cout << "Производитель: " << vendor << endl;
    cout << "Макс. базовый лист: 0x" << hex << max_basic_func << dec << endl;

    // получение названия процессора (CPU brand string)

    char brand[49] = { 0 };
    __cpuid(cpuInfo, 0x80000000);
    int max_ext_func = cpuInfo[0];

    if (max_ext_func >= 0x80000004) {
        __cpuid((int*)&brand[0], 0x80000002);
        __cpuid((int*)&brand[16], 0x80000003);
        __cpuid((int*)&brand[32], 0x80000004);
        cout << "Модель: " << brand << endl;
    }
    else {
        cout << "модель: не поддерживается\n";
    }

    //частота процессора

    if (max_basic_func >= 0x16) {
        __cpuid(cpuInfo, 0x16);
        int base_freq = cpuInfo[0] & 0xFFFF;  
        int max_freq = cpuInfo[1] & 0xFFFF;    
        int bus_freq = cpuInfo[2] & 0xFFFF;    

        cout << "\nЧастота:\n";
        cout << "  Базовая: " << base_freq << " МГц (" << base_freq / 1000.0 << " ГГц)\n";
        cout << "  Максимальная (Boost): " << max_freq << " МГц (" << max_freq / 1000.0 << " ГГц)\n";
        cout << "  Частота шины: " << bus_freq << " МГц\n";
    }
    else {
        cout << "\nчастота: информация не поддерживается\n";
    }

    // версия процессора и базовые флаги

    if (max_basic_func >= 1) {
        __cpuid(cpuInfo, 1);

        int stepping = cpuInfo[0] & 0xF;
        int model = (cpuInfo[0] >> 4) & 0xF;
        int family = (cpuInfo[0] >> 8) & 0xF;
        int ext_model = (cpuInfo[0] >> 16) & 0xF;
        int ext_family = (cpuInfo[0] >> 20) & 0xFF;

        cout << "\nВерсия:\n";
        cout << "  Family: " << family << " (" << ext_family << " Extended)\n";
        cout << "  Model: " << model << " (" << ext_model << " Extended)\n";
        cout << "  Stepping: " << stepping << endl;

        bool htt = check_bit(cpuInfo[3], 28);  
        int max_logical = extract_bits(cpuInfo[1], 16, 8);  

        cout << "\nЯдра и потоки:\n";
        if (htt) {
            cout << "  Hyper-Threading: ПОДДЕРЖИВАЕТСЯ\n";
            cout << "  Макс. логических процессоров: " << max_logical << endl;
        }
        else {
            cout << "  Hyper-Threading: НЕ ПОДДЕРЖИВАЕТСЯ\n";
        }

        cout << "\nПоддержка расширений (EDX):\n";
        cout << "  FPU: " << (check_bit(cpuInfo[3], 0) ? "ДА" : "НЕТ") << endl;
        cout << "  TSC: " << (check_bit(cpuInfo[3], 4) ? "ДА" : "НЕТ") << endl;
        cout << "  MMX: " << (check_bit(cpuInfo[3], 23) ? "ДА" : "НЕТ") << endl;
        cout << "  SSE: " << (check_bit(cpuInfo[3], 25) ? "ДА" : "НЕТ") << endl;
        cout << "  SSE2: " << (check_bit(cpuInfo[3], 26) ? "ДА" : "НЕТ") << endl;

        cout << "\nПоддержка расширений (ECX):\n";
        cout << "  SSE3: " << (check_bit(cpuInfo[2], 0) ? "ДА" : "НЕТ") << endl;
        cout << "  SSSE3: " << (check_bit(cpuInfo[2], 9) ? "ДА" : "НЕТ") << endl;
        cout << "  SSE4.1: " << (check_bit(cpuInfo[2], 19) ? "ДА" : "НЕТ") << endl;
        cout << "  SSE4.2: " << (check_bit(cpuInfo[2], 20) ? "ДА" : "НЕТ") << endl;
        cout << "  AVX: " << (check_bit(cpuInfo[2], 28) ? "ДА" : "НЕТ") << endl;
        cout << "  FMA3: " << (check_bit(cpuInfo[2], 12) ? "ДА" : "НЕТ") << endl;
    }


    if (max_basic_func >= 7) {
        __cpuidex(cpuInfo, 7, 0);

        cout << "\nПоддержка расширений:\n";
        cout << "  AVX2: " << (check_bit(cpuInfo[1], 5) ? "ДА" : "НЕТ") << endl;  
        cout << "  AVX512F: " << (check_bit(cpuInfo[1], 16) ? "ДА" : "НЕТ") << endl; 
        cout << "  SHA: " << (check_bit(cpuInfo[1], 29) ? "ДА" : "НЕТ") << endl;   

        cout << "  RTM (TSX): " << (check_bit(cpuInfo[1], 11) ? "ДА" : "НЕТ") << endl;

        __cpuidex(cpuInfo, 7, 1);
        cout << "  GFNI: " << (check_bit(cpuInfo[3], 8) ? "ДА" : "НЕТ") << endl;  
    }

    //расширенные функции 

    if (max_ext_func >= 0x80000001) {
        __cpuid(cpuInfo, 0x80000001);

        cout << "\nПоддержка расширений AMD:\n";
        cout << "  SSE4a: " << (check_bit(cpuInfo[2], 6) ? "ДА" : "НЕТ") << endl;   
        cout << "  FMA4: " << (check_bit(cpuInfo[2], 16) ? "ДА" : "НЕТ") << endl; 
        cout << "  3DNow!: " << (check_bit(cpuInfo[3], 31) ? "ДА" : "НЕТ") << endl; 
    }

    cout << "кэш-память:\n";

    if (max_basic_func >= 4) {
        vector<CacheInfo> caches;
        int subleaf = 0;

        while (true) {
            __cpuidex(cpuInfo, 4, subleaf);

            int cache_type = extract_bits(cpuInfo[0], 0, 5);  

            if (cache_type == 0) break;  

            CacheInfo cache;
            cache.level = extract_bits(cpuInfo[0], 5, 3); 
            cache.threads_share = extract_bits(cpuInfo[0], 14, 25) + 1;  
            cache.line_size = extract_bits(cpuInfo[1], 0, 12) + 1;  
            cache.ways = extract_bits(cpuInfo[1], 22, 10) + 1; 
            int sets = cpuInfo[2] + 1;  

            int size_bytes = cache.line_size * cache.ways * sets;
            cache.size_kb = size_bytes / 1024;

            if (cache_type == 1) cache.type = "Data";
            else if (cache_type == 2) cache.type = "Instruction";
            else if (cache_type == 3) cache.type = "Unified";
            else cache.type = "Unknown";

            caches.push_back(cache);
            subleaf++;
        }

        for (const auto& cache : caches) {
            cout << "\nL" << cache.level << " " << cache.type << " кэш:\n";
            cout << "  Размер: " << cache.size_kb << " KB\n";
            cout << "  Строка кэша: " << cache.line_size << " байт\n";
            cout << "  Ассоциативность: " << cache.ways << "-way\n";
            cout << "  Кол-во наборов: " << cache.size_kb * 1024 / (cache.line_size * cache.ways) << "\n";
            cout << "  Делят потоков: " << cache.threads_share << "\n";
        }

        if (caches.empty() && max_ext_func >= 0x8000001D) {
            int subleaf = 0;
            while (true) {
                __cpuidex(cpuInfo, 0x8000001D, subleaf);
                int cache_type = extract_bits(cpuInfo[0], 0, 5);
                if (cache_type == 0) break;

                int level = extract_bits(cpuInfo[0], 5, 3);
                int size_kb = cpuInfo[1] / 1024;
                int ways = extract_bits(cpuInfo[1], 22, 10) + 1;
                int line_size = extract_bits(cpuInfo[1], 0, 12) + 1;

                string type;
                if (cache_type == 1) type = "Data";
                else if (cache_type == 2) type = "Instruction";
                else type = "Unified";

                cout << "\nL" << level << " " << type << " кэш:\n";
                cout << "  Размер: " << size_kb << " KB\n";
                cout << "  Строка кэша: " << line_size << " байт\n";
                cout << "  Ассоциативность: " << ways << "-way\n";

                subleaf++;
            }
        }
    }
    else {
        cout << "информация о кэше не поддерживается\n";
    }


    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    cout << "информация об ОС\n";
    cout << "Логических процессоров: " << sysInfo.dwNumberOfProcessors << endl;

    if (max_basic_func >= 4) {
        __cpuidex(cpuInfo, 4, 0);
        int cores_per_package = extract_bits(cpuInfo[0], 26, 6) + 1;
        cout << "Физических ядер (по CPUID): " << cores_per_package << endl;
    }

    cout << "\nПрограмма завершена. Нажмите Enter...";
    cin.get();
}

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    get_cpu_info();

    return 0;
