#include <vector>
#include <iostream>
#include <map>

class A
{
	int a;
};

void check_leak(void)
{
	system("leaks a.out");
}

int main(void)
{
	atexit(check_leak);
	A *ptr = new A();
	std::vector<A*> v;
	v.push_back(ptr);
	v.erase(v.begin());
}

