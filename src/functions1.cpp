
#include "header.h"


void test_fnc1()
{
	Syn::vector<double> vec0 = SYN_VECTOR(double); vec0.reserve(100);



	Syn::vector<double> vec1 = SYN_VECTOR(double); vec1.reserve(100);


	Syn::vector<double> vec2 = SYN_VECTOR(double); vec2.reserve(100);
	Syn::vector<double> vec3 = SYN_VECTOR(double); vec3.reserve(100);



	Syn::Ref<int> ref1 = SYN_MAKE_REF(int);
	Syn::vector<double> vec4 = SYN_VECTOR(double); vec4.reserve(100);
	Syn::Ref<int> ref4 = SYN_MAKE_REF(int);



	Syn::Ref<int> ref0 = SYN_MAKE_REF(int);
	Syn::Ref<int> ref2 = SYN_MAKE_REF(int);
	Syn::Ref<int> ref3 = SYN_MAKE_REF(int);




}
