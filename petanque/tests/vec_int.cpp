#include <pa/vector.h>
#include <pa/prettyprinter.h>

int main()
{
	pa::Vector v;
	int ret = 0;
	for (int i = 0; i < 1024; i++) {
		v.set_int_be(i, 4);
		int vbe = v.get_int_be();
		int ref = i&15;
		if (vbe != ref) {
			std::cerr << "(be) " << pa::pretty_print(v) << " != " << ref << ", got " << vbe << std::endl;
			ret = 1;
		}

		v.set_int_le(i, 4);
		int vle = v.get_int_le();
		if (vle != ref) {
			std::cerr << "(le) " << pa::pretty_print(v) << " != " << ref << ", got " << vle << std::endl;
			ret = 1;
		}

		if (vle != vbe) {
			std::cerr << "for i = " << i << ", vbe != vle (" << vbe << " != " << vle << ")" << std::endl;
			ret = 1;
		}
	}

	return ret;
}
