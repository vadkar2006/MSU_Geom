#pragma onсe


#include "Int_Number.h"
#include "Library/Service_Names/Service_Names.h"
#include "Library/Quotient_Number/Quotient_Number.h"



Thread_Pool Int_Number::thread_pool;
Thread_Pool Int_Number::sub_thread_pool;




void Int_Number::Relocation_Bypass(int count) {
    if (count == 1) return;

    int len_dedicated_new = len_dedicated * count + 1;
    int* num_new = new int[len_dedicated_new];

    int plt_tail_new =  len_dedicated_new/2 - (ptr_head - plt_tail)/2;
    int ptr_head_new = plt_tail_new + (ptr_head - plt_tail);

    int i = 0;
    int j = plt_tail;
    while(i < len_dedicated_new){
        if(i < plt_tail_new)
            num_new[i++] = 0;
        else if(i <= ptr_head_new)
            num_new[i++] = num[j++];
        else
            num_new[i++] = 0;
    }

    int* num_old = num;
    num = num_new;

    len_dedicated = len_dedicated_new;
    ptr_head = ptr_head_new;
    plt_tail = plt_tail_new;

    delete[] num_old;
}

Int_Number Int_Number::Copy_Bypass() const{
    Int_Number copy_Nu(0);

    copy_Nu.Add_Bypass(*this);

    return std::move(copy_Nu);
}


int Int_Number::Comparator_Bypass(const Int_Number& Nu) const{
    if(this == &Nu)
        return 0;

    if(sign_plus && !Nu.sign_plus) return 1;
    if(!sign_plus && Nu.sign_plus) return -1;

    bool flag = sign_plus;

    if(len_num > Nu.len_num)
        return flag ? 1 : -1;
    if(len_num < Nu.len_num)
        return flag ? -1 : 1;
    
    for(int i = 0; i <= ptr_head - plt_tail; i++){
        if (num[ptr_head - i] < Nu.num[Nu.ptr_head - i])
            return flag ? -1 : 1;
        if (num[ptr_head - i] > Nu.num[Nu.ptr_head - i])
            return flag ? 1 : -1;
    }

    return 0;
}
int Int_Number::ABS_Comparator_Bypass(const Int_Number& Nu) const{
    if(this == &Nu)
        return 0;


    if(len_num > Nu.len_num)
        return 1;
    if(len_num < Nu.len_num)
        return -1;
    
    for(int i = 0; i <= ptr_head - plt_tail; i++){
        if (num[ptr_head - i] < Nu.num[Nu.ptr_head - i])
            return -1;
        if (num[ptr_head - i] > Nu.num[Nu.ptr_head - i])
            return 1;
    }

    return 0;
}


void Int_Number::Only_Add_With_Shift_Bypass(const Int_Number& Nu, int shift){
    if(Nu.ABS_Comparator_Bypass(0) == 0)
        return;
    
    if(len_dedicated <= std::max(ptr_head, Nu.ptr_head + shift) + 1)
        this -> Relocation_Bypass(2 * (Nu.ptr_head + shift) / len_dedicated + 2);

    long carry = 0;
    int i = plt_tail + shift;
    int j = Nu.plt_tail;

    while (i <= ptr_head && j <= Nu.ptr_head) {
        long sum = (long)num[i] + (long)Nu.num[j] + carry;
        num[i] = sum % 1000000000;
        carry = sum / 1000000000;
        i++;
        j++;
    }

    while (i <= ptr_head) {
        long sum = (long)num[i] + carry;
        num[i] = sum % 1000000000;
        carry = sum / 1000000000;
        i++;
    }

    while (j <= Nu.ptr_head) {
        long sum = (long)Nu.num[j] + carry;
        num[i] = sum % 1000000000;
        carry = sum / 1000000000;
        i++;
        j++;
    }

    ptr_head = i-1;
    if (carry > 0) {
        num[i] = carry;
        ptr_head = i;
    }

    len_num = 9 * (ptr_head - plt_tail) + Length_Int(num[ptr_head]);
    return;
}

void Int_Number::Add_Bypass(const Int_Number& Nu) {
    if(len_dedicated <= std::max(ptr_head, Nu.ptr_head) + 1)
        this -> Relocation_Bypass(2 * Nu.ptr_head / len_dedicated + 2);

    // одинаковые знаки
    if(sign_plus == Nu.sign_plus){ 
        long carry = 0;
        int i = plt_tail;
        int j = Nu.plt_tail;

        while (i <= ptr_head && j <= Nu.ptr_head) {
            long sum = (long)num[i] + (long)Nu.num[j] + carry;
            num[i] = sum % 1000000000;
            carry = sum / 1000000000;
            i++;
            j++;
        }

        while (i <= ptr_head) {
            long sum = (long)num[i] + carry;
            num[i] = sum % 1000000000;
            carry = sum / 1000000000;
            i++;
        }

        while (j <= Nu.ptr_head) {
            long sum = (long)Nu.num[j] + carry;
            num[i] = sum % 1000000000;
            carry = sum / 1000000000;
            i++;
            j++;
        }

        ptr_head = i-1;
        if (carry > 0) {
            num[i] = carry;
            ptr_head = i;
        }

        len_num = 9 * (ptr_head - plt_tail) + Length_Int(num[ptr_head]);
        return;
    }

    // разные знаки
    sign_plus = !sign_plus;
    this -> Subtract_Bypass(Nu);

    if((this -> len_num == 1) && (this -> num[plt_tail] == 0)){
        sign_plus = true;
        return;
    }

    sign_plus = !sign_plus;
    return;
}
void Int_Number::Subtract_Bypass(const Int_Number& Nu) {
    if(len_dedicated <= std::max(ptr_head, Nu.ptr_head) + 1)
        this -> Relocation_Bypass(Nu.ptr_head / len_dedicated + 2);

    int conpar = this -> ABS_Comparator_Bypass(Nu);
    
    // одинаковые знаки this равно по модулю
    if((sign_plus == Nu.sign_plus) && (conpar == 0)){
        for(int i = plt_tail; i <= ptr_head; i++)
            this -> num[i] = 0;
        
        len_num = 1;
        ptr_head = plt_tail;
        sign_plus = true;
        return;
    }
    
    // одинаковые знаки this больше по модулю
    if((sign_plus == Nu.sign_plus) && (conpar == 1)){ 
        long carry = 0;
        int i = plt_tail;
        int j = Nu.plt_tail;

        while (i <= ptr_head && j <= Nu.ptr_head) {
            long sum = (long)num[i] - (long)Nu.num[j] - carry;
            if(sum < 0){
                num[i] = sum + 1000000000;
                carry = 1;
            }
            else{
                num[i] = sum;
                carry = 0;
            }
            i++;
            j++;
        }
        int qwe = 1;

        while (i <= ptr_head) {
            long sum = (long)num[i] - carry;
            if(sum < 0){
                num[i] = sum + 1000000000;
                carry = 1;
            }
            else{
                num[i] = sum;
                carry = 0;
            }
            i++;
        }

        ptr_head = i-1;
        while ((num[ptr_head] == 0) && (ptr_head > plt_tail))
            ptr_head--;

        len_num = 9 * (ptr_head - plt_tail) + Length_Int(num[ptr_head]);
        return;
    }

    // одинаковые знаки this меньше по модулю
    if((sign_plus == Nu.sign_plus)){ 
        sign_plus = !sign_plus;
        long carry = 0;
        int i = plt_tail;
        int j = Nu.plt_tail;

        while (i <= ptr_head && j <= Nu.ptr_head) {
            long sum = -(long)num[i] + (long)Nu.num[j] - carry;
            if(sum < 0){
                num[i] = sum + 1000000000;
                carry = 1;
            }
            else{
                num[i] = sum;
                carry = 0;
            }
            i++;
            j++;
        }
        
        while (j <= Nu.ptr_head) {
            long sum = (long)Nu.num[j] - carry;
            if(sum < 0){
                num[i] = sum + 1000000000;
                carry = 1;
            }
            else{
                num[i] = sum;
                carry = 0;
            }
            i++;
            j++;
        }

        ptr_head = i-1;
        while ((num[ptr_head] == 0) && (ptr_head > plt_tail))
            ptr_head--;

        len_num = 9 * (ptr_head - plt_tail) + Length_Int(num[ptr_head]);
        return;
    }

    // разные знаки
    sign_plus = !sign_plus;
    this -> Add_Bypass(Nu);
    sign_plus = !sign_plus;
    return;
}


