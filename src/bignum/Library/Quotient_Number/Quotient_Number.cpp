#include "Library/Int_Number/Int_Number.h"
#include "Quotient_Number.h"


std::atomic<long long> Quotient_Number::fractional_part_output_length = 63;


void Quotient_Number::Irreducible_Fraction_Bypass(){
    if((up == 0) && (doun == 0))
        return;
    
    Int_Number q = NOD(up, doun);
    up /= q;
    doun /= q;
}

/*
Real_Number Quotient_Number::To_Real_Number() const{
    std::unique_lock<std::mutex> lock_u(guard);

    lock_u.lock();
    Int_Number modulo(up);
    Int_Number dout_Q(doun);
    lock_u.unlock();

    Int_Number whole(0);

    bool up_zero = modulo == 0;
    bool doun_zero = dout_Q == 0;


    if(up_zero && doun_zero){
        os << "Uncertainty";
        return os;
    }
    if(doun_zero){
        os << "Infinity";
        return os;
    }

    int output_length = Quotient_Number::fractional_part_output_length;

    Int_Number::Division_NOT_Bypass(modulo, dout_Q, whole);

    std::string output_str = whole.To_String();
    output_str += "  [.]  ";

    if(modulo == 0){
        output_str += "0";
        os << output_str;
        return os;
    }

    std::string output_str_tail;
    long long len_tail = 0;
    while ((modulo != 0) && (len_tail - 9 < output_length)){
        modulo << 1;

        Int_Number whole1(0);

        Int_Number::Division_NOT_Bypass(modulo, dout_Q, whole1);

        std::string output_str_tail1 = "000000000";
        std::string output_str_tail2 = whole1.To_Clear_String();

        int s = 0;
        while ((s < 9) && (output_str_tail2.length() - 1 - s > 0)){
            output_str_tail1[8 - s] = output_str_tail2[output_str_tail2.length() - 1 - s];
            s++;
        }
        output_str_tail += output_str_tail1;
        len_tail += 9;
    }

    int count = 0;
    int poz = 3;
    while(poz < output_str_tail.length()){
        if(count < 2){
            output_str_tail.insert(poz, "_");
            count++;

            poz += 3 + 1;
        }
        else{
            output_str_tail.insert(poz, "  ");
            count = 0;

            poz += 3 + 2;
        }
    }


    bool remainder_zero = true;
    bool tail_over = false;

    if(modulo != 0)
        remainder_zero = false;
    
    if (len_tail > output_length)
        tail_over = true;
    


    if(remainder_zero && !tail_over){
        while((output_str_tail[output_str_tail.length() - 1] == '0') || (output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();
        
        output_str += output_str_tail;

        os << output_str;
        return os;
    }

    if(remainder_zero){
        while((output_str_tail[output_str_tail.length() - 1] == '0') || (output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_')){
            if(output_str_tail[output_str_tail.length() - 1] == '0')
                len_tail--;
        
            output_str_tail.pop_back();
        }

        if (len_tail <= output_length){
            output_str += output_str_tail;
            os << output_str;
            return os;
        }

        while(len_tail > output_length){
            if((output_str_tail[output_str_tail.length() - 1] != ' ') && (output_str_tail[output_str_tail.length() - 1] != '_'))
                len_tail--;
        
            output_str_tail.pop_back();
        }

        if((output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();
        if((output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();

        output_str += output_str_tail + "...";
        

        os << output_str;
        return os;
    }



    while(len_tail > output_length){
        if((output_str_tail[output_str_tail.length() - 1] != ' ') && (output_str_tail[output_str_tail.length() - 1] != '_'))
                len_tail--;
        
            output_str_tail.pop_back();
        }

    if((output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();
    
    output_str += output_str_tail + "...";
    
    os << output_str;
    return os;
}
*/





