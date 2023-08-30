#include <iostream.hh>

class Test {

public:
    Test() {
        std::cout << "Constructor." << std::endl;
    }

    ~Test() {
        std::cout << "Destructor." << std::endl;
    }

    int test;
};

Test test;

int main(void) 
{
    test.test = 10;
    std::cout << "Test iostream" <<std::endl;
    return 0;
}