#include <iostream>
using namespace std;
int main(){
	int a = 1;                       
	auto b = []()  { return a++; }; 
	auto c = []()  { return a++; }; 
	auto d = []()  { return a+1; }; 
	auto e = [&]() { return a++; }; 
	auto f = [&](int x) { return a+=x; }; 
		
	return 0;
}