Quotient_Number::Quotient_Number(): up(0), doun(1), sign_plus(true) {}
Quotient_Number::Quotient_Number(int u): up(u), doun(1), sign_plus(true) {
    sign_plus = up.Is_plus(); 
    Abs(up);
}
Quotient_Number::Quotient_Number(float n){
    long long mantissa;
    int shift;


    extract_float(n, mantissa, shift);

    if(mantissa < 0){
        mantissa = -mantissa;
        sign_plus = false;
    }
    else{sign_plus = true;}

    int shift_9 = shift / 9;
    int shift_ne_9 = ((shift % 9) + 9) % 9;

    if(shift < 0){
        if(shift_ne_9 != 0)
            shift_9--;
        
        while(shift_ne_9-- > 0)
            mantissa *= 10;

        up = mantissa;
        doun = Int_Number(1) << (-shift_9);

        return;
    }

    while (shift_ne_9-- > 0)
        mantissa *= 10;
    
    up = mantissa;
    up << shift_9;
    doun = 1;
}
Quotient_Number::Quotient_Number(double n){
    long long mantissa;
    int shift;

    extract_double(n, mantissa, shift);

    if(mantissa < 0){
        mantissa = -mantissa;
        sign_plus = false;
    }
    else{sign_plus = true;}
    
    int shift_9 = shift / 9;
    int shift_ne_9 = ((shift % 9) + 9) % 9;

    if(shift < 0){
        if(shift_ne_9 != 0)
            shift_9--;
        
        up = mantissa;
        while(shift_ne_9-- > 0)
            up *= 10;

        doun = Int_Number(1) << (-shift_9);

        return;
    }

    up = mantissa;
    while (shift_ne_9-- > 0)
        up *= 10;
    
    up << shift_9;
    doun = 1;
}
Quotient_Number::Quotient_Number(long double n){
    long long mantissa;
    int shift;

    extract_longdouble(n, mantissa, shift);

    if(mantissa < 0){
        mantissa = -mantissa;
        sign_plus = false;
    }
    else{sign_plus = true;}
    
    int shift_9 = shift / 9;
    int shift_ne_9 = ((shift % 9) + 9) % 9;

    if(shift < 0){
        if(shift_ne_9 != 0)
            shift_9--;
        
        up = mantissa;
        while(shift_ne_9-- > 0)
            up *= 10;

        doun = Int_Number(1) << (-shift_9);

        return;
    }

    up = mantissa;
    while (shift_ne_9-- > 0)
        up *= 10;
    
    up << shift_9;
    doun = 1;
}
Quotient_Number::Quotient_Number(const Int_Number& u): up(u), doun(1), sign_plus(true) {
    sign_plus = up.Is_plus(); 
    Abs(up);
}
Quotient_Number::Quotient_Number(const Int_Number& u1, const Int_Number& u2): up(u1), doun(u2), sign_plus(true) {
    sign_plus = up.Is_plus() == doun.Is_plus(); 
    Abs(up); 
    Abs(doun);
    if(up == 0) sign_plus = true;
    this -> Irreducible_Fraction_Bypass();
}
Quotient_Number::Quotient_Number(const Int_Number& u1, const Int_Number& u2, bool plus): up(u1), doun(u2), sign_plus(plus) {
    Abs(up); 
    Abs(doun);
    if(up == 0) sign_plus = true;
    this -> Irreducible_Fraction_Bypass();
}
Quotient_Number::Quotient_Number(const Quotient_Number& q): up(q.up), doun(q.doun), sign_plus(q.sign_plus) {}

Quotient_Number::Quotient_Number(Quotient_Number&& q): up(std::move(q.up)), doun(std::move(q.doun)), sign_plus(q.sign_plus) {}
Quotient_Number::Quotient_Number(Int_Number&& u1, Int_Number&& u2): up(u1), doun(u2), sign_plus(true) {
    sign_plus = up.Is_plus() == doun.Is_plus(); 
    Abs(up); 
    Abs(doun);
    if(up == 0) sign_plus = true;
    this -> Irreducible_Fraction_Bypass();
}
Quotient_Number::Quotient_Number(Int_Number&& u1, Int_Number&& u2, bool plus): up(u1), doun(u2), sign_plus(plus) { 
    Abs(up); 
    Abs(doun);
    if(up == 0) sign_plus = true;
    this -> Irreducible_Fraction_Bypass();
}




