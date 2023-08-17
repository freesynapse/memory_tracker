
#include <vector>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>

#include "syn_allocator.h"
#include "header.h"


using namespace Syn;



// shared pointer example

//
class testClass
{
public:
    testClass(int a, double b, const std::string& c) :
	m_a(a), m_b(b), m_c(c)
    {}

    void print() { std::cout << "a=" << m_a << ", b=" << m_b << ", c='" << m_c << "'.\n"; }
public:
    int m_a;
    double m_b;
    std::string m_c;
};


int main()
{
    Syn::vector<int> vec = SYN_VECTOR(int);
    std::cout << vec.size() << "\n";
    for (int i = 0; i < 20; i++)
	vec.push_back(i);
    Syn::vector<double> vec1 = SYN_VECTOR(double); vec1.reserve(10);

    Syn::map<int, std::string> map0 = SYN_MAP(int, std::string);
    map0[0] = "1000";
    map0[10] = "apaspdksafkdskfjksfjkajfkdsf";

    Syn::unordered_map<int, std::string> umap0 = SYN_UNORDERED_MAP(int, std::string);
    umap0[90] = "kdflsfd";
    umap0[32] = "jkdsöadskf";

    Syn::list<int> list0 = SYN_LIST(int);
    list0.push_back(100);
    list0.push_back(1);

    std::cout << "\nfootprint of STL allocation tracking: " << s_STLMemRsrcHandler.getMemSize() << " bytes.\n";

    std::cout << "\n\n";
	
    int* pint = SYN_ALLOCATE(int, 100);
    int* pint2 = SYN_ALLOCATE_N(int, 12000);
    int* pint3 = SYN_ALLOCATE(int);
    SYN_DEALLOCATE(pint);
    SYN_DEALLOCATE_N(pint2);
    SYN_DEALLOCATE(pint3);
    test_fnc0();
    test_fnc1();

    memory_log::print_alloc_all(false, true);
    std::cout << "\n\n";

}



