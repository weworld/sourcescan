#include <cstdlib>
#include <cstring>
#include <iostream>

#define ABC(x)  { "abc" ## x }

char* source()
{
	return strdup("echo too bad!"); // /source/path/may/inject/evil/cmd
}

void sink(char *p)
{
	system(p);
}

struct B {
	B(const char *s) : f(strdup(s)) {
	}
	~B() {
		free(f);
	}

	char *f;
};

struct A {
	A() : g("/init/path0") {
	}

	B g;
};

void foo(A *z)
{
	B *x = &z->g;
	char *w = source();
	x->f = w; // valid code, but leaks
}

int main(int argc, char **argv)
{
	A *a = new A();
	B &b = a->g;
	if (true) {
		foo(a);
		if (true) {
			std::cout << "Nice!" << std::endl;
		}
	}
	sink(b.f);
}