void Quotient_Number::Make_Irreducible_Fraction() {
    std::lock_guard<std::mutex> lock(guard);

    if((up == 0) && (doun == 0))
        return;

    Int_Number q = NOD(up, doun);
    up /= q;
    doun /= q;
}



int Quotient_Number::operator <=> (const Quotient_Number& Q) const{
    if(this == &Q)
        return 0;
    
    std::scoped_lock lock(guard, Q.guard);

    int is_0_a = up <=> 0;
    int is_0_b = doun <=> 0;
    int is_0_c = Q.up <=> 0;
    int is_0_d = Q.doun <=> 0;

    if(((is_0_a == 0) && (is_0_b == 0)) || ((is_0_c == 0) && (is_0_d == 0)))
        throw Error_Comparison_Uncertainty();
    
    if(sign_plus && !Q.sign_plus) return 1;
    if(!sign_plus && Q.sign_plus) return -1;

    bool flag = sign_plus;

    if ((is_0_b == 0) && (is_0_d == 0))
        return flag ? up <=> Q.up : Q.up <=> up;
        //throw Error_Comparison_Infinities();
    
    if(is_0_b == 0)
        return flag ? 1 : -1;
    if(is_0_d == 0)
        return flag ? -1 : 1;
    

        
    int up_com = up <=> Q.up;
    int doun_com = doun <=> Q.doun;

    if((up_com == 0) && (doun_com == 0))
        return 0;

    if((up_com == 1) && (doun_com == -1))
        return flag ? 1 : -1;
    if((up_com == -1) && (doun_com == 1))
        return flag ? -1 : 1;
    
    if((up_com == 0) && (doun_com == -1))
        return flag ? 1 : -1;
    if((up_com == 0) && (doun_com == 1))
        return flag ? -1 : 1; 
    
    if((up_com == 1) && (doun_com == 0))
        return flag ? 1 : -1;
    if((up_com == -1) && (doun_com == 0))
        return flag ? -1 : 1; 
    


    int final_com = (up * Q.doun) <=> (Q.up * doun);
    
    return flag ? final_com : -final_com;
}


Quotient_Number& Quotient_Number::operator-() {
    std::lock_guard<std::mutex> lock(guard);
    
    sign_plus = !sign_plus; 
    return *this;
}
Quotient_Number Quotient_Number::operator-() const{
    Quotient_Number q(*this);
    
    q.sign_plus = !sign_plus; 
    return std::move(q);
}