void Int_Number::Add_In_Bypass_Paralel(const Int_Number& Nu){
    if(len_dedicated <= std::max(ptr_head, Nu.ptr_head) + 1)
        this -> Relocation_Bypass(2 * Nu.ptr_head / len_dedicated + 2);
    

    //одинаковые знаки
    if(sign_plus == Nu.sign_plus){
        int count_thread_in_pool = thread_pool.Count_Worker();

        long max_count_block = std::max(ptr_head - plt_tail + 1, Nu.ptr_head - Nu.plt_tail + 1);
        long Nu_count_block = Nu.ptr_head - Nu.plt_tail + 1;
        std::vector<int> transition_block(max_count_block);
            
        for(int i = Nu_count_block - 1; i < max_count_block; i++)
            transition_block[i] = 0;

        //poz_start_mas_block, poz_start_mas_in, poz_start_mas_out, count_block
        int mas_variable_pool[count_thread_in_pool][4];
        //transition_block, Nu.num, this.num, poz_start_mas_block, poz_start_mas_in, poz_start_mas_out, count_block
        void* task_variable[count_thread_in_pool][7];
        Task mas_task[count_thread_in_pool];

        int Block_to_task = Nu_count_block / count_thread_in_pool;
        int Block_to_task_over = Nu_count_block % count_thread_in_pool;

        for(int i = 0; i < Block_to_task_over; i++){
            mas_variable_pool[i][0] = i * (Block_to_task + 1);
            mas_variable_pool[i][1] = mas_variable_pool[i][0] + plt_tail;
            mas_variable_pool[i][2] = mas_variable_pool[i][0] + Nu.plt_tail;
            mas_variable_pool[i][3] = Block_to_task + 1;
        }
        for(int i = 0; i < count_thread_in_pool - Block_to_task_over; i++){
            mas_variable_pool[i + Block_to_task_over][0] = Block_to_task_over * (Block_to_task + 1) + i * Block_to_task;
            mas_variable_pool[i + Block_to_task_over][1] = mas_variable_pool[i + Block_to_task_over][0] + plt_tail;
            mas_variable_pool[i + Block_to_task_over][2] = mas_variable_pool[i + Block_to_task_over][0] + Nu.plt_tail;
            mas_variable_pool[i + Block_to_task_over][3] = Block_to_task;
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            task_variable[i][0] = transition_block.data();
            task_variable[i][1] = Nu.num;
            task_variable[i][2] = num;

            task_variable[i][3] = &mas_variable_pool[i][0];
            task_variable[i][4] = &mas_variable_pool[i][1];
            task_variable[i][5] = &mas_variable_pool[i][2];
            task_variable[i][6] = &mas_variable_pool[i][3];
        }


        for(int i = 0; i < count_thread_in_pool; i++){
            Task ptr_task(&__local__Add_In_Bypass_Paralel, task_variable[i]);
            mas_task[i].Move_To(std::move(ptr_task));

            thread_pool.Sending_To_Processing(&mas_task[i]);
        }
        
        for(int i = 0; i < count_thread_in_pool; i++)
            mas_task[i].Wait_Completed();


        for(int i = 1; i < count_thread_in_pool; i++){
            if(transition_block[mas_variable_pool[i][0] - 1] != 0){
                bool flag = true;
                int k = 0;

                long sum;

                while(flag){
                    flag = false;

                    sum = this -> num[mas_variable_pool[i][1] + k] + transition_block[mas_variable_pool[i][0] - 1 + k];
                    transition_block[mas_variable_pool[i][0] - 1 + k] = 0;
                    
                    if(sum / 1000000000 > 0){
                        flag = true;
                        transition_block[mas_variable_pool[i][0] + k] += sum / 1000000000;
                    }

                    this -> num[mas_variable_pool[i][1] + k] = sum % 1000000000;

                    k++;
                }
            }
        }

        bool flag = true;
        int k = 0;

        long sum;

        while(flag){
            flag = false;

            sum = this -> num[Nu.ptr_head + k + 1] + transition_block[Nu.ptr_head - Nu.plt_tail + k];

            if(sum / 1000000000 > 0){
                flag = true;
                transition_block[Nu.ptr_head + k] += sum / 1000000000;
            }

            this -> num[Nu.ptr_head + k + 1] = sum % 1000000000;

            k++;
        }

        int first_block = std::max(ptr_head - plt_tail, Nu.ptr_head - Nu.plt_tail);

        ptr_head = plt_tail + first_block;
        
        if(num[plt_tail + first_block + 1] != 0)
            ptr_head++;
        
        len_num = 9 * (ptr_head - plt_tail) + Length_Int(num[ptr_head]);
        return;
    }
    
    // разные знаки
    sign_plus = !sign_plus;
    this -> Subtract_In_Bypass_Paralel(Nu);

    if((this -> len_num == 1) && (this -> num[plt_tail] == 0)){
        sign_plus = true;
        return;
    }

    sign_plus = !sign_plus;
    return;
}
void Int_Number::Subtract_In_Bypass_Paralel(const Int_Number& Nu){
    if(len_dedicated <= std::max(ptr_head, Nu.ptr_head) + 1)
        this -> Relocation_Bypass(2 * Nu.ptr_head / len_dedicated + 2);
    
    int conpar = this -> ABS_Comparator_Bypass(Nu);

    // одинаковые знаки this равно по модулю
    if((sign_plus == Nu.sign_plus) && (conpar == 0)){
        for(int i = plt_tail; i <= ptr_head; i++)
            this -> num[i] = 0;
        
        len_num = 1;
        ptr_head = plt_tail;
        sign_plus = true;
        return;
    }
    
    
    // одинаковые знаки this больше по модулю
    if((sign_plus == Nu.sign_plus) && (conpar == 1)){
        int count_thread_in_pool = thread_pool.Count_Worker();

        long max_count_block = std::max(ptr_head - plt_tail + 1, Nu.ptr_head - Nu.plt_tail + 1);
        long Nu_count_block = Nu.ptr_head - Nu.plt_tail + 1;
        std::vector<int> transition_block(max_count_block);
            
        for(int i = Nu_count_block - 1; i < max_count_block; i++)
            transition_block[i] = 0;

        //poz_start_mas_block, poz_start_mas_in, poz_start_mas_out, count_block
        int mas_variable_pool[count_thread_in_pool][4];
        //transition_block, Nu.num, this.num, poz_start_mas_block, poz_start_mas_in, poz_start_mas_out, count_block
        void* task_variable[count_thread_in_pool][7];
        Task mas_task[count_thread_in_pool];

        int Block_to_task = Nu_count_block / count_thread_in_pool;
        int Block_to_task_over = Nu_count_block % count_thread_in_pool;

        for(int i = 0; i < Block_to_task_over; i++){
            mas_variable_pool[i][0] = i * (Block_to_task + 1);
            mas_variable_pool[i][1] = mas_variable_pool[i][0] + plt_tail;
            mas_variable_pool[i][2] = mas_variable_pool[i][0] + Nu.plt_tail;
            mas_variable_pool[i][3] = Block_to_task + 1;
        }
        for(int i = 0; i < count_thread_in_pool - Block_to_task_over; i++){
            mas_variable_pool[i + Block_to_task_over][0] = Block_to_task_over * (Block_to_task + 1) + i * Block_to_task;
            mas_variable_pool[i + Block_to_task_over][1] = mas_variable_pool[i + Block_to_task_over][0] + plt_tail;
            mas_variable_pool[i + Block_to_task_over][2] = mas_variable_pool[i + Block_to_task_over][0] + Nu.plt_tail;
            mas_variable_pool[i + Block_to_task_over][3] = Block_to_task;
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            task_variable[i][0] = transition_block.data();
            task_variable[i][1] = Nu.num;
            task_variable[i][2] = num;

            task_variable[i][3] = &mas_variable_pool[i][0];
            task_variable[i][4] = &mas_variable_pool[i][1];
            task_variable[i][5] = &mas_variable_pool[i][2];
            task_variable[i][6] = &mas_variable_pool[i][3];
        }


        for(int i = 0; i < count_thread_in_pool; i++){
            Task ptr_task(&__local_first__Subtract_In_Bypass_Paralel, task_variable[i]);
            mas_task[i].Move_To(std::move(ptr_task));

            thread_pool.Sending_To_Processing(&mas_task[i]);
        }
        
        for(int i = 0; i < count_thread_in_pool; i++)
            mas_task[i].Wait_Completed();



        for(int i = 1; i < count_thread_in_pool; i++){
            if(transition_block[mas_variable_pool[i][0] - 1] != 0){
                bool flag = true;
                int k = 0;

                long subt;

                while(flag){
                    flag = false;

                    subt = this -> num[mas_variable_pool[i][1] + k] + transition_block[mas_variable_pool[i][0] - 1 + k];
                    transition_block[mas_variable_pool[i][0] - 1 + k] = 0;
                    
                    if(subt < 0){
                        flag = true;
                        transition_block[mas_variable_pool[i][0] + k] += -1;

                        this -> num[mas_variable_pool[i][1] + k] = 1000000000 + subt;
                    }
                    else
                        this -> num[mas_variable_pool[i][1] + k] = subt;

                    k++;
                }
            }
        }

        bool flag = true;
        int k = 0;

        long subt;

        while(flag){
            flag = false;

            subt = this -> num[Nu.ptr_head + k] + transition_block[Nu.ptr_head - Nu.plt_tail + k];
            transition_block[Nu.ptr_head - Nu.plt_tail + k] = 0;
            
            if(subt < 0){
                flag = true;
                transition_block[Nu.ptr_head - Nu.plt_tail + 1 + k] += -1;

                this -> num[Nu.ptr_head + k] = 1000000000 + subt;
            }
            else
                this -> num[Nu.ptr_head + k] = subt;

            k++;
        }

        while((num[ptr_head] == 0) && (ptr_head > plt_tail))
            ptr_head--;
        
        len_num = 9 * (ptr_head - plt_tail) + Length_Int(num[ptr_head]);
        return;
    }


    //число this меньше
    if(sign_plus == Nu.sign_plus){
        sign_plus = !sign_plus;

        int count_thread_in_pool = thread_pool.Count_Worker();

        long max_count_block = std::max(ptr_head - plt_tail + 1, Nu.ptr_head - Nu.plt_tail + 1);
        long Nu_count_block = Nu.ptr_head - Nu.plt_tail + 1;
        std::vector<int> transition_block(max_count_block);
            
        for(int i = Nu_count_block - 1; i < max_count_block; i++)
            transition_block[i] = 0;

        //poz_start_mas_block, poz_start_mas_in, poz_start_mas_out, count_block
        int mas_variable_pool[count_thread_in_pool][4];
        //transition_block, Nu.num, this.num, poz_start_mas_block, poz_start_mas_in, poz_start_mas_out, count_block
        void* task_variable[count_thread_in_pool][7];
        Task mas_task[count_thread_in_pool];

        int Block_to_task = Nu_count_block / count_thread_in_pool;
        int Block_to_task_over = Nu_count_block % count_thread_in_pool;

        for(int i = 0; i < Block_to_task_over; i++){
            mas_variable_pool[i][0] = i * (Block_to_task + 1);
            mas_variable_pool[i][1] = mas_variable_pool[i][0] + plt_tail;
            mas_variable_pool[i][2] = mas_variable_pool[i][0] + Nu.plt_tail;
            mas_variable_pool[i][3] = Block_to_task + 1;
        }
        for(int i = 0; i < count_thread_in_pool - Block_to_task_over; i++){
            mas_variable_pool[i + Block_to_task_over][0] = Block_to_task_over * (Block_to_task + 1) + i * Block_to_task;
            mas_variable_pool[i + Block_to_task_over][1] = mas_variable_pool[i + Block_to_task_over][0] + plt_tail;
            mas_variable_pool[i + Block_to_task_over][2] = mas_variable_pool[i + Block_to_task_over][0] + Nu.plt_tail;
            mas_variable_pool[i + Block_to_task_over][3] = Block_to_task;
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            task_variable[i][0] = transition_block.data();
            task_variable[i][1] = Nu.num;
            task_variable[i][2] = num;

            task_variable[i][3] = &mas_variable_pool[i][0];
            task_variable[i][4] = &mas_variable_pool[i][1];
            task_variable[i][5] = &mas_variable_pool[i][2];
            task_variable[i][6] = &mas_variable_pool[i][3];
        }


        for(int i = 0; i < count_thread_in_pool; i++){
            Task ptr_task(&__local_second__Subtract_In_Bypass_Paralel, task_variable[i]);
            mas_task[i].Move_To(std::move(ptr_task));

            thread_pool.Sending_To_Processing(&mas_task[i]);
        }
        
        for(int i = 0; i < count_thread_in_pool; i++)
            mas_task[i].Wait_Completed();



        for(int i = 1; i < count_thread_in_pool; i++){
            if(transition_block[mas_variable_pool[i][0] - 1] != 0){
                bool flag = true;
                int k = 0;

                long subt;

                while(flag){
                    flag = false;

                    subt = this -> num[mas_variable_pool[i][1] + k] + transition_block[mas_variable_pool[i][0] - 1 + k];
                    transition_block[mas_variable_pool[i][0] - 1 + k] = 0;
                    
                    if(subt < 0){
                        flag = true;
                        transition_block[mas_variable_pool[i][0] + k] += -1;

                        this -> num[mas_variable_pool[i][1] + k] = 1000000000 + subt;
                    }
                    else
                        this -> num[mas_variable_pool[i][1] + k] = subt;

                    k++;
                }
            }
        }

        bool flag = true;
        int k = 0;

        long subt;

        while(flag){
            flag = false;

            subt = this -> num[Nu.ptr_head + k] + transition_block[Nu.ptr_head - Nu.plt_tail + k];
            transition_block[Nu.ptr_head - Nu.plt_tail + k] = 0;
            
            if(subt < 0){
                flag = true;
                transition_block[Nu.ptr_head - Nu.plt_tail + 1 + k] += -1;

                this -> num[Nu.ptr_head + k] = 1000000000 + subt;
            }
            else
                this -> num[Nu.ptr_head + k] = subt;

            k++;
        }

        ptr_head = plt_tail + Nu.ptr_head - Nu.plt_tail;
        while((num[ptr_head] == 0) && (ptr_head > plt_tail))
            ptr_head--;
        
        len_num = 9 * (ptr_head - plt_tail) + Length_Int(num[ptr_head]);
        return;
    }


    // разные знаки
    sign_plus = !sign_plus;
    this -> Add_In_Bypass_Paralel(Nu);

    if((this -> len_num == 1) && (this -> num[plt_tail] == 0)){
        sign_plus = true;
        return;
    }

    sign_plus = !sign_plus;
    return;
}


