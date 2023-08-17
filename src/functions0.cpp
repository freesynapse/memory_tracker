
#include "header.h"


void test_fnc0()
{
    // Syn::vector<int> vec0 = SYN_VECTOR(int); vec0.reserve(100);
    auto vec0 = SYN_VECTOR(int); vec0.reserve(100);
    // Syn::vector<int> vec1 = SYN_VECTOR(int); vec1.reserve(100);
    auto vec1 = SYN_VECTOR(int); vec1.reserve(100);


    Syn::Ref<int> ref0 = SYN_MAKE_REF(int);


    Syn::vector<int> vec2 = SYN_VECTOR(int); vec2.reserve(100);

    Syn::Ref<int> ref3 = SYN_MAKE_REF(int);
    Syn::Ref<int> ref4 = SYN_MAKE_REF(int);

    Syn::vector<int> vec3 = SYN_VECTOR(int); vec3.reserve(100);
    Syn::vector<int> vec4 = SYN_VECTOR(int); vec4.reserve(100);

    Syn::Ref<int> ref2 = SYN_MAKE_REF(int);
}