Quotient_Number Quotient_Number::operator+(const Quotient_Number& Q) const {
    Int_Number new_up;
    Int_Number new_doun;
    
    if(this == &Q){
        std::lock_guard<std::mutex> lock(guard);
        new_doun = Q.doun * doun;
        if(sign_plus == Q.sign_plus){
            new_up = up * Q.doun + Q.up * doun;
            Quotient_Number q(std::move(new_up), std::move(new_doun), sign_plus);
            q.Irreducible_Fraction_Bypass();
            return q;
        }
    }
    else{
        std::scoped_lock lock(guard, Q.guard);
        new_doun = Q.doun * doun;
        if(sign_plus == Q.sign_plus){
            new_up = up * Q.doun + Q.up * doun;
            Quotient_Number q(std::move(new_up), std::move(new_doun), sign_plus);
            q.Irreducible_Fraction_Bypass();
            return q;
        }
    }
    

    if (sign_plus)
        new_up = up * Q.doun - Q.up * doun;
    else
        new_up = Q.up * doun - up * Q.doun;
    
    
    Quotient_Number q(std::move(new_up), std::move(new_doun));
    q.Irreducible_Fraction_Bypass();
    return q;
}
Quotient_Number Quotient_Number::operator-(const Quotient_Number& Q) const {
    Int_Number new_up;
    Int_Number new_doun;
    
    if(this == &Q){
        std::lock_guard<std::mutex> lock(guard);
        new_doun = Q.doun * doun;
        if(sign_plus == Q.sign_plus){
            new_up = up * Q.doun - Q.up * doun;
            if (!sign_plus) new_up = -new_up; // Инвертируем для отрицательных
            Quotient_Number q(std::move(new_up), std::move(new_doun));
            q.Irreducible_Fraction_Bypass();
            return q;
        }
    }
    else{
        std::scoped_lock lock(guard, Q.guard);
        new_doun = Q.doun * doun;
        if(sign_plus == Q.sign_plus){
            new_up = up * Q.doun - Q.up * doun;
            if (!sign_plus) new_up = -new_up;
            Quotient_Number q(std::move(new_up), std::move(new_doun));
            q.Irreducible_Fraction_Bypass();
            return q;
        }
    }

    
    if (sign_plus) {
        new_up = up * Q.doun + Q.up * doun;
        Quotient_Number q(std::move(new_up), std::move(new_doun), true);
        q.Irreducible_Fraction_Bypass();
        return q;
    } else {
        new_up = up * Q.doun + Q.up * doun;
        Quotient_Number q(std::move(new_up), std::move(new_doun), false);
        q.Irreducible_Fraction_Bypass();
        return q;
    }
}
Quotient_Number Quotient_Number::operator*(const Quotient_Number& Q) const {
    Int_Number new_up;
    Int_Number new_doun;
    bool sing = true;
    
    if(this == &Q){
        std::lock_guard<std::mutex> lock(guard);

        new_up = up * Q.up;
        new_doun = Q.doun * doun;
    }
    else{
        std::scoped_lock lock(guard, Q.guard);
        
        new_up = up * Q.up;
        new_doun = Q.doun * doun;

        sing = this -> sign_plus == Q.sign_plus;
    }

    Quotient_Number q = Quotient_Number(std::move(new_up), std::move(new_doun), sing);
    q.Irreducible_Fraction_Bypass();
    return std::move(q);
}
Quotient_Number Quotient_Number::operator/(const Quotient_Number& Q) const {
    Int_Number new_up;
    Int_Number new_doun;
    bool sing;
    
    if(this == &Q){
        std::lock_guard<std::mutex> lock1(guard);

        Quotient_Number q = Quotient_Number(1, 1);

        return std::move(q);
    }
    else{
        std::scoped_lock lock(guard, Q.guard);
        
        new_up = up * Q.doun;
        new_doun = Q.up * doun;

        sing = this -> sign_plus == Q.sign_plus;
    }

    Quotient_Number q = Quotient_Number(std::move(new_up), std::move(new_doun), sing);
    q.Irreducible_Fraction_Bypass();
    return std::move(q);
}


