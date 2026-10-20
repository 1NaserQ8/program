#include <iostream>
using namespace std;

int main() {
    int a = 1;

    // auto b = []()  { return a++; };   // Error: a is not captured
    // auto c = []()  { return a++; };   // Error: a is not captured
    // auto d = []()  { return a + 1; }; // Error: a is not captured

    auto e = [&]() { return a++; };
    auto f = [&](int x) { return a += x; };

    cout << "e() = " << e() << endl;
    cout << "a after e = " << a << endl;

    cout << "f(5) = " << f(5) << endl;
    cout << "a after f = " << a << endl;

    return 0;
}
