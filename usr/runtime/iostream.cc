#include <iostream.hh>

extern "C"
{
#include <string.h>
#include <stdio.h>
}

namespace std
{
    ostream cout{};

    ostream &ostream::operator<<(const char *str)
    {
        printf("%s", str);
        return *this;
    }

    ostream &ostream::operator<<(const int &num)
    {
        printf("%d", num);
        return *this;
    }

    ostream &ostream::operator<<(const unsigned int &num)
    {
        printf("%u", num);
        return *this;
    }


    ostream &ostream::operator<<(ostream &(*func)(ostream &out))
    {
        return func(*this);
    }

    ostream::ostream()
    {
    }

}