#include <iostream>
using namespace std;

int main()
{
        // Declare 1st array
        int src[5] = {1, 2, 3, 4, 5};

        // Declare 2nd array
        int dst[5];

        // Declare lambda function F1
        auto F1 = [](int source[], int destination[], int size) {
                for (int i = 0; i < size; i++)
                {
                        destination[i] = source[i] * 2;
                }
        };

        F1(src, dst, 5);

        for(int i = 0; i < 5; i++)
        {
                cout << "src = " << src[i] << endl;
                cout << "dst = " << dst[i] << endl;
                cout << endl;
        }

        return 0;
}
