#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <thread>



#include "../Service_Names/Service_Names.h"
#include "../Error/Error.h"
#include "../Thread_Pool/Thread_Pool.h"







class Int_Number {
    int* num;
    int ptr_head;
    int plt_tail;
    int len_dedicated;
    long len_num;
    bool sign_plus;

    mutable std::mutex guard;

    static Thread_Pool thread_pool;
    static Thread_Pool sub_thread_pool;

    static void Division_NOT_Bypass(Int_Number& R, Int_Number& Nu2, Int_Number& Q){
        if(R.len_num < Nu2.len_num)
            return;
        
        if(R < Nu2)
            return;

        bool plus_flag = true;

        while(Abs(R) >= Nu2){
            if(!plus_flag)
                -R;

            long long q;
            int move_flag = 0;


            if (R.num[R.ptr_head] >= Nu2.num[Nu2.ptr_head])
                q = (R.num[R.ptr_head] / Nu2.num[Nu2.ptr_head]);
            
            else{
                move_flag = 1;
                q = (((long long)R.num[R.ptr_head] * (long long)1000000000 + (long long)R.num[R.ptr_head-1]) / (Nu2.num[Nu2.ptr_head] + 1));
            }
            
            int go = ((R.len_num - 1) / 9 + 1) - ((Nu2.len_num - 1) / 9 + 1) - move_flag;

            if(plus_flag){
                R -= (Nu2 * q) << go;
                Q += Int_Number(q) << go;
            }
            else{
                R += (Nu2 * q) << go;
                Q -= Int_Number(q) << go;
            }


            if(R.Is_plus())
                plus_flag = true;
            else
                plus_flag = false;
        }
    }
    friend std::ostream& operator<<(std::ostream& os, const Quotient_Number& Q);
    friend std::ofstream& operator<<(std::ofstream& of, const Quotient_Number& Q);



    void Relocation_Bypass(int count);

    Int_Number Copy_Bypass() const;


    int Comparator_Bypass(const Int_Number& Nu) const;
    int ABS_Comparator_Bypass(const Int_Number& Nu) const;


    void Only_Add_With_Shift_Bypass(const Int_Number& Nu, int shift = 0);

    void Add_Bypass(const Int_Number& Nu);
    void Subtract_Bypass(const Int_Number& Nu);



    //transition_block, Nu.num, this.num, poz_start_mas_block, poz_start_mas_in, poz_start_mas_out, count_block
    static void __local__Add_In_Bypass_Paralel(void** mas){
        long sum;

        int* transition_block = static_cast<int*>(mas[0]);
        int* Nu_num = static_cast<int*>(mas[1]);
        int* this_num = static_cast<int*>(mas[2]);

        int poz_start_mas_block = *static_cast<int*>(mas[3]);
        int poz_start_mas_in = *static_cast<int*>(mas[4]);
        int poz_start_mas_out = *static_cast<int*>(mas[5]);
        int count_block = *static_cast<int*>(mas[6]);

        for(int i = 0; i < count_block; i++){
            sum = this_num[poz_start_mas_in + i] + Nu_num[poz_start_mas_out + i];
            transition_block[poz_start_mas_block + i] = sum / 1000000000;
            this_num[poz_start_mas_in + i] = sum % 1000000000;
        }
        
        for(int i = 0; i < count_block-1; i++){
            sum = this_num[poz_start_mas_in + i + 1] + transition_block[poz_start_mas_block + i];
            transition_block[poz_start_mas_block + i + 1] += sum / 1000000000;
            this_num[poz_start_mas_in + i + 1] = sum % 1000000000;
            transition_block[poz_start_mas_block + i] = 0;
        }
    };
    void Add_In_Bypass_Paralel(const Int_Number& Nu);


