#pragma once

#include <cstdint>
#include <cstring>


#include "../Int_Number/Int_Number.h"
#include "../Quotient_Number/Quotient_Number.h"












class Real_Number{
    static std::atomic<long long> MAX_after_arithmetic_fractional_part_block;

    // float (6 знаков)
    inline static void extract_float(float num, long long& mantissa, int& shift) {
        if (num == 0.0f) {
            mantissa = 0;
            shift = 0;
            return;
        }
        
        bool neg = num < 0;
        float abs_num = neg ? -num : num;
        
        int exp10 = static_cast<int>(std::floor(std::log10(abs_num)));
        float norm = abs_num / std::pow(10.0f, exp10);
        
        mantissa = static_cast<int>(norm * 1000000.0f + 0.5f);
        shift = exp10 - 6;
        
        if (neg) mantissa = -mantissa;
    }

    // double (15 знаков)
    inline static void extract_double(double num, long long& mantissa, int& shift) {
        if (num == 0.0) {
            mantissa = 0;
            shift = 0;
            return;
        }
        
        bool neg = num < 0;
        double abs_num = neg ? -num : num;
        
        int exp10 = static_cast<int>(std::floor(std::log10(abs_num)));
        double norm = abs_num / std::pow(10.0, exp10);
        
        mantissa = static_cast<long long>(norm * 1000000000000000.0 + 0.5);
        shift = exp10 - 15;
        
        if (neg) mantissa = -mantissa;
    }

    // long double (18 знаков)
    inline static void extract_longdouble(long double num, long long& mantissa, int& shift) {
        if (num == 0.0L) {
            mantissa = 0;
            shift = 0;
            return;
        }
        
        bool neg = num < 0;
        long double abs_num = neg ? -num : num;
        
        int exp10 = static_cast<int>(std::floor(std::log10(abs_num)));
        long double norm = abs_num / std::pow(10.0L, exp10);
        
        mantissa = static_cast<long long>(norm * 1000000000000000000.0L + 0.5L);
        shift = exp10 - 18;
        
        if (neg) mantissa = -mantissa;
    }


    enum State{Standard_Number, Infinity, Uncertainty};

    Int_Number number;
    State state;
    int count_fractional_part_block;

    mutable std::mutex guard;

public:
    Real_Number();
    Real_Number(int len_dedicated1, int plt_tail1);

    Real_Number(int n);
    Real_Number(long long int n);

    Real_Number(float n);
    Real_Number(double n);
    Real_Number(long double n);


    explicit Real_Number(Int_Number&& Nu);
    explicit Real_Number(const Int_Number& Nu);

    explicit Real_Number(const Quotient_Number& Q_old);


    Real_Number(const std::string& line_num);




    void All_Print(int count = 10) const;
    void Technical_Print() const;



};