Int_Number& Int_Number::Shift_Right_Bypass(int i) {
    if(i == 0)
        return *this;

    if(i < 0)
        return this -> Shift_Left_Bypass(-i);
    
    if(this -> Comparator_Bypass(0) == 0)
        return *this;
    
    if(plt_tail <= i)
        this -> Relocation_Bypass(2 * i / len_dedicated + 2);
    
    plt_tail -= i;
    len_num += 9*i;

    return *this;
}
Int_Number& Int_Number::Shift_Left_Bypass(int i) {
    if(i == 0)
        return *this;

    if(i < 0)
        return this -> Shift_Right_Bypass(-i);
    
    while ((i > 0) && (plt_tail < ptr_head)){
        num[plt_tail++] = 0;
        i--;
        len_num -= 9;
    }

    if(i > 0){
        num[plt_tail] = 0;
        len_num = 1;
    }
    return *this;
}


void Int_Number::Multiplication_Bypass(int q) {
    if(q == 0){
        this -> Shift_Left_Bypass((this ->  len_num)/9 + 1);
        return;
    }

    if(q == 1)
        return;
    
    if(q == -1){
        sign_plus = !sign_plus;
        return;
    }

    if(len_dedicated <= ptr_head + 2)
        this -> Relocation_Bypass(2);

    if(q < 0)
        sign_plus = !sign_plus;
    q = abs(q);

    long carry = 0;
    int i = plt_tail;

    while (i <= ptr_head) {
        long long mul = (long long)num[i] * (long long)q + carry;
        num[i] = mul % 1000000000;
        carry = mul / 1000000000;
        i++;
    }

    ptr_head = i-1;
    if (carry > 0) {
        num[i] = carry;
        ptr_head = i;
    }

    len_num = 9 * (ptr_head - plt_tail) + Length_Int(num[ptr_head]);
    return;
}
void Int_Number::Multiplication_Bypass(const Int_Number& Nu) {
    if(len_num + Nu.len_num + 1 > len_dedicated)
        this -> Relocation_Bypass(Nu.len_num/len_num + 2);

    Int_Number Nu_ptr = this -> Copy_Bypass();
    this -> Multiplication_Bypass(Nu.num[Nu.plt_tail]);

    for(int i = Nu.plt_tail + 1; i <= Nu.ptr_head; i++)
        this -> Only_Add_With_Shift_Bypass(Nu_ptr * Nu.num[i], i - Nu.plt_tail);

    sign_plus = sign_plus == Nu.sign_plus;
}
//В НЕЕ НЕЛЬЗЯ ПИХАТЬ ОДИН И ТОТ ЖЕ ОБЬЕКТ(ЧИСЛО)