void Quotient_Number::operator+=(const Quotient_Number& Q){
    if(this == &Q){
        std::lock_guard<std::mutex> lock(guard);
        // x + x = 2x. Просто умножаем числитель на 2, знаменатель не меняем.
        up *= 2; 
        if (up == 0) sign_plus = true;
        this->Irreducible_Fraction_Bypass();
        return;
    }
    
    // Захватываем оба мьютекса на всю операцию
    std::scoped_lock lock(guard, Q.guard);
    Int_Number new_doun = Q.doun * doun;
    Int_Number new_up;

    if(sign_plus == Q.sign_plus){
        new_up = up * Q.doun + Q.up * doun;
        up = std::move(new_up);
        doun = std::move(new_doun);
        if (up == 0) sign_plus = true; // Защита от -0
    }
    else {
        // Знаки разные. Вычисляем так, чтобы new_up мог стать отрицательным
        if (sign_plus) { 
            new_up = up * Q.doun - Q.up * doun;
        } else {         
            new_up = Q.up * doun - up * Q.doun;
        }
        
        // Извлекаем знак из результата и делаем up модулем
        sign_plus = new_up.Is_plus();
        up = std::move(new_up);
        Abs(up); 
        doun = std::move(new_doun);
        if (up == 0) sign_plus = true;
    }
    this->Irreducible_Fraction_Bypass();
}
void Quotient_Number::operator-=(const Quotient_Number& Q){
    if(this == &Q){
        std::lock_guard<std::mutex> lock(guard);
        // x - x = 0
        up = Int_Number(0);
        doun = Int_Number(1);
        sign_plus = true;
        return;
    }
    
    std::scoped_lock lock(guard, Q.guard);
    Int_Number new_doun = Q.doun * doun;
    Int_Number new_up;

    if(sign_plus == Q.sign_plus){
        // Если оба отрицательные (-A - (-B) = B - A), порядок вычитания инвертируется
        if (sign_plus) {
            new_up = up * Q.doun - Q.up * doun;
        } else {
            new_up = Q.up * doun - up * Q.doun;
        }
        
        sign_plus = new_up.Is_plus();
        up = std::move(new_up);
        Abs(up);
        doun = std::move(new_doun);
        if (up == 0) sign_plus = true;
    }
    else {
        // Знаки разные: вычитание превращается в сложение модулей
        new_up = up * Q.doun + Q.up * doun;
        up = std::move(new_up);
        doun = std::move(new_doun);
        // Знак сохраняется (отбрасываем от нуля в ту же сторону)
        if (up == 0) sign_plus = true;
    }
    this->Irreducible_Fraction_Bypass();
}
void Quotient_Number::operator*=(const Quotient_Number& Q){
    if(this == &Q){
        std::lock_guard<std::mutex> lock(guard);
        // x * x всегда >= 0
        up *= up;
        doun *= doun;
        sign_plus = true; 
        if (up == 0) sign_plus = true;
        this->Irreducible_Fraction_Bypass();
        return;
    }
    
    std::scoped_lock lock(guard, Q.guard);
    up *= Q.up;
    doun *= Q.doun;
    
    sign_plus = (sign_plus == Q.sign_plus);
    if (up == 0) sign_plus = true;
    
    this->Irreducible_Fraction_Bypass();
}
void Quotient_Number::operator/=(const Quotient_Number& Q){
    if(this == &Q){
        std::lock_guard<std::mutex> lock(guard);
        // Особые случаи для x / x
        if (up == 0 && doun == 0) {
            // Неопределённость / Неопределённость = Неопределённость (0/0)
        } else if (doun == 0) {
            // Бесконечность / Бесконечность = Неопределённость (0/0)
            up = Int_Number(0);
            doun = Int_Number(0);
            sign_plus = true;
        } else {
            // Конечное число / Само себя = 1
            up = Int_Number(1);
            doun = Int_Number(1);
            sign_plus = true;
        }
        return;
    }
    
    std::scoped_lock lock(guard, Q.guard);
    Int_Number new_up = up * Q.doun;
    Int_Number new_doun = Q.up * doun;
    
    up = std::move(new_up);
    doun = std::move(new_doun);
    
    sign_plus = (sign_plus == Q.sign_plus);
    if (up == 0) sign_plus = true; // Защита от -0
    
    this->Irreducible_Fraction_Bypass();
}

void Quotient_Number::operator=(Quotient_Number&& Q){
    if(this == &Q)
        return;
    
    std::scoped_lock lock(guard, Q.guard);

    up = std::move(Q.up);
    doun = std::move(Q.doun);
    sign_plus = Q.sign_plus;

    Q.sign_plus = true;
}
void Quotient_Number::operator=(const Quotient_Number& Q){
    if(this == &Q)
        return;
    
    std::scoped_lock lock(guard, Q.guard);

    up = Q.up;
    doun = Q.doun;
    sign_plus = Q.sign_plus;
}



