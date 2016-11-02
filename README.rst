Arybo
=====

Arybo is a software for manipulating such expressions using bit vectors and
gives a bit-per-bit symbolic representation.

The ANF (Algebric Normal Form) form is used, which basically represents boolean
expressions using the XOR and AND operators.

The whole documentation is available here: https://pythonhosted.org/arybo/

Quick start
===========

Under Linux/OSX/Windows, Arybo can be installed through pip for Python 2 and 3:

.. code::

   $ pip install arybo

Please note that Python 2 support for pytanque under Windows isn't available, as the
official compiler for Python extensions is Visual Studio 2010, which can't
compile libpetanque.

You will need at least clang 3.5 or GCC 4.9 to compile the python extension
"pytanque" under Linux/OS.

Users of Ubuntu 14.04 need to install GCC 6. See the documentation for detailed
instructions:
https://github.com/quarkslab/arybo/blob/master/docs/source/setup.rst#notes-for-ubuntu-1404.

More details on the installation process can be found here: https://pythonhosted.org/arybo/setup.html

To quickly use Arybo, you can the IPython shell by simply launching ``iarybo``:

.. code::

   # Starts an IPython interactive shell with 8-bit symbolic variables defined
   $ iarybo 8
   In [1]: x|0x7f
   Out[1]:Vec([
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   x7
   ])
   In [2]: (x^y)&a
   Out[2]: 
   Vec([
   ((x0 * a0) + (y0 * a0)),
   ((x1 * a1) + (y1 * a1)),
   ((x2 * a2) + (y2 * a2)),
   ((x3 * a3) + (y3 * a3)),
   ((x4 * a4) + (y4 * a4)),
   ((x5 * a5) + (y5 * a5)),
   ((x6 * a6) + (y6 * a6)),
   ((x7 * a7) + (y7 * a7))
   ])

Tutorials can be found here: https://pythonhosted.org/arybo/tutorial.html.
Advanced usage examples can be found in the **examples** directory. 

License
=======

This is published under a BSD license (see LICENSE.txt file)

Contact
=======

For any issue, do not hesitate to open an issue/create a pull request on Github.