Int_Number Int_Number::Multiplication_NTT_Outward_Bypass(const Int_Number& Nu) const{
    int count_modile = 3;

    int power_2 = 6;

    while((1 << power_2) < (len_num/9 + Nu.len_num/9 + 2))
        power_2++;

    if((1 << power_2) > 470000000)
        count_modile = 4;
    


    if(count_modile == 3){
        std::vector<int> Nu1_MOD1((1 << power_2), 0);
        std::vector<int> Nu1_MOD2((1 << power_2), 0);
        std::vector<int> Nu1_MOD3((1 << power_2), 0);

        std::vector<int> Nu2_MOD1((1 << power_2), 0);
        std::vector<int> Nu2_MOD2((1 << power_2), 0);
        std::vector<int> Nu2_MOD3((1 << power_2), 0);


        for(int i = 0; i <= ptr_head - plt_tail; i++){
            Nu1_MOD1[i] = num[i + plt_tail];
            Nu1_MOD2[i] = num[i + plt_tail];
            Nu1_MOD3[i] = num[i + plt_tail];
        }

        for(int i = 0; i <= Nu.ptr_head - Nu.plt_tail; i++){
            Nu2_MOD1[i] = Nu.num[i + Nu.plt_tail];
            Nu2_MOD2[i] = Nu.num[i + Nu.plt_tail];
            Nu2_MOD3[i] = Nu.num[i + Nu.plt_tail];
        }

        std::vector<std::thread> threads;

        
        threads.emplace_back(PerformNTTMultiplication<Mod998244353>, power_2, std::ref(Nu1_MOD1), std::ref(Nu2_MOD1));
        threads.emplace_back(PerformNTTMultiplication<Mod1004535809>, power_2, std::ref(Nu1_MOD2), std::ref(Nu2_MOD2));
        threads.emplace_back(PerformNTTMultiplication<Mod469762049>, power_2, std::ref(Nu1_MOD3), std::ref(Nu2_MOD3));

        for(int i = 0; i < threads.size(); i++)
            threads[i].join();
        
        
        Int_Number P1(Mod998244353::MOD);
        Int_Number P2(Mod1004535809::MOD);
        Int_Number P3(Mod469762049::MOD);
        Int_Number M12 = P1 * P2;
        Int_Number M123 = M12 * P3;
        
        long long int inv_P1_mod_P2 = Mod_Num_<Mod998244353>::modpow(Mod998244353::MOD % Mod1004535809::MOD, Mod1004535809::MOD - 2, Mod1004535809::MOD);
        long long int inv_M12_mod_P3 = Mod_Num_<Mod998244353>::modpow((1LL * Mod998244353::MOD * Mod1004535809::MOD) % Mod469762049::MOD, Mod469762049::MOD - 2, Mod469762049::MOD);
        long long int inv_P1_mod_P3 = Mod_Num_<Mod998244353>::modpow(Mod998244353::MOD % Mod469762049::MOD, Mod469762049::MOD - 2, Mod469762049::MOD);
        

        Int_Number FinalResult(ptr_head + Nu.ptr_head + (plt_tail + Nu.plt_tail)/20 - (plt_tail + Nu.plt_tail) + 3, (plt_tail + Nu.plt_tail)/20);
        FinalResult.sign_plus = this -> sign_plus == Nu.sign_plus;



        int count_thread_in_pool = thread_pool.Count_Worker();

        void* mas_task_data[count_thread_in_pool][14];
        long long int mas_start_count[count_thread_in_pool][2];
        Int_Number mas_int_number[count_thread_in_pool];
        Task mas_task[count_thread_in_pool];

        int Block_to_task = (1 << power_2) / count_thread_in_pool;
        int Block_to_task_over = (1 << power_2) % count_thread_in_pool;

        for(int i = 0; i < Block_to_task_over; i++){
            mas_start_count[i][0] = i * (Block_to_task + 1);
            mas_start_count[i][1] = Block_to_task + 1;
        }
        for(int i = 0; i < count_thread_in_pool - Block_to_task_over; i++){
            mas_start_count[i + Block_to_task_over][0] = i * Block_to_task + Block_to_task_over * (Block_to_task + 1);
            mas_start_count[i + Block_to_task_over][1] = Block_to_task;
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task_data[i][0] = &Nu1_MOD1;
            mas_task_data[i][1] = &Nu1_MOD2;
            mas_task_data[i][2] = &Nu1_MOD3;

            mas_task_data[i][3] = &mas_start_count[i][0];
            mas_task_data[i][4] = &mas_start_count[i][1];

            mas_task_data[i][5] = &P1;
            mas_task_data[i][6] = &P2;
            mas_task_data[i][7] = &P3;
            mas_task_data[i][8] = &M12;
            mas_task_data[i][9] = &M123;

            mas_task_data[i][10] = &inv_P1_mod_P2;
            mas_task_data[i][11] = &inv_M12_mod_P3;
            mas_task_data[i][12] = &inv_P1_mod_P3;

            mas_task_data[i][13] = &mas_int_number[i];
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task[i].Move_To(Task(&__local_CRT__3_Outward_Bypass, mas_task_data[i]));
            thread_pool.Sending_To_Processing(&mas_task[i]);
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task[i].Wait_Completed();
            
            FinalResult.Only_Add_With_Shift_Bypass(mas_int_number[i], mas_start_count[i][0]);
        }
        
        return std::move(FinalResult);
    }

    if(count_modile == 4){
        std::vector<int> Nu1_MOD1((1 << power_2), 0);
        std::vector<int> Nu1_MOD2((1 << power_2), 0);
        std::vector<int> Nu1_MOD3((1 << power_2), 0);
        std::vector<int> Nu1_MOD4((1 << power_2), 0);

        std::vector<int> Nu2_MOD1((1 << power_2), 0);
        std::vector<int> Nu2_MOD2((1 << power_2), 0);
        std::vector<int> Nu2_MOD3((1 << power_2), 0);
        std::vector<int> Nu2_MOD4((1 << power_2), 0);


        for(int i = 0; i <= ptr_head - plt_tail; i++){
            Nu1_MOD1[i] = num[i + plt_tail];
            Nu1_MOD2[i] = num[i + plt_tail];
            Nu1_MOD3[i] = num[i + plt_tail];
            Nu1_MOD4[i] = num[i + plt_tail];
        }

        for(int i = 0; i <= Nu.ptr_head - Nu.plt_tail; i++){
            Nu2_MOD1[i] = Nu.num[i + Nu.plt_tail];
            Nu2_MOD2[i] = Nu.num[i + Nu.plt_tail];
            Nu2_MOD3[i] = Nu.num[i + Nu.plt_tail];
            Nu2_MOD4[i] = Nu.num[i + Nu.plt_tail];
        }

        std::vector<std::thread> threads;

        threads.emplace_back(PerformNTTMultiplication<Mod998244353>, power_2, std::ref(Nu1_MOD1), std::ref(Nu2_MOD1));
        threads.emplace_back(PerformNTTMultiplication<Mod1004535809>, power_2, std::ref(Nu1_MOD2), std::ref(Nu2_MOD2));
        threads.emplace_back(PerformNTTMultiplication<Mod469762049>, power_2, std::ref(Nu1_MOD3), std::ref(Nu2_MOD3));
        threads.emplace_back(PerformNTTMultiplication<Mod167772161>, power_2, std::ref(Nu1_MOD4), std::ref(Nu2_MOD4));


        for(int i = 0; i < threads.size(); i++)
            threads[i].join();
        
        Int_Number P1(Mod998244353::MOD);
        Int_Number P2(Mod1004535809::MOD);
        Int_Number P3(Mod469762049::MOD);
        Int_Number P4(Mod167772161::MOD);
        Int_Number M12 = P1 * P2;
        Int_Number M123 = M12 * P3;
        Int_Number M1234 = M123 * P4;
        
        long long int inv_P1_mod_P2 = Mod_Num_<Mod998244353>::modpow(Mod998244353::MOD % Mod1004535809::MOD, Mod1004535809::MOD - 2, Mod1004535809::MOD);
        long long int inv_M12_mod_P3 = Mod_Num_<Mod998244353>::modpow((1LL * Mod998244353::MOD * Mod1004535809::MOD) % Mod469762049::MOD, Mod469762049::MOD - 2, Mod469762049::MOD);
        long long int inv_M123_mod_P4 = Mod_Num_<Mod998244353>::modpow(130839182, Mod167772161::MOD - 2, Mod167772161::MOD);
        //130839182 посчитано руками (переполнение)

        
        Int_Number FinalResult(ptr_head + Nu.ptr_head + (plt_tail + Nu.plt_tail)/20 - (plt_tail + Nu.plt_tail) + 3, (plt_tail + Nu.plt_tail)/20);
        FinalResult.sign_plus = this -> sign_plus == Nu.sign_plus;

        
        int count_thread_in_pool = thread_pool.Count_Worker();

        void* mas_task_data[count_thread_in_pool][17];
        long long int mas_start_count[count_thread_in_pool][2];
        Int_Number mas_int_number[count_thread_in_pool];
        Task mas_task[count_thread_in_pool];

        int Block_to_task = (1 << power_2) / count_thread_in_pool;
        int Block_to_task_over = (1 << power_2) % count_thread_in_pool;

        for(int i = 0; i < Block_to_task_over; i++){
            mas_start_count[i][0] = i * (Block_to_task + 1);
            mas_start_count[i][1] = Block_to_task + 1;
        }
        for(int i = 0; i < count_thread_in_pool - Block_to_task_over; i++){
            mas_start_count[i][0] = i * Block_to_task + Block_to_task_over * (Block_to_task + 1);
            mas_start_count[i][1] = Block_to_task;
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task_data[i][0] = &Nu1_MOD1;
            mas_task_data[i][1] = &Nu1_MOD2;
            mas_task_data[i][2] = &Nu1_MOD3;
            mas_task_data[i][3] = &Nu1_MOD4;

            mas_task_data[i][4] = &mas_start_count[i][0];
            mas_task_data[i][5] = &mas_start_count[i][1];

            mas_task_data[i][6] = &P1;
            mas_task_data[i][7] = &P2;
            mas_task_data[i][8] = &P3;
            mas_task_data[i][9] = &P4;
            mas_task_data[i][10] = &M12;
            mas_task_data[i][11] = &M123;
            mas_task_data[i][12] = &M1234;

            mas_task_data[i][13] = &inv_P1_mod_P2;
            mas_task_data[i][14] = &inv_M12_mod_P3;
            mas_task_data[i][15] = &inv_M123_mod_P4;

            mas_task_data[i][16] = &mas_int_number[i];
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task[i].Move_To(Task(&__local_CRT__4_Outward_Bypass, mas_task_data[i]));
            thread_pool.Sending_To_Processing(&mas_task[i]);
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task[i].Wait_Completed();
            
            FinalResult.Only_Add_With_Shift_Bypass(mas_int_number[i], mas_start_count[i][0]);
        }
        
        return std::move(FinalResult);
    }

return Int_Number(0);
}
Int_Number Int_Number::Multiplication_NTT_Outward_Thread_Pool_Bypass(const Int_Number& Nu) const{
    int count_modile = 3;

    int power_2 = 6;

    while((1 << power_2) < (len_num/9 + Nu.len_num/9 + 2))
        power_2++;

    if((1 << power_2) > 470000000)
        count_modile = 4;
    


    if(count_modile == 3){
        std::vector<int> Nu1_MOD1((1 << power_2), 0);
        std::vector<int> Nu1_MOD2((1 << power_2), 0);
        std::vector<int> Nu1_MOD3((1 << power_2), 0);

        std::vector<int> Nu2_MOD1((1 << power_2), 0);
        std::vector<int> Nu2_MOD2((1 << power_2), 0);
        std::vector<int> Nu2_MOD3((1 << power_2), 0);


        for(int i = 0; i <= ptr_head - plt_tail; i++){
            Nu1_MOD1[i] = num[i + plt_tail];
            Nu1_MOD2[i] = num[i + plt_tail];
            Nu1_MOD3[i] = num[i + plt_tail];
        }

        for(int i = 0; i <= Nu.ptr_head - Nu.plt_tail; i++){
            Nu2_MOD1[i] = Nu.num[i + Nu.plt_tail];
            Nu2_MOD2[i] = Nu.num[i + Nu.plt_tail];
            Nu2_MOD3[i] = Nu.num[i + Nu.plt_tail];
        }

        void* mas_1[3];
        void* mas_2[3];
        void* mas_3[3];

        mas_1[0] = &power_2;
        mas_2[0] = &power_2;
        mas_3[0] = &power_2;

        mas_1[1] = &Nu1_MOD1;
        mas_2[1] = &Nu1_MOD2;
        mas_3[1] = &Nu1_MOD3;

        mas_1[2] = &Nu2_MOD1;
        mas_2[2] = &Nu2_MOD2;
        mas_3[2] = &Nu2_MOD3;

        Task task1(&__local_Mod__NTT_Outward_Bypass_Paralel<Mod998244353>, mas_1);
        Task task2(&__local_Mod__NTT_Outward_Bypass_Paralel<Mod1004535809>, mas_2);
        Task task3(&__local_Mod__NTT_Outward_Bypass_Paralel<Mod469762049>, mas_3);


        thread_pool.Sending_To_Processing(&task1);
        thread_pool.Sending_To_Processing(&task2);
        thread_pool.Sending_To_Processing(&task3);

        task1.Wait_Completed();
        task2.Wait_Completed();
        task3.Wait_Completed();

        
        
        Int_Number P1(Mod998244353::MOD);
        Int_Number P2(Mod1004535809::MOD);
        Int_Number P3(Mod469762049::MOD);
        Int_Number M12 = P1 * P2;
        Int_Number M123 = M12 * P3;
        
        long long int inv_P1_mod_P2 = Mod_Num_<Mod998244353>::modpow(Mod998244353::MOD % Mod1004535809::MOD, Mod1004535809::MOD - 2, Mod1004535809::MOD);
        long long int inv_M12_mod_P3 = Mod_Num_<Mod998244353>::modpow((1LL * Mod998244353::MOD * Mod1004535809::MOD) % Mod469762049::MOD, Mod469762049::MOD - 2, Mod469762049::MOD);
        long long int inv_P1_mod_P3 = Mod_Num_<Mod998244353>::modpow(Mod998244353::MOD % Mod469762049::MOD, Mod469762049::MOD - 2, Mod469762049::MOD);
        

        Int_Number FinalResult(ptr_head + Nu.ptr_head + (plt_tail + Nu.plt_tail)/20 - (plt_tail + Nu.plt_tail) + 3, (plt_tail + Nu.plt_tail)/20);
        FinalResult.sign_plus = this -> sign_plus == Nu.sign_plus;



        int count_thread_in_pool = thread_pool.Count_Worker();

        void* mas_task_data[count_thread_in_pool][14];
        long long int mas_start_count[count_thread_in_pool][2];
        Int_Number mas_int_number[count_thread_in_pool];
        Task mas_task[count_thread_in_pool];

        int Block_to_task = (1 << power_2) / count_thread_in_pool;
        int Block_to_task_over = (1 << power_2) % count_thread_in_pool;

        for(int i = 0; i < Block_to_task_over; i++){
            mas_start_count[i][0] = i * (Block_to_task + 1);
            mas_start_count[i][1] = Block_to_task + 1;
        }
        for(int i = 0; i < count_thread_in_pool - Block_to_task_over; i++){
            mas_start_count[i + Block_to_task_over][0] = i * Block_to_task + Block_to_task_over * (Block_to_task + 1);
            mas_start_count[i + Block_to_task_over][1] = Block_to_task;
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task_data[i][0] = &Nu1_MOD1;
            mas_task_data[i][1] = &Nu1_MOD2;
            mas_task_data[i][2] = &Nu1_MOD3;

            mas_task_data[i][3] = &mas_start_count[i][0];
            mas_task_data[i][4] = &mas_start_count[i][1];

            mas_task_data[i][5] = &P1;
            mas_task_data[i][6] = &P2;
            mas_task_data[i][7] = &P3;
            mas_task_data[i][8] = &M12;
            mas_task_data[i][9] = &M123;

            mas_task_data[i][10] = &inv_P1_mod_P2;
            mas_task_data[i][11] = &inv_M12_mod_P3;
            mas_task_data[i][12] = &inv_P1_mod_P3;

            mas_task_data[i][13] = &mas_int_number[i];
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task[i].Move_To(Task(&__local_CRT__3_Outward_Bypass, mas_task_data[i]));
            thread_pool.Sending_To_Processing(&mas_task[i]);
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task[i].Wait_Completed();
            
            FinalResult.Only_Add_With_Shift_Bypass(mas_int_number[i], mas_start_count[i][0]);
        }
        
        return std::move(FinalResult);
    }

    if(count_modile == 4){
        std::vector<int> Nu1_MOD1((1 << power_2), 0);
        std::vector<int> Nu1_MOD2((1 << power_2), 0);
        std::vector<int> Nu1_MOD3((1 << power_2), 0);
        std::vector<int> Nu1_MOD4((1 << power_2), 0);

        std::vector<int> Nu2_MOD1((1 << power_2), 0);
        std::vector<int> Nu2_MOD2((1 << power_2), 0);
        std::vector<int> Nu2_MOD3((1 << power_2), 0);
        std::vector<int> Nu2_MOD4((1 << power_2), 0);


        for(int i = 0; i <= ptr_head - plt_tail; i++){
            Nu1_MOD1[i] = num[i + plt_tail];
            Nu1_MOD2[i] = num[i + plt_tail];
            Nu1_MOD3[i] = num[i + plt_tail];
            Nu1_MOD4[i] = num[i + plt_tail];
        }

        for(int i = 0; i <= Nu.ptr_head - Nu.plt_tail; i++){
            Nu2_MOD1[i] = Nu.num[i + Nu.plt_tail];
            Nu2_MOD2[i] = Nu.num[i + Nu.plt_tail];
            Nu2_MOD3[i] = Nu.num[i + Nu.plt_tail];
            Nu2_MOD4[i] = Nu.num[i + Nu.plt_tail];
        }
        void* mas_1[3];
        void* mas_2[3];
        void* mas_3[3];
        void* mas_4[3];

        mas_1[0] = &power_2;
        mas_2[0] = &power_2;
        mas_3[0] = &power_2;
        mas_4[0] = &power_2;

        mas_1[1] = &Nu1_MOD1;
        mas_2[1] = &Nu1_MOD2;
        mas_3[1] = &Nu1_MOD3;
        mas_4[1] = &Nu1_MOD4;

        mas_1[2] = &Nu2_MOD1;
        mas_2[2] = &Nu2_MOD2;
        mas_3[2] = &Nu2_MOD3;
        mas_4[2] = &Nu2_MOD4;

        Task task1(&__local_Mod__NTT_Outward_Bypass_Paralel<Mod998244353>, mas_1);
        Task task2(&__local_Mod__NTT_Outward_Bypass_Paralel<Mod1004535809>, mas_2);
        Task task3(&__local_Mod__NTT_Outward_Bypass_Paralel<Mod469762049>, mas_3);
        Task task4(&__local_Mod__NTT_Outward_Bypass_Paralel<Mod167772161>, mas_4);


        thread_pool.Sending_To_Processing(&task1);
        thread_pool.Sending_To_Processing(&task2);
        thread_pool.Sending_To_Processing(&task3);
        thread_pool.Sending_To_Processing(&task4);

        task1.Wait_Completed();
        task2.Wait_Completed();
        task3.Wait_Completed();
        task4.Wait_Completed(); 

        
        Int_Number P1(Mod998244353::MOD);
        Int_Number P2(Mod1004535809::MOD);
        Int_Number P3(Mod469762049::MOD);
        Int_Number P4(Mod167772161::MOD);
        Int_Number M12 = P1 * P2;
        Int_Number M123 = M12 * P3;
        Int_Number M1234 = M123 * P4;
        
        long long int inv_P1_mod_P2 = Mod_Num_<Mod998244353>::modpow(Mod998244353::MOD % Mod1004535809::MOD, Mod1004535809::MOD - 2, Mod1004535809::MOD);
        long long int inv_M12_mod_P3 = Mod_Num_<Mod998244353>::modpow((1LL * Mod998244353::MOD * Mod1004535809::MOD) % Mod469762049::MOD, Mod469762049::MOD - 2, Mod469762049::MOD);
        long long int inv_M123_mod_P4 = Mod_Num_<Mod998244353>::modpow(130839182, Mod167772161::MOD - 2, Mod167772161::MOD);
        //130839182 посчитано руками (переполнение)

        
        Int_Number FinalResult(ptr_head + Nu.ptr_head + (plt_tail + Nu.plt_tail)/20 - (plt_tail + Nu.plt_tail) + 3, (plt_tail + Nu.plt_tail)/20);
        FinalResult.sign_plus = this -> sign_plus == Nu.sign_plus;

        
        int count_thread_in_pool = thread_pool.Count_Worker();

        void* mas_task_data[count_thread_in_pool][17];
        long long int mas_start_count[count_thread_in_pool][2];
        Int_Number mas_int_number[count_thread_in_pool];
        Task mas_task[count_thread_in_pool];

        int Block_to_task = (1 << power_2) / count_thread_in_pool;
        int Block_to_task_over = (1 << power_2) % count_thread_in_pool;

        for(int i = 0; i < Block_to_task_over; i++){
            mas_start_count[i][0] = i * (Block_to_task + 1);
            mas_start_count[i][1] = Block_to_task + 1;
        }
        for(int i = 0; i < count_thread_in_pool - Block_to_task_over; i++){
            mas_start_count[i + Block_to_task_over][0] = i * Block_to_task + Block_to_task_over * (Block_to_task + 1);
            mas_start_count[i + Block_to_task_over][1] = Block_to_task;
        }



        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task_data[i][0] = &Nu1_MOD1;
            mas_task_data[i][1] = &Nu1_MOD2;
            mas_task_data[i][2] = &Nu1_MOD3;
            mas_task_data[i][3] = &Nu1_MOD4;

            mas_task_data[i][4] = &mas_start_count[i][0];
            mas_task_data[i][5] = &mas_start_count[i][1];

            mas_task_data[i][6] = &P1;
            mas_task_data[i][7] = &P2;
            mas_task_data[i][8] = &P3;
            mas_task_data[i][9] = &P4;
            mas_task_data[i][10] = &M12;
            mas_task_data[i][11] = &M123;
            mas_task_data[i][12] = &M1234;

            mas_task_data[i][13] = &inv_P1_mod_P2;
            mas_task_data[i][14] = &inv_M12_mod_P3;
            mas_task_data[i][15] = &inv_M123_mod_P4;

            mas_task_data[i][16] = &mas_int_number[i];
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task[i].Move_To(Task(&__local_CRT__4_Outward_Bypass, mas_task_data[i]));
            thread_pool.Sending_To_Processing(&mas_task[i]);
        }

        for(int i = 0; i < count_thread_in_pool; i++){
            mas_task[i].Wait_Completed();
            
            FinalResult.Only_Add_With_Shift_Bypass(mas_int_number[i], mas_start_count[i][0]);
        }
        
        return std::move(FinalResult);
    }