    //transition_block, Nu.num, this.num, poz_start_mas_block, poz_start_mas_in, poz_start_mas_out, count_block
    static void __local_first__Subtract_In_Bypass_Paralel(void** mas){
        long subt;

        int* transition_block = static_cast<int*>(mas[0]);
        int* Nu_num = static_cast<int*>(mas[1]);
        int* this_num = static_cast<int*>(mas[2]);

        int poz_start_mas_block = *static_cast<int*>(mas[3]);
        int poz_start_mas_in = *static_cast<int*>(mas[4]);
        int poz_start_mas_out = *static_cast<int*>(mas[5]);
        int count_block = *static_cast<int*>(mas[6]);


        for(int i = 0; i < count_block; i++){
            subt = this_num[poz_start_mas_in + i] - Nu_num[poz_start_mas_out + i];
            
            if(subt < 0){
                transition_block[poz_start_mas_block + i] = -1;
                this_num[poz_start_mas_in + i] = 1000000000 + subt;
            }
            else{
                transition_block[poz_start_mas_block + i] = 0;
                this_num[poz_start_mas_in + i] = subt;
            }
        }
        
        for(int i = 0; i < count_block-1; i++){
            subt = this_num[poz_start_mas_in + i + 1] + transition_block[poz_start_mas_block + i];
            
            if(subt < 0){
                transition_block[poz_start_mas_block + i + 1] = -1;
                transition_block[poz_start_mas_block + i] = 0;
                this_num[poz_start_mas_in + i + 1] = 1000000000 + subt;
            }
            else{
                transition_block[poz_start_mas_block + i] = 0;
                this_num[poz_start_mas_in + i + 1] = subt;
            }
        }
    };
    static void __local_second__Subtract_In_Bypass_Paralel(void** mas){
        long subt;

        int* transition_block = static_cast<int*>(mas[0]);
        int* Nu_num = static_cast<int*>(mas[1]);
        int* this_num = static_cast<int*>(mas[2]);

        int poz_start_mas_block = *static_cast<int*>(mas[3]);
        int poz_start_mas_in = *static_cast<int*>(mas[4]);
        int poz_start_mas_out = *static_cast<int*>(mas[5]);
        int count_block = *static_cast<int*>(mas[6]);


        for(int i = 0; i < count_block; i++){
            subt = -(this_num[poz_start_mas_in + i]) + Nu_num[poz_start_mas_out + i];
            
            if(subt < 0){
                transition_block[poz_start_mas_block + i] = -1;
                this_num[poz_start_mas_in + i] = 1000000000 + subt;
            }
            else{
                transition_block[poz_start_mas_block + i] = 0;
                this_num[poz_start_mas_in + i] = subt;
            }
        }
        
        for(int i = 0; i < count_block-1; i++){
            subt = this_num[poz_start_mas_in + i + 1] + transition_block[poz_start_mas_block + i];
            
            if(subt < 0){
                transition_block[poz_start_mas_block + i + 1] = -1;
                transition_block[poz_start_mas_block + i] = 0;
                this_num[poz_start_mas_in + i + 1] = 1000000000 + subt;
            }
            else{
                transition_block[poz_start_mas_block + i] = 0;
                this_num[poz_start_mas_in + i + 1] = subt;
            }
        }
    };
    void Subtract_In_Bypass_Paralel(const Int_Number& Nu);


    Int_Number& Shift_Right_Bypass(int i);
    Int_Number& Shift_Left_Bypass(int i);


    void Multiplication_Bypass(int q);
    void Multiplication_Bypass(const Int_Number& Nu);
    //В НЕЕ НЕЛЬЗЯ ПИХАТЬ ОДИН И ТОТ ЖЕ ОБЬЕКТ(ЧИСЛО)


