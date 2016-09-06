#include <pa/algos.h>
#include <pa/exprs.h>

#include <iostream>

int main()
{
	{
		int ar_test[] = {0, 0, 1, 2, 3, 3, 4, 5, 5, 6, 7, 7, 7};
		int* ar_end = pa::remove_consecutives(std::begin(ar_test), std::end(ar_test));
		std::cout << std::distance(ar_test, ar_end) << std::endl;
		for (int* pI = &ar_test[0]; pI < ar_end; pI++) {
			std::cout << *pI << ",";
		}
		std::cout << std::endl;
	}

	{
		int ar_test[] = {0, 1, 2, 3, 4, 5, 6};
		int* ar_end = pa::remove_consecutives(std::begin(ar_test), std::end(ar_test));
		std::cout << std::distance(ar_test, ar_end) << std::endl;
		for (int* pI = &ar_test[0]; pI < ar_end; pI++) {
			std::cout << *pI << ",";
		}
		std::cout << std::endl;
	}

	{
		int ar_test[] = {1};
		int* ar_end = pa::remove_consecutives(std::begin(ar_test), std::end(ar_test));
		std::cout << std::distance(ar_test, ar_end) << std::endl;
		for (int* pI = &ar_test[0]; pI < ar_end; pI++) {
			std::cout << *pI << ",";
		}
		std::cout << std::endl;
	}


	{
		int ar_test[] = {0, 0, 0, 0};
		int* ar_end = pa::remove_consecutives(std::begin(ar_test), std::end(ar_test));
		std::cout << std::distance(ar_test, ar_end) << std::endl;
		for (int* pI = &ar_test[0]; pI < ar_end; pI++) {
			std::cout << *pI << ",";
		}
		std::cout << std::endl;
	}
	return 0;
}