return Int_Number(0);
}




Int_Number::Int_Number() {
    num = new int[1];
    num[0] = 0;
    len_dedicated = 1;
    ptr_head = 0;
    plt_tail = 0;
    len_num = 1;
    sign_plus = true;
}
Int_Number::Int_Number(int len_dedicated1, int plt_tail1) {
    len_dedicated = len_dedicated1;
    ptr_head = plt_tail1;
    plt_tail = plt_tail1;

    num = new int[len_dedicated];
    for(long i = 0; i < len_dedicated; i++)
        num[i] = 0;

    len_num = 1;
    sign_plus = true;
}
Int_Number::Int_Number(int n): Int_Number(3, 1) {
    sign_plus = n >= 0;
    n = sign_plus ? n : -n;
    num[ptr_head] = n;
    len_num = Length_Int(n);
}
Int_Number::Int_Number(long long int n): Int_Number(6, 2) {
    sign_plus = n >= 0;
    n = sign_plus ? n : -n;

    len_num = 0;
    while (n > 0){
        num[ptr_head++] = n % 1000000000;
        n = n / 1000000000;
        
        if(n > 0)
            len_num += 9;
        else
            ptr_head--;
    }

    len_num += Length_Int(num[ptr_head]);
}
Int_Number::Int_Number(Int_Number&& Nu) {
    std::lock_guard<std::mutex> lock(Nu.guard);
    num = Nu.num;
    ptr_head = Nu.ptr_head;
    plt_tail = Nu.plt_tail;
    len_dedicated = Nu.len_dedicated;
    len_num = Nu.len_num;
    sign_plus = Nu.sign_plus;

    Nu.num = nullptr;
    Nu.ptr_head = 0;
    Nu.plt_tail = 0;
    Nu.len_dedicated = 0;
    Nu.len_num = 1;
    Nu.sign_plus = true;
}
Int_Number::Int_Number(const Int_Number& Nu){
    std::lock_guard<std::mutex> lock(Nu.guard);
    len_dedicated = Nu.len_dedicated;
    ptr_head = Nu.ptr_head;
    plt_tail = Nu.plt_tail;
    len_num = Nu.len_num;
    sign_plus = Nu.sign_plus;

    num = new int[len_dedicated];

    for(int i = plt_tail; i <= ptr_head; i++)
        num[i] = Nu.num[i];
}
Int_Number::Int_Number(const std::string& line_num): Int_Number(){
    bool plus_flag = true;
    if(line_num[0] == '-')
        plus_flag = false;


    int mas[line_num.length()/9 + 2];
    int poz = 0;

    int q = 0;
    int shift = 1;
    for(int i = line_num.length() - 1; i > -1; i--){
        if(shift > 100000001){
            mas[poz++] = q;
            q = 0;
            shift = 1;
        }

        int coun = int(line_num[i]) - 48;

        if((coun >= 0) && (coun <= 9)){
            q += coun * shift;
            shift *= 10;
        }
    }

    mas[poz++] = q;


    for(int i = 0; i < poz; i++)
        this -> Add_Bypass(Int_Number(mas[i]) << i);

    if(this -> Comparator_Bypass(0) != 0)
        this -> sign_plus = plus_flag;
}