Quotient_Number::operator float() const{
    Int_Number ans = up / doun;
    Int_Number ost = up - doun*ans;


    float fld = 0.0;

    long double a = 1.0;
    while (ans > 0){
        fld += ans.Last_Element_Get() * a;
        ans >> 1;
        a *= 1e+9;
    }

    a = 1e-9;
    for(int i = 0; i < 1; i++){
        ans = (ost << 1) / doun;
        ost = ost - doun*ans;
        fld += (double)ans.Last_Element_Get() * a;
        a *= 1e-9;
    }
    
    if (!sign_plus)
        fld = -fld;
    return fld;
}
Quotient_Number::operator double() const{
    Int_Number ans = up / doun;
    Int_Number ost = up - doun*ans;

    double fld = 0.0;

    long double a = 1.0;
    while (ans > 0){
        fld += ans.Last_Element_Get() * a;
        ans >> 1;
        a *= 1e+9;
    }

    a = 1e-9;
    for(int i = 0; i < 2; i++){
        ans = (ost << 1) / doun;
        ost = ost - doun*ans;
        fld += (double)ans.Last_Element_Get() * a;
        a *= 1e-9;
    }
    
    if (!sign_plus)
        fld = -fld;
    return fld;
}
Quotient_Number::operator long double() const{
    Int_Number ans = up / doun;
    Int_Number ost = up - doun*ans;

    long double fld = 0.0;

    long double a = 1.0;
    while (ans > 0){
        fld += ans.Last_Element_Get() * a;
        ans >> 1;
        a *= 1e+9;
    }

    a = 1e-9;
    for(int i = 0; i < 4; i++){
        ans = (ost << 1) / doun;
        ost = ost - doun*ans;
        fld += (double)ans.Last_Element_Get() * a;
        a *= 1e-9;
    }
    
    if (!sign_plus)
        fld = -fld;
    return fld;
}

