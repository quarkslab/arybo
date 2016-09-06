#include <pa/sorted_vector.h>
#include <iostream>
#include <vector>

typedef std::vector<int> vec_int;
typedef pa::SortedVector<vec_int> svec_int;

#define CHECK(b) _CHECK(__LINE__,b)
#define _CHECK(L,b)\
	if (!(b)) {\
		std::cerr << "error line " << L << std::endl;\
		ret = 1;\
	}

int main()
{
	int ret = 0;

	{
		vec_int ref{{10,20,40,50}};
		svec_int v0{true,ref};
		svec_int v1{true,ref};
		v0.insert(v1);
		CHECK(v0 == ref);
	}

	{
		vec_int ref{{10,20,40,50}};
		svec_int v0;
		svec_int v1{true,ref};
		v0.insert(v1);
		CHECK(v0 == ref);
	}

	{
		vec_int ref{{10,20,40,50}};
		svec_int v0{true,ref};
		svec_int v1;
		v0.insert(v1);
		CHECK(v0 == ref);
	}

	{
		vec_int ref;
		svec_int v0;
		svec_int v1;
		v0.insert(v1);
		CHECK(v0 == ref);
	}

	{
		vec_int ref{{1,10,20,40,45,50,55,56}};
		svec_int v0(true,vec_int{10,20,40,50});
		svec_int v1(true,vec_int{1,45,55,56});
		v0.insert(v1);
		CHECK(v0 == ref);
	}

	{
		vec_int ref{{1,10,20,40,50}};
		svec_int v0(true,vec_int{10,20,40,50});
		svec_int v1(true,vec_int{1});
		v0.insert(v1);
		CHECK(v0 == ref);
	}

	{
		vec_int ref{{10,20,40,50,55}};
		svec_int v0(true,vec_int{10,20,40,50});
		svec_int v1(true,vec_int{55});
		v0.insert(v1);
		CHECK(v0 == ref);
	}

	{
		vec_int ref{{1,10,11,12,14,15,20,21,40,50}};
		svec_int v0(true,vec_int{10,20,40,50});
		svec_int v1(true,vec_int{1,11,12,14,15,20,21});
		v0.insert(v1);
		CHECK(v0 == ref);
	}

	{
		vec_int ref{{1,2,4}};
		svec_int v0(true,vec_int{1,2});
		svec_int v1(true,vec_int{1,4});
		v0.insert(v1);
		CHECK(v0 == ref);
	}

	{
		vec_int ref{{0,9,10,11,12,15,16}};
		svec_int v0(true,vec_int{0,1,2,4,8,10,11,12});
		svec_int v1(true,vec_int{1,2,4,8,9,15,16});
		v0.insert(v1.begin(), v1.end(), [](svec_int& v, svec_int::const_iterator it) { return v.erase(it); });
		CHECK(v0 == ref);
	}

	{
		svec_int v(true, vec_int{5,6});
		v.insert(4);
		v.insert(3);
		v.insert(2);
		v.insert(1);
		v.insert(0);
		vec_int ref{{0,1,2,3,4,5,6}};
		CHECK(v == ref);
	}

	return ret;
}