void Int_Number::All_Print(int count) const {
    std::lock_guard<std::mutex> lock(guard);

    long i = len_dedicated - 1;
    int flag = count;

    if(sign_plus)
        std::cout << "+ ";
    else    
        std::cout << "- ";
    
    while (i > ptr_head){
        int q = num[i];
        int a1 = num[i] / 1000000;
        int a2 = num[i] % 1000000 / 1000;
        int a3 = num[i] % 1000;

        std::cout << FG_YELLOW;

        if (Length_Int(a1) == 1)
            std::cout << "00" << a1 << "_";
        else if (Length_Int(a1) == 2)
            std::cout << "0" << a1 << "_";
        else
            std::cout << a1 << "_";
        
        if (Length_Int(a2) == 1)
            std::cout << "00" << a2 << "_";
        else if (Length_Int(a2) == 2)
            std::cout << "0" << a2 << "_";
        else
            std::cout << a2 << "_";

        if (Length_Int(a3) == 1)
            std::cout << "00" << a3 << "  ";
        else if (Length_Int(a3) == 2)
            std::cout << "0" << a3 << "  ";
        else
            std::cout << a3 << "  ";
        
        flag--;
        i--;

        if((flag == 0) && (i > -1)){
            flag = count;
            std::cout << std::endl << "  ";
        }
    }

    std::cout << RESET;

    while (i >= plt_tail){
        int q = num[i];
        int a1 = num[i] / 1000000;
        int a2 = num[i] % 1000000 / 1000;
        int a3 = num[i] % 1000;

        std::cout << FG_GREEN;

        if (Length_Int(a1) == 1)
            std::cout << "00" << a1 << "_";
        else if (Length_Int(a1) == 2)
            std::cout << "0" << a1 << "_";
        else
            std::cout << a1 << "_";
        
        if (Length_Int(a2) == 1)
            std::cout << "00" << a2 << "_";
        else if (Length_Int(a2) == 2)
            std::cout << "0" << a2 << "_";
        else
            std::cout << a2 << "_";

        if (Length_Int(a3) == 1)
            std::cout << "00" << a3 << "  ";
        else if (Length_Int(a3) == 2)
            std::cout << "0" << a3 << "  ";
        else
            std::cout << a3 << "  ";
        
        flag--;
        i--;

        if((flag == 0) && (i > -1)){
            flag = count;
            std::cout << std::endl << "  ";
        }
    }

    std::cout << RESET;

    while (i > -1){
        int q = num[i];
        int a1 = num[i] / 1000000;
        int a2 = num[i] % 1000000 / 1000;
        int a3 = num[i] % 1000;

        std::cout << FG_YELLOW;

        if (Length_Int(a1) == 1)
            std::cout << "00" << a1 << "_";
        else if (Length_Int(a1) == 2)
            std::cout << "0" << a1 << "_";
        else
            std::cout << a1 << "_";
        
        if (Length_Int(a2) == 1)
            std::cout << "00" << a2 << "_";
        else if (Length_Int(a2) == 2)
            std::cout << "0" << a2 << "_";
        else
            std::cout << a2 << "_";

        if (Length_Int(a3) == 1)
            std::cout << "00" << a3 << "  ";
        else if (Length_Int(a3) == 2)
            std::cout << "0" << a3 << "  ";
        else
            std::cout << a3 << "  ";
        
        flag--;
        i--;

        if((flag == 0) && (i > -1)){
            flag = count;
            std::cout << std::endl << "  ";
        }
    }

    std::cout << RESET;    

    std::cout << std::endl << std::endl;
    std::cout << "ptr_head = " << ptr_head << std::endl;
    std::cout << "plt_tail = " << plt_tail << std::endl;
    std::cout << "len_dedicated = " << len_dedicated << std::endl;
    std::cout << "len_num = " << len_num << std::endl << std::endl;
}
void Int_Number::Technical_Print() const {
    std::lock_guard<std::mutex> lock(guard);

    std::cout << std::endl << std::endl;
    std::cout << "ptr_head = " << ptr_head << std::endl;
    std::cout << "plt_tail = " << plt_tail << std::endl;
    std::cout << "len_dedicated = " << len_dedicated << std::endl;
    std::cout << "len_num = " << len_num << std::endl << std::endl;
}

void Int_Number::Relocation(int count) {
    std::lock_guard<std::mutex> lock(guard);

    if (count == 1) return;

    int len_dedicated_new = len_dedicated * count + 1;
    int* num_new = new int[len_dedicated_new];

    int plt_tail_new =  len_dedicated_new/2 - (ptr_head - plt_tail)/2;
    int ptr_head_new = plt_tail_new + (ptr_head - plt_tail);

    int i = 0;
    int j = plt_tail;
    while(i < len_dedicated_new){
        if(i < plt_tail_new)
            num_new[i++] = 0;
        else if(i <= ptr_head_new)
            num_new[i++] = num[j++];
        else
            num_new[i++] = 0;
    }

    int* num_old = num;
    num = num_new;

    len_dedicated = len_dedicated_new;
    ptr_head = ptr_head_new;
    plt_tail = plt_tail_new;

    delete[] num_old;
}

bool Int_Number::Is_plus() const{
    std::lock_guard<std::mutex> lock(guard);

    return sign_plus;
}

int Int_Number::Length_Number() const{
    std::lock_guard<std::mutex> lock(guard);

    return len_num;
}

int Int_Number::Last_Element_Get() const{
    std::lock_guard<std::mutex> lock(guard);

    return num[plt_tail];
}

Int_Number& Abs(Int_Number& Nu) {
    std::lock_guard<std::mutex> lock(Nu.guard);

    Nu.sign_plus = true; 
    return Nu;
}


std::string Int_Number::To_String() const{
    std::lock_guard<std::mutex> lock(guard); 

    std::string str;

    long i = ptr_head;

    if(sign_plus)
        str += "+ ";
    else    
        str +=  "- ";
    
    std::string first_num = std::to_string(num[i--]);
    int len_first_num = first_num.length();

    if(len_first_num > 3)
        first_num.insert(len_first_num - 3, "_");
    if(len_first_num > 6)
        first_num.insert(len_first_num - 6, "_");

    str += first_num + "  ";
    
    while (i >= plt_tail){
        int q = num[i];
        int a1 = num[i] / 1000000;
        int a2 = num[i] % 1000000 / 1000;
        int a3 = num[i] % 1000;

        if (Length_Int(a1) == 1)
            str += "00" + std::to_string(a1) + "_";
        else if (Length_Int(a1) == 2)
            str += "0" + std::to_string(a1) + "_";
        else
            str += std::to_string(a1) + "_";
        
        if (Length_Int(a2) == 1)
            str += "00" + std::to_string(a2) + "_";
        else if (Length_Int(a2) == 2)
            str += "0" + std::to_string(a2) + "_";
        else
            str += std::to_string(a2) + "_";

        if (Length_Int(a3) == 1)
            str += "00" + std::to_string(a3) + "  ";
        else if (Length_Int(a3) == 2)
            str += "0" + std::to_string(a3) + "  ";
        else
            str += std::to_string(a3) + "  ";
        
        i--;
    }
    while(str[str.length() - 1] == ' ')
        str.pop_back();
    return std::move(str);
}
std::string Int_Number::To_Clear_String() const{
    std::lock_guard<std::mutex> lock(guard); 

    std::string str;

    long i = ptr_head;

    if(sign_plus)
        str += "+";
    else    
        str +=  "-";


    str += std::to_string(num[i--]);

    while (i >= plt_tail){
        int len = Length_Int(num[i]);
        std::string s2(9 - len, '0');
        str += s2 + std::to_string(num[i--]);
    }

    

    return std::move(str);
}