    template<class Mod>
    static void __local_Mod__NTT_Transform(void** mas){
        std::vector<int>* ve = static_cast<std::vector<int>*>(mas[0]);
        bool* inversee = static_cast<bool*>(mas[1]);

        NTT<Mod_Num_<Mod>>::transform(*ve, *inversee);
    }
    template<class Mod>
    static void __local_Mod__NTT_Outward_Bypass_Paralel(void** mas){
        void* mas_1[2];
        void* mas_2[2];

        bool false1 = false;

        mas_1[0] = mas[1];
        mas_2[0] = mas[2];
        mas_1[1] = &false1;
        mas_2[1] = &false1;

        Task task1(&__local_Mod__NTT_Transform<Mod>, mas_1);
        Task task2(&__local_Mod__NTT_Transform<Mod>, mas_2);

        sub_thread_pool.Sending_To_Processing(&task1);
        sub_thread_pool.Sending_To_Processing(&task2);


        int* Power_2 = static_cast<int*>(mas[0]);

        std::vector<int>* nu1 = static_cast<std::vector<int>*>(mas[1]);
        std::vector<int>* nu2 = static_cast<std::vector<int>*>(mas[2]);

        task1.Wait_Completed();
        task2.Wait_Completed();

        for (int i = 0; i < (1 << *Power_2); i++) 
            (*nu1)[i] = (1LL * (*nu1)[i] * (*nu2)[i]) % Mod::MOD;
        
        NTT<Mod_Num_<Mod>>::transform(*nu1, true);
    }
    //Nu1_MOD1 Nu1_MOD2 Nu1_MOD3   start count   P1 P2 P3  M12 M123    inv_P1_mod_P2 inv_M12_mod_P3 inv_P1_mod_P3    ans
    static void __local_CRT__3_Outward_Bypass(void** mas){ 
        std::vector<int>* Nu1_MOD1 = static_cast<std::vector<int>*>(mas[0]);
        std::vector<int>* Nu1_MOD2 = static_cast<std::vector<int>*>(mas[1]);
        std::vector<int>* Nu1_MOD3 = static_cast<std::vector<int>*>(mas[2]);

        long long int* start = static_cast<long long int*>(mas[3]);
        long long int* count = static_cast<long long int*>(mas[4]);

        Int_Number* P1 = static_cast<Int_Number*>(mas[5]);
        Int_Number* P2 = static_cast<Int_Number*>(mas[6]);
        Int_Number* P3 = static_cast<Int_Number*>(mas[7]);
        Int_Number* M12 = static_cast<Int_Number*>(mas[8]);
        Int_Number* M123 = static_cast<Int_Number*>(mas[9]);

        long long int* inv_P1_mod_P2 = static_cast<long long int*>(mas[10]);
        long long int* inv_M12_mod_P3 = static_cast<long long int*>(mas[11]);
        long long int* inv_P1_mod_P3 = static_cast<long long int*>(mas[12]);

        Int_Number* ans = static_cast<Int_Number*>(mas[13]);

        for (long long int i = 0; i < *count; i++) {
            long long int r1 = (*Nu1_MOD1)[i + *start];
            long long int r2 = (*Nu1_MOD2)[i + *start];
            long long int r3 = (*Nu1_MOD3)[i + *start];
            

            long long int diff2 = (r2 - r1) % Mod1004535809::MOD;
            if (diff2 < 0) 
                diff2 += Mod1004535809::MOD;
            long long int t2 = (diff2 * *inv_P1_mod_P2) % Mod1004535809::MOD;
            Int_Number X12 = Int_Number(r1) + Int_Number(t2) * (*P1);
            

            Int_Number X12_mod_P3_pred = X12 % (*P3);
            long long int X12_mod_P3 = X12_mod_P3_pred.num[X12_mod_P3_pred.plt_tail];

            long long int diff3 = (r3 - X12_mod_P3) % Mod469762049::MOD;
            if (diff3 < 0) 
                diff3 += Mod469762049::MOD;
            long long int t3 = (diff3 * *inv_M12_mod_P3) % Mod469762049::MOD;
            
            Int_Number X = X12 + Int_Number(t3) * (*M12);
            
            ans -> Only_Add_With_Shift_Bypass(X, i);
        }
    }
    //Nu1_MOD1 Nu1_MOD2 Nu1_MOD3 Nu1_MOD4   start count   P1 P2 P3 P4  M12 M123 M1234    inv_P1_mod_P2 inv_M12_mod_P3 inv_M123_mod_P4    ans
    static void __local_CRT__4_Outward_Bypass(void** mas){ 
        std::vector<int>* Nu1_MOD1 = static_cast<std::vector<int>*>(mas[0]);
        std::vector<int>* Nu1_MOD2 = static_cast<std::vector<int>*>(mas[1]);
        std::vector<int>* Nu1_MOD3 = static_cast<std::vector<int>*>(mas[2]);
        std::vector<int>* Nu1_MOD4 = static_cast<std::vector<int>*>(mas[3]);

        long long int* start = static_cast<long long int*>(mas[4]);
        long long int* count = static_cast<long long int*>(mas[5]);

        Int_Number* P1 = static_cast<Int_Number*>(mas[6]);
        Int_Number* P2 = static_cast<Int_Number*>(mas[7]);
        Int_Number* P3 = static_cast<Int_Number*>(mas[8]);
        Int_Number* P4 = static_cast<Int_Number*>(mas[9]);
        Int_Number* M12 = static_cast<Int_Number*>(mas[10]);
        Int_Number* M123 = static_cast<Int_Number*>(mas[11]);
        Int_Number* M1234 = static_cast<Int_Number*>(mas[12]);

        long long int* inv_P1_mod_P2 = static_cast<long long int*>(mas[13]);
        long long int* inv_M12_mod_P3 = static_cast<long long int*>(mas[14]);
        long long int* inv_M123_mod_P4 = static_cast<long long int*>(mas[15]);

        Int_Number* ans = static_cast<Int_Number*>(mas[16]);

        for (long long int i = 0; i < *count; i++) {
            long long int r1 = (*Nu1_MOD1)[i + *start];
            long long int r2 = (*Nu1_MOD2)[i + *start];
            long long int r3 = (*Nu1_MOD3)[i + *start];
            long long int r4 = (*Nu1_MOD4)[i + *start];
            

            long long int diff2 = (r2 - r1) % Mod1004535809::MOD;
            if (diff2 < 0) 
                diff2 += Mod1004535809::MOD;
            long long int t2 = (diff2 * *inv_P1_mod_P2) % Mod1004535809::MOD;
            Int_Number X12 = Int_Number(r1) + Int_Number(t2) * (*P1);
            

            Int_Number X12_mod_P3_pred = X12 % *P3;
            long long int X12_mod_P3 = X12_mod_P3_pred.num[X12_mod_P3_pred.plt_tail];

            long long int diff3 = (r3 - X12_mod_P3) % Mod469762049::MOD;
            if (diff3 < 0) 
                diff3 += Mod469762049::MOD;
            long long int t3 = (diff3 * *inv_M12_mod_P3) % Mod469762049::MOD;
            
            Int_Number X123 = X12 + Int_Number(t3) * *M12;
            

            Int_Number X123_mod_P4_pred = X123 % *P4;
            long long int X123_mod_P4 = X123_mod_P4_pred.num[X123_mod_P4_pred.plt_tail];

            long long int diff4 = (r4 - X123_mod_P4) % Mod167772161::MOD;
            if (diff4 < 0) 
                diff4 += Mod167772161::MOD;
            long long int t4 = (diff4 * *inv_M123_mod_P4) % Mod167772161::MOD;
            
            Int_Number X = X123 + Int_Number(t4) * *M123;
            

            ans -> Only_Add_With_Shift_Bypass(X, i);
        }
    }

