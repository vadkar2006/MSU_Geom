#pragma once

#include <iostream>
#include <fstream>

#include "Library/Service_Names/Service_Names.h"


class Error{
protected:
    enum Type_Error{Int_Number, Quotient_Number, NTT};
    Type_Error error;
};







class Error_Int_Number: Error{
protected:
    enum Type_Int_Number_Error{Division_By_Zero, Output, Input};
    Type_Int_Number_Error error_int_number;
public:
    Error_Int_Number(){error = Int_Number;}
    void What(std::ostream* os){*os << "Int_Number ";}
};



class Error_Division_By_Zero: Error_Int_Number{
public:
    Error_Division_By_Zero(){error_int_number = Division_By_Zero;}
    void What(std::ostream* os){Error_Int_Number::What(os); *os << "Division_By_Zero" << std::endl;}
};

class Error_Output: Error_Int_Number{
public:
    Error_Output(){error_int_number = Output;}
    void What(std::ostream* os){Error_Int_Number::What(os); *os << "Output" << std::endl;}
};

class Error_Input: Error_Int_Number{
public:
    Error_Input(){error_int_number = Input;}
    void What(std::ostream* os){Error_Int_Number::What(os); *os << "Input" << std::endl;}
};





class Error_Quotient_Number: Error{
protected:
    enum Type_Quotient_Number_Error{Comparison_Uncertainty};
    Type_Quotient_Number_Error error_quotient_number;
public:
    Error_Quotient_Number(){error = Quotient_Number;}
    void What(std::ostream* os){*os << "Quotient_Number ";}
};


class Error_Comparison_Uncertainty: Error_Quotient_Number{
public:
    Error_Comparison_Uncertainty(){error_quotient_number = Comparison_Uncertainty;}
    void What(std::ostream* os){Error_Quotient_Number::What(os); *os << "Comparison_Uncertainty" << std::endl;}
};









class Error_NTT: Error{
protected:
    enum Type_NTT_Error{Size_Array, Too_High_Degree};
    Type_NTT_Error error_ntt;
public:
    Error_NTT(){error = NTT;}
    void What(std::ostream* os){*os << "NTT ";}
};



class Error_Size_Array: Error_NTT{
public:
    Error_Size_Array(){error_ntt = Size_Array;}
    void What(std::ostream* os){Error_NTT::What(os); *os << "Size_Array" << std::endl;}
};

class Error_Too_High_Degree: Error_NTT{
public:
    Error_Too_High_Degree(){error_ntt = Too_High_Degree;}
    void What(std::ostream* os){Error_NTT::What(os); *os << "Too_High_Degree" << std::endl;}
};