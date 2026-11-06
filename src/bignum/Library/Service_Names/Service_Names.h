#pragma once

#include <iostream>
#include <cmath>

#include "../Error/Error.h"
#include "../Thread_Pool/Thread_Pool.h"


#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define UNDERLINE   "\033[4m"
#define INVERT      "\033[7m"


#define FG_BLACK    "\033[30m"
#define FG_RED      "\033[31m"
#define FG_GREEN    "\033[32m"
#define FG_YELLOW   "\033[33m"
#define FG_BLUE     "\033[34m"
#define FG_MAGENTA  "\033[35m"
#define FG_CYAN     "\033[36m"
#define FG_WHITE    "\033[37m"


#define FG_BRIGHT_BLACK    "\033[90m" 
#define FG_BRIGHT_RED      "\033[91m"
#define FG_BRIGHT_GREEN    "\033[92m"
#define FG_BRIGHT_YELLOW   "\033[93m"
#define FG_BRIGHT_BLUE     "\033[94m"
#define FG_BRIGHT_MAGENTA  "\033[95m"
#define FG_BRIGHT_CYAN     "\033[96m"
#define FG_BRIGHT_WHITE    "\033[97m"


#define BG_BLACK    "\033[40m"
#define BG_RED      "\033[41m"
#define BG_GREEN    "\033[42m"
#define BG_YELLOW   "\033[43m"
#define BG_BLUE     "\033[44m"
#define BG_MAGENTA  "\033[45m"
#define BG_CYAN     "\033[46m"
#define BG_WHITE    "\033[47m"


#define BG_BRIGHT_BLACK    "\033[100m"
#define BG_BRIGHT_RED      "\033[101m"
#define BG_BRIGHT_GREEN    "\033[102m"
#define BG_BRIGHT_YELLOW   "\033[103m"
#define BG_BRIGHT_BLUE     "\033[104m"
#define BG_BRIGHT_MAGENTA  "\033[105m"
#define BG_BRIGHT_CYAN     "\033[106m"
#define BG_BRIGHT_WHITE    "\033[107m"







class Error;
class Int_Number;
class Quotient_Number;
class Real_Number;



template<typename T>
int Length_Int(T num) {
    if (num == 0) return 1;
    
    T n = num < 0 ? -num : num;
    
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    
    return (long)log10(n) + 1;
}







struct Mod998244353 {
    static constexpr int MOD = 998244353;
    static constexpr int ROOT = 3;
    static constexpr int MAX_LOG = 23;
};

struct Mod1004535809 {
    static constexpr int MOD = 1004535809;
    static constexpr int ROOT = 3;
    static constexpr int MAX_LOG = 21;
};

struct Mod469762049 {
    static constexpr int MOD = 469762049;
    static constexpr int ROOT = 3;
    static constexpr int MAX_LOG = 26;
};

struct Mod167772161 {
    static constexpr int MOD = 167772161;
    static constexpr int ROOT = 3;
    static constexpr int MAX_LOG = 25;
};

struct Mod104857601 {
    static constexpr int MOD = 104857601;
    static constexpr int ROOT = 3;
    static constexpr int MAX_LOG = 22;
};







template<class MOD_Num_>
struct NTT {
    static void transform(std::vector<int>& a, bool inverse) {
        MOD_Num_::Init_Mod_Num_();

        int n = static_cast<int>(a.size());
        

        if (n > (1 << MOD_Num_::MAX_LOG))
            throw Error_Size_Array();
        if ((n & (n - 1)) != 0)
            throw Error_Size_Array();
        

        int max_n = 1 << MOD_Num_::MAX_LOG;
        int log_n = 0;
        while ((1 << log_n) < n) log_n++;
        

        for (int i = 1, j = 0; i < n; i++) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1)
                j ^= bit;
            j ^= bit;
            
            if (i < j)
                std::swap(a[i], a[j]);
        }
        

        const std::vector<int>& w = inverse ? MOD_Num_::roots_inv : MOD_Num_::roots;
        
        
        for (int len = 2; len <= n; len <<= 1) {
            int step = max_n / len;
            int half = len >> 1;
            
            for (int i = 0; i < n; i += len) {
                for (int j = 0; j < half; j++) {
                    int w_val = w[j * step];
                    
                    int u = a[i + j];
                    int v = (1LL * a[i + j + half] * w_val) % MOD_Num_::MOD;
                    
                    a[i + j] = u + v;
                    if (a[i + j] >= MOD_Num_::MOD) a[i + j] -= MOD_Num_::MOD;
                    
                    a[i + j + half] = u - v;
                    if (a[i + j + half] < 0) a[i + j + half] += MOD_Num_::MOD;
                }
            }
        }
        
        if (inverse) {
            int n_inv = MOD_Num_::modpow(n, MOD_Num_::MOD - 2, MOD_Num_::MOD);
            for (int& x : a) x = (1LL * x * n_inv) % MOD_Num_::MOD;
        }
    }
};




template<class MOD_>
class Mod_Num_: public MOD_{
    static std::vector<int> roots;
    static std::vector<int> roots_inv;

    static bool initialized;

    static std::mutex init_mutex;

public:
    static void Init_Mod_Num_() {
        if (initialized) 
            return;
        
        std::lock_guard<std::mutex> lock(init_mutex);

        if (initialized) 
            return;
        
        int LEN_ARRAY = 1 << MOD_::MAX_LOG;
        
        roots.resize(LEN_ARRAY);
        roots_inv.resize(LEN_ARRAY);;
        
        
        int base = modpow(MOD_::ROOT, (MOD_::MOD - 1) / LEN_ARRAY, MOD_::MOD);
        int base_inv = modpow(base, MOD_::MOD - 2, MOD_::MOD);

        roots[0] = 1;
        roots_inv[0] = 1;
        for (int i = 1; i < LEN_ARRAY; ++i) {
            roots[i] = (1LL * roots[i-1] * base) % MOD_::MOD;
            roots_inv[i] = (1LL * roots_inv[i-1] * base_inv) % MOD_::MOD;
        }

        initialized = true;
    }
    static int modpow(int a, int n, int mod) {
        int res = 1;
        while (n) {
            if (n & 1) res = (1LL * res * a) % mod;
            a = (1LL * a * a) % mod;
            n >>= 1;
        }
        return res;
    }

    friend NTT<Mod_Num_<MOD_>>;
};




template<class MOD_>
std::vector<int> Mod_Num_<MOD_>::roots;

template<class MOD_>
std::vector<int> Mod_Num_<MOD_>::roots_inv;

template<class MOD_>
bool Mod_Num_<MOD_>::initialized = false;

template<class MOD_>
std::mutex Mod_Num_<MOD_>::init_mutex;








template<class Mod>
void PerformNTTMultiplication(int Power_2, std::vector<int>& Nu1, std::vector<int>& Nu2){
    std::thread thread1(NTT<Mod_Num_<Mod>>::transform, std::ref(Nu1), false);
    std::thread thread2(NTT<Mod_Num_<Mod>>::transform, std::ref(Nu2), false);

    thread1.join();
    thread2.join();

    for (int i = 0; i < (1 << Power_2); i++) 
        Nu1[i] = (1LL * Nu1[i] * Nu2[i]) % Mod::MOD;
    
    NTT<Mod_Num_<Mod>>::transform(Nu1, true);
    
};









