#pragma once

#pragma once

namespace std
{
    class ostream
    {
    public:
        explicit ostream();
        ~ostream() = default;
        ostream &operator<<(const char *str);
        ostream &operator<<(const int &num);
        ostream &operator<<(const unsigned int &num);

        ostream &operator<<(ostream &(*func)(ostream &out));
    };

    inline ostream &endl(ostream &out)
    {
        out << "\n";
        return out;
    }

    extern ostream cout;
}