    Int_Number Multiplication_NTT_Outward_Bypass(const Int_Number& Nu) const;
    Int_Number Multiplication_NTT_Outward_Thread_Pool_Bypass(const Int_Number& Nu) const;
    


public:
    
    Int_Number();
    Int_Number(int len_dedicated1, int plt_tail1);
    Int_Number(int n);
    Int_Number(long long int n);
    Int_Number(Int_Number&& Nu);
    Int_Number(const Int_Number& Nu);
    Int_Number(const std::string& line_num);

    
    
    void All_Print(int count = 10) const;
    void Technical_Print() const;
    void Relocation(int count = 3);
    bool Is_plus() const;
    int Length_Number() const;
    int Last_Element_Get() const;
    friend Int_Number& Abs(Int_Number& Nu);

    std::string To_String() const;
    std::string To_Clear_String() const;

    

    void operator = (Int_Number&& Nu);
    void operator = (const Int_Number& Nu);
    
    
    int operator <=> (const Int_Number& Nu) const;
    
    bool operator >(const Int_Number& Nu) const{return (*this <=> Nu) == 1 ? true : false;}
    bool operator <(const Int_Number& Nu) const{return (*this <=> Nu) == -1 ? true : false;}
    bool operator >= (const Int_Number& Nu) const{return (*this <=> Nu) == -1 ? false : true;}
    bool operator <= (const Int_Number& Nu) const{return (*this <=> Nu) == 1 ? false : true;}
    bool operator == (const Int_Number& Nu) const{return (*this <=> Nu) == 0 ? true : false;}
    bool operator != (const Int_Number& Nu) const{return (*this <=> Nu) == 0 ? false : true;}
    
    
    Int_Number& operator - ();
    void operator += (const Int_Number& Nu);
    void operator -= (const Int_Number& Nu);
    Int_Number operator +(const Int_Number& Nu) const;
    Int_Number operator -(const Int_Number& Nu) const;
    
    
    Int_Number& operator << (int i);
    Int_Number& operator >> (int i);
    
    
    void operator *= (int q);
    void operator *= (const Int_Number& Nu);
    Int_Number operator * (int q) const;
    Int_Number operator * (const Int_Number& Nu) const;
    
    
    void operator /= (const Int_Number& Nu);
    void operator %= (const Int_Number& Nu);
    Int_Number operator / (const Int_Number& Nu) const;
    Int_Number operator % (const Int_Number& Nu) const;
    
    Quotient_Number operator | (const Int_Number& Nu1) const;

    friend Int_Number NOD(const Int_Number& Nu1, const Int_Number& Nu2);

    
    friend std::ostream& operator << (std::ostream& os, const Int_Number& Nu);
    friend std::ofstream& operator << (std::ofstream& of, const Int_Number& Nu);

    friend std::istream& operator >> (std::istream& is, Int_Number& Nu);
    friend std::ifstream& operator >> (std::ifstream& is, Int_Number& Nu);
    
    
    ~Int_Number();
};