void Int_Number::operator=(Int_Number&& Nu) {
    if(this == &Nu)
        return;
    
    std::scoped_lock lock(guard, Nu.guard);

    if(this -> num == Nu.num)
        return;
    
    delete[] this -> num;

    num = Nu.num;
    ptr_head = Nu.ptr_head;
    plt_tail = Nu.plt_tail;
    len_dedicated = Nu.len_dedicated;
    len_num = Nu.len_num;
    sign_plus = Nu.sign_plus;

    Nu.num = nullptr;
    Nu.ptr_head = 0;
    Nu.plt_tail = 0;
    Nu.len_dedicated = 0;
    Nu.len_num = 1;
    Nu.sign_plus = true;
}
void Int_Number::operator=(const Int_Number& Nu) {
    if(this == &Nu)
        return;
    
    std::scoped_lock lock(guard, Nu.guard);

    Int_Number Nu_j(std::move(Nu.Copy_Bypass()));

    delete[] this -> num;

    num = Nu_j.num;
    ptr_head = Nu_j.ptr_head;
    plt_tail = Nu_j.plt_tail;
    len_dedicated = Nu_j.len_dedicated;
    len_num = Nu_j.len_num;
    sign_plus = Nu_j.sign_plus;

    Nu_j.num = nullptr;
    Nu_j.ptr_head = 0;
    Nu_j.plt_tail = 0;
    Nu_j.len_dedicated = 0;
    Nu_j.len_num = 1;
    Nu_j.sign_plus = true;
}


int Int_Number::operator <=> (const Int_Number& Nu) const {
    if(this == &Nu)
        return 0;
    
    std::scoped_lock lock(guard, Nu.guard);

    if(sign_plus && !Nu.sign_plus) return 1;
    if(!sign_plus && Nu.sign_plus) return -1;

    bool flag = sign_plus;

    if(len_num > Nu.len_num)
        return flag ? 1 : -1;
    if(len_num < Nu.len_num)
        return flag ? -1 : 1;
    
    for(int i = 0; i <= ptr_head - plt_tail; i++){
        if (num[ptr_head - i] < Nu.num[Nu.ptr_head - i])
            return flag ? -1 : 1;
        if (num[ptr_head - i] > Nu.num[Nu.ptr_head - i])
            return flag ? 1 : -1;
    }

    return 0;
}



Int_Number& Int_Number::operator-() {
    std::lock_guard<std::mutex> lock(guard);
    
    sign_plus = !sign_plus; 
    return *this;
}

void Int_Number::operator+=(const Int_Number& Nu) {
    if(this == &Nu){
        std::lock_guard<std::mutex> lock(guard);
        if(Nu.len_num > 32*9)
            this -> Add_In_Bypass_Paralel(Nu);
        else
            this -> Add_Bypass(Nu);
    }
    else{
        std::scoped_lock lock(guard, Nu.guard);
        if(Nu.len_num > 32*9)
            this -> Add_In_Bypass_Paralel(Nu);
        else
            this -> Add_Bypass(Nu);
    }
}
void Int_Number::operator-=(const Int_Number& Nu) {
    if(&guard == &Nu.guard){
        std::lock_guard<std::mutex> lock(guard);
        
        delete[] this -> num;

        num = new int[1];
        num[0] = 0;
        len_dedicated = 1;
        ptr_head = 0;
        plt_tail = 0;
        len_num = 1;
        sign_plus = true;
    }
    else{
        std::scoped_lock lock(guard, Nu.guard);
        if(Nu.len_num > 32*9)
            this -> Subtract_In_Bypass_Paralel(Nu);
        else
            this -> Subtract_Bypass(Nu);
    }
}

Int_Number Int_Number::operator+(const Int_Number& Nu) const {
    if(this == &Nu){
        std::lock_guard<std::mutex> lock(guard);

        Int_Number new_nu(std::move(this -> Copy_Bypass()));
        if(Nu.len_num > 32*9)
            new_nu.Add_In_Bypass_Paralel(Nu);
        else
            new_nu.Add_Bypass(Nu);
        return std::move(new_nu);
    }
    else{
        std::scoped_lock lock(guard, Nu.guard);

        Int_Number new_nu(std::move(this -> Copy_Bypass()));
        if(Nu.len_num > 32*9)
            new_nu.Add_In_Bypass_Paralel(Nu);
        else
            new_nu.Add_Bypass(Nu);
        return std::move(new_nu);
    }
}
Int_Number Int_Number::operator-(const Int_Number& Nu) const {
    if(this == &Nu)
        return std::move(Int_Number(0));
    
    std::scoped_lock lock(guard, Nu.guard);

    Int_Number new_nu(std::move(this -> Copy_Bypass()));
    if(Nu.len_num > 32*9)
        new_nu.Subtract_In_Bypass_Paralel(Nu);
    else
        new_nu.Subtract_Bypass(Nu);

    return std::move(new_nu);

}


Int_Number& Int_Number::operator<<(int i) {
    std::lock_guard<std::mutex> lock(guard);
    
    this -> Shift_Right_Bypass(i);

    return *this;
}
Int_Number& Int_Number::operator>>(int i) {
    std::lock_guard<std::mutex> lock(guard);
    
    this -> Shift_Left_Bypass(i);
    
    return *this;
}


void Int_Number::operator*=(int q) {
    std::lock_guard<std::mutex> lock(guard);

    this -> Multiplication_Bypass(q);
}
void Int_Number::operator*=(const Int_Number& Nu) {
    if(this == &Nu){
        std::lock_guard<std::mutex> lock(guard);
        if(Nu.len_num > 32*9){
            Int_Number qwe(std::move(this -> Multiplication_NTT_Outward_Thread_Pool_Bypass(Nu)));
            
            delete[] this -> num;

            num = qwe.num;
            ptr_head = qwe.ptr_head;
            plt_tail = qwe.plt_tail;
            len_dedicated = qwe.len_dedicated;
            len_num = qwe.len_num;
            sign_plus = qwe.sign_plus;

            qwe.num = nullptr;
            qwe.ptr_head = 0;
            qwe.plt_tail = 0;
            qwe.len_dedicated = 0;
            qwe.len_num = 1;
            qwe.sign_plus = true;
        }
        else
            this -> Multiplication_Bypass(Nu.Copy_Bypass());
    }
    else{
        std::scoped_lock lock(guard, Nu.guard);
        if(Nu.len_num > 32*9){
            Int_Number qwe(std::move(this -> Multiplication_NTT_Outward_Thread_Pool_Bypass(Nu)));
            
            delete[] this -> num;

            num = qwe.num;
            ptr_head = qwe.ptr_head;
            plt_tail = qwe.plt_tail;
            len_dedicated = qwe.len_dedicated;
            len_num = qwe.len_num;
            sign_plus = qwe.sign_plus;

            qwe.num = nullptr;
            qwe.ptr_head = 0;
            qwe.plt_tail = 0;
            qwe.len_dedicated = 0;
            qwe.len_num = 1;
            qwe.sign_plus = true;
        }
        else
            this -> Multiplication_Bypass(Nu);
    }
}

Int_Number Int_Number::operator*(int q) const {
    std::lock_guard<std::mutex> lock(guard);

    Int_Number new_nu(std::move(this -> Copy_Bypass()));
    new_nu.Multiplication_Bypass(q);
    return std::move(new_nu);
}
Int_Number Int_Number::operator*(const Int_Number& Nu) const {
    if(this == &Nu){
        std::lock_guard<std::mutex> lock(guard);

        if(Nu.len_num > 32*9)
            return std::move(this -> Multiplication_NTT_Outward_Thread_Pool_Bypass(Nu));

        Int_Number new_nu(std::move(this -> Copy_Bypass()));
        new_nu.Multiplication_Bypass(Nu);
        return std::move(new_nu);
    }
    else{
        std::scoped_lock lock(guard, Nu.guard);

        if(Nu.len_num > 32*9)
            return std::move(this -> Multiplication_NTT_Outward_Thread_Pool_Bypass(Nu));
            
        Int_Number new_nu(std::move(this -> Copy_Bypass()));
        new_nu.Multiplication_Bypass(Nu);
        return std::move(new_nu);
    }
}


