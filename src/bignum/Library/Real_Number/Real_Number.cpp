

#include "Real_Number.h"


std::atomic<long long> Real_Number::MAX_after_arithmetic_fractional_part_block = 9;



Real_Number::Real_Number(): number(), count_fractional_part_block(0), state(Standard_Number){}
Real_Number::Real_Number(int len_dedicated1, int plt_tail1): number(len_dedicated1, plt_tail1), count_fractional_part_block(0), state(Standard_Number){}

Real_Number::Real_Number(int n): number(n), count_fractional_part_block(0), state(Standard_Number){}
Real_Number::Real_Number(long long int n): number(n), count_fractional_part_block(0), state(Standard_Number){}

Real_Number::Real_Number(Int_Number&& Nu): number(Nu), count_fractional_part_block(0), state(Standard_Number){}
Real_Number::Real_Number(const Int_Number& Nu): number(Nu), count_fractional_part_block(0), state(Standard_Number){}



Real_Number::Real_Number(float n){
    long long mantissa;
    int shift;

    state = Standard_Number;

    extract_float(n, mantissa, shift);

    int shift_9 = shift / 9;
    int shift_ne_9 = ((shift % 9) + 9) % 9;

    if(shift < 0){
        if(shift_ne_9 != 0)
            shift_9--;
        
        while(shift_ne_9-- > 0)
            mantissa *= 10;

        number = mantissa;
        count_fractional_part_block = -shift_9;

        return;
    }

    while (shift_ne_9-- > 0)
        mantissa *= 10;
    
    number = mantissa;
    number << shift_9;
    count_fractional_part_block = 0;
}
Real_Number::Real_Number(double n){
    long long mantissa;
    int shift;

    state = Standard_Number;

    extract_double(n, mantissa, shift);
    
    int shift_9 = shift / 9;
    int shift_ne_9 = ((shift % 9) + 9) % 9;

    if(shift < 0){
        if(shift_ne_9 != 0)
            shift_9--;
        
        number = mantissa;
        while(shift_ne_9-- > 0)
            number *= 10;

        count_fractional_part_block = -shift_9;

        return;
    }

    number = mantissa;
    while (shift_ne_9-- > 0)
        number *= 10;
    
    number << shift_9;
    count_fractional_part_block = 0;
}
Real_Number::Real_Number(long double n){
    long long mantissa;
    int shift;

    state = Standard_Number;

    extract_longdouble(n, mantissa, shift);
    
    int shift_9 = shift / 9;
    int shift_ne_9 = ((shift % 9) + 9) % 9;

    if(shift < 0){
        if(shift_ne_9 != 0)
            shift_9--;
        
        number = mantissa;
        while(shift_ne_9-- > 0)
            number *= 10;

        count_fractional_part_block = -shift_9;

        return;
    }

    number = mantissa;
    while (shift_ne_9-- > 0)
        number *= 10;
    
    number << shift_9;
    count_fractional_part_block = 0;
}


Real_Number::Real_Number(const Quotient_Number& Q_old){
    


}






void Real_Number::All_Print(int count) const {
    std::lock_guard<std::mutex> lock(guard);

    number.All_Print();
    std::cout << "count_fractional_part_block = " << count_fractional_part_block << std::endl << std::endl;
}
void Real_Number::Technical_Print() const {
    std::lock_guard<std::mutex> lock(guard);

    number.All_Print();
    std::cout << "count_fractional_part_block = " << count_fractional_part_block << std::endl << std::endl;
}


