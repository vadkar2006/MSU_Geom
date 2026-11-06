#pragma once

#include <mutex>
#include <atomic>

#include "../Int_Number/Int_Number.h"
#include "../Real_Number/Real_Number.h"



class Quotient_Number {
    Int_Number up;
    Int_Number doun;
    bool sign_plus;

    static std::atomic<long long> fractional_part_output_length;

    mutable std::mutex guard;
    
    void Irreducible_Fraction_Bypass();

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

public:
    static void Set_Precision(long long i){
        if (i > 0)
            fractional_part_output_length = i;
    }
    
    Quotient_Number();
    Quotient_Number(int u);
    Quotient_Number(float u);
    Quotient_Number(double u);
    Quotient_Number(long double u);
    Quotient_Number(const Int_Number& u);
    Quotient_Number(const Int_Number& u1, const Int_Number& u2);
    Quotient_Number(const Int_Number& u1, const Int_Number& u2, bool plus);
    Quotient_Number(const Quotient_Number& q);

    Quotient_Number(Quotient_Number&& q);
    Quotient_Number(Int_Number&& u1, Int_Number&& u2);
    Quotient_Number(Int_Number&& u1, Int_Number&& u2, bool plus);
    
    
    void Make_Irreducible_Fraction();

    Real_Number To_Real_Number() const;


    int operator <=> (const Quotient_Number& Q) const;

    bool operator >(const Quotient_Number& Q) const{return (*this <=> Q) == 1 ? true : false;}
    bool operator <(const Quotient_Number& Q) const{return (*this <=> Q) == -1 ? true : false;}
    bool operator >= (const Quotient_Number& Q) const{return (*this <=> Q) == -1 ? false : true;}
    bool operator <= (const Quotient_Number& Q) const{return (*this <=> Q) == 1 ? false : true;}
    bool operator == (const Quotient_Number& Q) const{return (*this <=> Q) == 0 ? true : false;}
    bool operator != (const Quotient_Number& Q) const{return (*this <=> Q) == 0 ? false : true;}
    
    Quotient_Number& operator - ();
    Quotient_Number operator - () const;

    Quotient_Number operator+(const Quotient_Number& Q) const;
    Quotient_Number operator-(const Quotient_Number& Q) const;
    Quotient_Number operator*(const Quotient_Number& Q) const;
    Quotient_Number operator/(const Quotient_Number& Q) const;


    void operator=(Quotient_Number&& Q);
    void operator=(const Quotient_Number& Q);
    
    void operator+=(const Quotient_Number& Q);
    void operator-=(const Quotient_Number& Q);
    void operator*=(const Quotient_Number& Q);
    void operator/=(const Quotient_Number& Q);

    operator float() const;
    operator double() const;
    operator long double() const;
    
    friend std::ostream& operator<<(std::ostream& os, const Quotient_Number& Q);
    friend std::ofstream& operator<<(std::ofstream& of, const Quotient_Number& Q);

    friend std::istream& operator >> (std::istream& is, Quotient_Number& Q);
    friend std::ifstream& operator >> (std::ifstream& is, Quotient_Number& Q);
};