void Int_Number::operator/=(const Int_Number& Nu) {
    if(Nu == 0)
        throw Error_Division_By_Zero();

    if(this == &Nu){
        std::lock_guard<std::mutex> lock(guard);
        Int_Number Q(1);

        delete[] this -> num;

        num = Q.num;
        ptr_head = Q.ptr_head;
        plt_tail = Q.plt_tail;
        len_dedicated = Q.len_dedicated;
        len_num = Q.len_num;
        sign_plus = Q.sign_plus;

        Q.num = nullptr;
        Q.ptr_head = 0;
        Q.plt_tail = 0;
        Q.len_dedicated = 0;
        Q.len_num = 1;
        Q.sign_plus = true;

        return; 
    }

    std::scoped_lock lock(guard, Nu.guard);

    Int_Number R(std::move(this -> Copy_Bypass()));
    Int_Number Nu2(std::move(Nu.Copy_Bypass()));
    Int_Number Q(0);

    int plus = 1;
    if(R.sign_plus != Nu2.sign_plus)
        plus = -1;


    Division_NOT_Bypass(Abs(R), Abs(Nu2), Q);

    Q *= plus;

    delete[] this -> num;

    num = Q.num;
    ptr_head = Q.ptr_head;
    plt_tail = Q.plt_tail;
    len_dedicated = Q.len_dedicated;
    len_num = Q.len_num;
    sign_plus = Q.sign_plus;

    Q.num = nullptr;
    Q.ptr_head = 0;
    Q.plt_tail = 0;
    Q.len_dedicated = 0;
    Q.len_num = 1;
    Q.sign_plus = true;
}
void Int_Number::operator%=(const Int_Number& Nu) {
    if(Nu == 0)
        throw Error_Division_By_Zero();

    if(this == &Nu){
        std::lock_guard<std::mutex> lock(guard);
        Int_Number Q(0);

        delete[] this -> num;

        num = Q.num;
        ptr_head = Q.ptr_head;
        plt_tail = Q.plt_tail;
        len_dedicated = Q.len_dedicated;
        len_num = Q.len_num;
        sign_plus = Q.sign_plus;

        Q.num = nullptr;
        Q.ptr_head = 0;
        Q.plt_tail = 0;
        Q.len_dedicated = 0;
        Q.len_num = 1;
        Q.sign_plus = true;
        
        return; 
    }

    std::scoped_lock lock(guard, Nu.guard);

    Int_Number R(std::move(this -> Copy_Bypass()));
    Int_Number Nu2(std::move(Nu.Copy_Bypass()));
    Int_Number Q(0);

    int plus = 1;
    if(R.sign_plus != Nu2.sign_plus)
        plus = -1;


    Division_NOT_Bypass(Abs(R), Abs(Nu2), Q);

    delete[] this -> num;

    num = R.num;
    ptr_head = R.ptr_head;
    plt_tail = R.plt_tail;
    len_dedicated = R.len_dedicated;
    len_num = R.len_num;
    sign_plus = R.sign_plus;

    R.num = nullptr;
    R.ptr_head = 0;
    R.plt_tail = 0;
    R.len_dedicated = 0;
    R.len_num = 1;
    R.sign_plus = true;
}

Int_Number Int_Number::operator/(const Int_Number& Nu) const {
    if(Nu == 0)
        throw Error_Division_By_Zero();

    if(this == &Nu)
        return Int_Number(1);


    std::scoped_lock lock(guard, Nu.guard);

    Int_Number R(std::move(this -> Copy_Bypass()));
    Int_Number Nu2(std::move(Nu.Copy_Bypass()));
    Int_Number Q(0);

    int plus = 1;
    if(R.sign_plus != Nu2.sign_plus)
        plus = -1;

    Division_NOT_Bypass(Abs(R), Abs(Nu2), Q);

    Q *= plus;

    return std::move(Q);
}
Int_Number Int_Number::operator%(const Int_Number& Nu) const {
    if(Nu == 0)
        throw Error_Division_By_Zero();

    if(this == &Nu)
        return Int_Number(0);


    std::scoped_lock lock(guard, Nu.guard);

    Int_Number R(std::move(this -> Copy_Bypass()));
    Int_Number Nu2(std::move(Nu.Copy_Bypass()));
    Int_Number Q(0);

    int plus = 1;
    if(R.sign_plus != Nu2.sign_plus)
        plus = -1;

    Division_NOT_Bypass(Abs(R), Abs(Nu2), Q);

    return std::move(R);
}



Int_Number NOD(const Int_Number& Nu1, const Int_Number& Nu2) {
    if(&Nu1 == &Nu2){
        std::lock_guard<std::mutex> lock(Nu1.guard);
        return std::move(Nu1.Copy_Bypass());
    }
    
    std::scoped_lock lock(Nu1.guard, Nu2.guard);

    Int_Number A(std::move(Nu1.Copy_Bypass()));
    Int_Number B(std::move(Nu2.Copy_Bypass()));

    Abs(A);
    Abs(B);

    if(A == 0)
        return std::move(B);
    if(B == 0)
        return std::move(A);

    Int_Number* ptr_1 = &A;
    Int_Number* ptr_2 = &B;

    while((*ptr_1 > 1) && (*ptr_2 > 1)){
        if(*ptr_1 < *ptr_2){
            Int_Number* svap = ptr_1;
            ptr_1 = ptr_2;
            ptr_2 = svap;
        }

        *ptr_1 %= *ptr_2;

        if(*ptr_1 == Int_Number(0))
            return std::move(*ptr_2);
    }

    return Int_Number(1);
}


Quotient_Number Int_Number::operator|(const Int_Number& Nu1) const {
    if(this == &Nu1)
        return std::move(Quotient_Number(1));

    std::scoped_lock lock(guard, Nu1.guard);

    Quotient_Number q = Quotient_Number(std::move(this -> Copy_Bypass()), std::move(Nu1.Copy_Bypass()));

    return std::move(q);
}


std::ofstream& operator<<(std::ofstream& of, const Int_Number& Nu) {
    std::lock_guard<std::mutex> lock(Nu.guard);

    long i = Nu.ptr_head;

    if(Nu.sign_plus)
        of << "+ ";
    else    
        of << "- ";
    
    std::string first_num = std::to_string(Nu.num[i--]);
    int len_first_num = first_num.length();

    if(len_first_num > 3)
        first_num.insert(len_first_num - 3, "_");
    if(len_first_num > 6)
        first_num.insert(len_first_num - 6, "_");

    of << first_num << "  ";
    
    while (i >= Nu.plt_tail){
        int q = Nu.num[i];
        int a1 = Nu.num[i] / 1000000;
        int a2 = Nu.num[i] % 1000000 / 1000;
        int a3 = Nu.num[i] % 1000;

        if (Length_Int(a1) == 1)
            of << "00" << a1 << "_";
        else if (Length_Int(a1) == 2)
            of << "0" << a1 << "_";
        else
            of << "" << a1 << "_";
        
        if (Length_Int(a2) == 1)
            of << "00" << a2 << "_";
        else if (Length_Int(a2) == 2)
            of << "0" << a2 << "_";
        else
            of << "" << a2 << "_";

        if (Length_Int(a3) == 1)
            of << "00" << a3 << "  ";
        else if (Length_Int(a3) == 2)
            of << "0" << a3 << "  ";
        else
            of << "" << a3 << "  ";
        
        i--;
    }
    of << std::endl;
    return of;
}
std::ostream& operator<<(std::ostream& os, const Int_Number& Nu) {
    std::lock_guard<std::mutex> lock(Nu.guard);

    long i = Nu.ptr_head;

    if(Nu.sign_plus)
        os << "+ ";
    else    
        os << "- ";
    
    std::string first_num = std::to_string(Nu.num[i--]);
    int len_first_num = first_num.length();

    if(len_first_num > 3)
        first_num.insert(len_first_num - 3, "_");
    if(len_first_num > 6)
        first_num.insert(len_first_num - 6, "_");

    os << first_num + "  ";
    
    while (i >= Nu.plt_tail){
        int q = Nu.num[i];
        int a1 = Nu.num[i] / 1000000;
        int a2 = Nu.num[i] % 1000000 / 1000;
        int a3 = Nu.num[i] % 1000;

        if (Length_Int(a1) == 1)
            os << "00" << a1 << "_";
        else if (Length_Int(a1) == 2)
            os << "0" << a1 << "_";
        else
            os << a1 << "_";
        
        if (Length_Int(a2) == 1)
            os << "00" << a2 << "_";
        else if (Length_Int(a2) == 2)
            os << "0" << a2 << "_";
        else
            os << a2 << "_";

        if (Length_Int(a3) == 1)
            os << "00" << a3 << "  ";
        else if (Length_Int(a3) == 2)
            os << "0" << a3 << "  ";
        else
            os << a3 << "  ";
        
        i--;
    }
    os << std::endl;
    return os;
}

std::istream& operator >> (std::istream& is, Int_Number& Nu){
    try{
        std::string lin;
        std::getline(is, lin);


        bool plus_flag = true;
        if(lin[0] == '-')
            plus_flag = false;


        int mas[lin.length()/9 + 2];
        int poz = 0;

        int q = 0;
        int shift = 1;
        for(int i = lin.length() - 1; i > -1; i--){
            if(shift > 100000001){
                mas[poz++] = q;
                q = 0;
                shift = 1;
            }

            int coun = int(lin[i]) - 48;

            if((coun >= 0) && (coun <= 9)){
                q += coun * shift;
                shift *= 10;
            }
        }

        mas[poz++] = q;

        std::lock_guard<std::mutex> lock(Nu.guard);
        Nu.Multiplication_Bypass(0);
        for(int i = 0; i < poz; i++)
            Nu.Only_Add_With_Shift_Bypass(Int_Number(mas[i]), i);

        if(Nu.Comparator_Bypass(0) != 0)
            Nu.sign_plus = plus_flag;
    }
    catch(...){
        throw Error_Input();
    }

    return is;
}
std::ifstream& operator >> (std::ifstream& is, Int_Number& Nu){
    try{
        std::string lin;
        std::getline(is, lin);

        
        bool plus_flag = true;
        if(lin[0] == '-')
            plus_flag = false;

        
        int mas[lin.length()/9 + 2];
        long long int poz = 0;

        int q = 0;
        int shift = 1;
        for(long long int i = lin.length() - 1; i > -1; i--){
            if(shift > 100000001){
                mas[poz++] = q;
                q = 0;
                shift = 1;
            }

            int coun = int(lin[i]) - 48;

            if((coun >= 0) && (coun <= 9)){
                q += coun * shift;
                shift *= 10;
            }
        }

        mas[poz++] = q;

        std::lock_guard<std::mutex> lock(Nu.guard);
        Nu.Multiplication_Bypass(0);
        for(long long int i = 0; i < poz; i++)
            Nu.Only_Add_With_Shift_Bypass(Int_Number(mas[i]), i);
        
        if(Nu.Comparator_Bypass(0) != 0)
            Nu.sign_plus = plus_flag;
    }
    catch(...){
        throw Error_Input();
    }

    return is;
}


Int_Number::~Int_Number() {
    delete[] num;
}