std::ostream& operator<<(std::ostream& os, const Quotient_Number& Q) {
    std::lock_guard<std::mutex> lock(Q.guard);

    
    Int_Number modulo(Q.up);
    Int_Number dout_Q(Q.doun);
    Int_Number whole(0);

    bool up_zero = modulo == 0;
    bool doun_zero = dout_Q == 0;


    if(up_zero && doun_zero){
        os << "Uncertainty";
        return os;
    }
    if(doun_zero){
        os << "Infinity";
        return os;
    }

    int output_length = Quotient_Number::fractional_part_output_length;

    Int_Number::Division_NOT_Bypass(modulo, dout_Q, whole);

    std::string output_str = whole.To_String();
    if(!Q.sign_plus)
        output_str[0] = '-';

    output_str += "  [.]  ";

    if(modulo == 0){
        output_str += "0";
        os << output_str;
        return os;
    }

    std::string output_str_tail;
    long long len_tail = 0;
    while ((modulo != 0) && (len_tail - 9 < output_length)){
        modulo << 1;

        Int_Number whole1(0);

        Int_Number::Division_NOT_Bypass(modulo, dout_Q, whole1);

        std::string output_str_tail1 = "000000000";
        std::string output_str_tail2 = whole1.To_Clear_String();

        int s = 0;
        while ((s < 9) && (output_str_tail2.length() - 1 - s > 0)){
            output_str_tail1[8 - s] = output_str_tail2[output_str_tail2.length() - 1 - s];
            s++;
        }
        output_str_tail += output_str_tail1;
        len_tail += 9;
    }

    int count = 0;
    int poz = 3;
    while(poz < output_str_tail.length()){
        if(count < 2){
            output_str_tail.insert(poz, "_");
            count++;

            poz += 3 + 1;
        }
        else{
            output_str_tail.insert(poz, "  ");
            count = 0;

            poz += 3 + 2;
        }
    }


    bool remainder_zero = true;
    bool tail_over = false;

    if(modulo != 0)
        remainder_zero = false;
    
    if (len_tail > output_length)
        tail_over = true;
    


    if(remainder_zero && !tail_over){
        while((output_str_tail[output_str_tail.length() - 1] == '0') || (output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();
        
        output_str += output_str_tail;

        os << output_str;
        return os;
    }

    if(remainder_zero){
        while((output_str_tail[output_str_tail.length() - 1] == '0') || (output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_')){
            if(output_str_tail[output_str_tail.length() - 1] == '0')
                len_tail--;
        
            output_str_tail.pop_back();
        }

        if (len_tail <= output_length){
            output_str += output_str_tail;
            os << output_str;
            return os;
        }

        while(len_tail > output_length){
            if((output_str_tail[output_str_tail.length() - 1] != ' ') && (output_str_tail[output_str_tail.length() - 1] != '_'))
                len_tail--;
        
            output_str_tail.pop_back();
        }

        if((output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();
        if((output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();

        output_str += output_str_tail + "...";
        

        os << output_str;
        return os;
    }



    while(len_tail > output_length){
        if((output_str_tail[output_str_tail.length() - 1] != ' ') && (output_str_tail[output_str_tail.length() - 1] != '_'))
                len_tail--;
        
            output_str_tail.pop_back();
        }

    if((output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();
    
    output_str += output_str_tail + "...";
    
    os << output_str;
    return os;
}   
std::ofstream& operator<<(std::ofstream& of, const Quotient_Number& Q){
    std::lock_guard<std::mutex> lock(Q.guard);

    
    Int_Number modulo(Q.up);
    Int_Number dout_Q(Q.doun);
    Int_Number whole(0);

    bool up_zero = modulo == 0;
    bool doun_zero = dout_Q == 0;


    if(up_zero && doun_zero){
        of << "Uncertainty";
        return of;
    }
    if(doun_zero){
        of << "Infinity";
        return of;
    }
    
    int output_length = Quotient_Number::fractional_part_output_length;

    Int_Number::Division_NOT_Bypass(modulo, dout_Q, whole);

    std::string output_str = whole.To_String();
    if(!Q.sign_plus)
        output_str[0] = '-';

    output_str += "  [.]  ";

    if(modulo == 0){
        output_str += "0";
        of << output_str;
        return of;
    }


    std::string output_str_tail;
    long long len_tail = 0;
    while ((modulo != 0) && (len_tail - 9 < output_length)){
        modulo << 1;

        Int_Number whole1(0);

        Int_Number::Division_NOT_Bypass(modulo, dout_Q, whole1);

        std::string output_str_tail1 = "000000000";
        std::string output_str_tail2 = whole1.To_Clear_String();

        int s = 0;
        while ((s < 9) && (output_str_tail2.length() - 1 - s > 0)){
            output_str_tail1[8 - s] = output_str_tail2[output_str_tail2.length() - 1 - s];
            s++;
        }
        output_str_tail += output_str_tail1;
        len_tail += 9;
    }

    int count = 0;
    int poz = 3;
    while(poz < output_str_tail.length()){
        if(count < 2){
            output_str_tail.insert(poz, "_");
            count++;

            poz += 3 + 1;
        }
        else{
            output_str_tail.insert(poz, "  ");
            count = 0;

            poz += 3 + 2;
        }
    }


    bool remainder_zero = true;
    bool tail_over = false;

    if(modulo != 0)
        remainder_zero = false;
    
    if (len_tail > output_length)
        tail_over = true;
    


    if(remainder_zero && !tail_over){
        while((output_str_tail[output_str_tail.length() - 1] == '0') || (output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();
        
        output_str += output_str_tail;

        of << output_str;
        return of;
    }

    if(remainder_zero){
        while((output_str_tail[output_str_tail.length() - 1] == '0') || (output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_')){
            if(output_str_tail[output_str_tail.length() - 1] == '0')
                len_tail--;
        
            output_str_tail.pop_back();
        }

        if (len_tail <= output_length){
            output_str += output_str_tail;
            of << output_str;
            return of;
        }

        while(len_tail > output_length){
            if((output_str_tail[output_str_tail.length() - 1] != ' ') && (output_str_tail[output_str_tail.length() - 1] != '_'))
                len_tail--;
        
            output_str_tail.pop_back();
        }

        if((output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();
        if((output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();

        output_str += output_str_tail + "...";
        

        of << output_str;
        return of;
    }



    while(len_tail > output_length){
        if((output_str_tail[output_str_tail.length() - 1] != ' ') && (output_str_tail[output_str_tail.length() - 1] != '_'))
                len_tail--;
        
            output_str_tail.pop_back();
        }

    if((output_str_tail[output_str_tail.length() - 1] == ' ') || (output_str_tail[output_str_tail.length() - 1] == '_'))
            output_str_tail.pop_back();
    
    output_str += output_str_tail + "...";
    
    of << output_str;
    return of;
}   

std::istream& operator >> (std::istream& is, Quotient_Number& Q){
    try{
        std::string lin;
        std::getline(is, lin);

        bool plus_flag = true;
        if(lin[0] == '-')
            plus_flag = false;

        size_t index_dot = lin.find('.');
        size_t index_delenie = lin.find('/');

        if((index_dot != std::string::npos) && (index_delenie != std::string::npos))
            throw Error_Input();
        

        if(index_delenie != std::string::npos){
            std::string up = lin.substr(0, index_delenie);
            std::string doun = lin.substr(index_delenie + 1);

            
            Quotient_Number New_Q(Int_Number(up), Int_Number(doun), plus_flag);

            Q = std::move(New_Q);

            return is;
        }

        if(index_dot != std::string::npos){
            Int_Number up_int(lin);

            std::string frac_digits;

            for(char c : lin.substr(index_dot + 1)) {
                if(c >= '0' && c <= '9')
                    frac_digits.push_back(c);
}

            long len_doun_int = frac_digits.length();

            Int_Number doun_int_1(1);
            doun_int_1 << len_doun_int/9;
            for(int i = 0; i < len_doun_int % 9; i++)
                doun_int_1 *= 10;

            Quotient_Number New_Q(std::move(up_int), std::move(doun_int_1), plus_flag);

            Q = std::move(New_Q);
            
            return is;
        }

        Int_Number up_int(lin);
        Quotient_Number New_Q(std::move(up_int), 1, plus_flag);

        Q = std::move(New_Q);
        
        return is;
    }
    catch(...){
        throw Error_Input();
    }

    return is;
}
std::ifstream& operator >> (std::ifstream& is, Quotient_Number& Q){
    try{
        std::string lin;
        std::getline(is, lin);

        bool plus_flag = true;
        if(lin[0] == '-')
            plus_flag = false;

        size_t index_dot = lin.find('.');
        size_t index_delenie = lin.find('/');

        if((index_dot != std::string::npos) && (index_delenie != std::string::npos))
            throw Error_Input();
        

        if(index_delenie != std::string::npos){
            std::string up = lin.substr(0, index_delenie);
            std::string doun = lin.substr(index_delenie + 1);

            
            Quotient_Number New_Q(Int_Number(up), Int_Number(doun), plus_flag);

            Q = std::move(New_Q);

            return is;
        }

        if(index_dot != std::string::npos){
            Int_Number up_int(lin);

            std::string frac_digits;

            for(char c : lin.substr(index_dot + 1)) {
                if(c >= '0' && c <= '9')
                    frac_digits.push_back(c);
            }

            long len_doun_int = frac_digits.length();

            Int_Number doun_int_1(1);
            doun_int_1 << len_doun_int/9;
            for(int i = 0; i < len_doun_int % 9; i++)
                doun_int_1 *= 10;

            Quotient_Number New_Q(std::move(up_int), std::move(doun_int_1), plus_flag);

            Q = std::move(New_Q);
            
            return is;
        }

        Int_Number up_int(lin);
        Quotient_Number New_Q(std::move(up_int), 1, plus_flag);

        Q = std::move(New_Q);
        
        return is;
    }
    catch(...){
        throw Error_Input();
    }

    return is;
}

