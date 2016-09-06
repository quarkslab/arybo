#include <pa/exprs.h>
#include <pa/syms_set.h>
#include <pa/prettyprinter.h>
#include <pa/symbols.h>

#define ASSERT_VALID(b)\
	if (!(b)) {\
		std::cerr << #b << " is false" << std::endl;\
		ret = 1;\
	}

int main()
{
	pa::SymbolsSet sset;
	pa::ExprSym sA = pa::symbol("sA");
	pa::ExprSym sB = pa::symbol("sB");
	pa::ExprSym sC = pa::symbol("sC");
	pa::ExprSym sD = pa::symbol("sD");

	int ret = 0;

	ASSERT_VALID(sset.insert(sA) == true);
	ASSERT_VALID(sset.insert(sA) == false);
	ASSERT_VALID(sset.has(sA)); 
	ASSERT_VALID(sset.insert(sB) == true);
	ASSERT_VALID(sset.insert(sC) == true);
	ASSERT_VALID(sset.insert(sD) == true);
	ASSERT_VALID(sset.insert(sB) == false);
	ASSERT_VALID(sset.insert(sC) == false);
	ASSERT_VALID(sset.insert(sD) == false);

	std::set<pa::ExprSym> list;
	for (pa::ExprSym S: sset) {
		list.insert(S);
	}
	std::set<pa::ExprSym> ref;
	ref.insert(sA);
	ref.insert(sB);
	ref.insert(sC);
	ref.insert(sD);
	ASSERT_VALID((list == ref));


	return ret;
}
