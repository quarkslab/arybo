============
Installation
============

Requirements
------------

petanque is compatible with OSX, various flavors of Linux/Unix and Windows.
Python bindings can be compiled for python 2 or 3.

Requirements for compiling petanque is clang >= 3.5 or GCC >= 4.9 (under
Unixes/OSX), and Visual Studio 2015 Update 3 under Windows.

Under Ubuntu >= 16.04, one can simply do:

.. code:: bash

    $ sudo apt-get install build-essentials g++

Arybo is a Python 3 only library. For now, only petanque can be used as a
backend for handling symbolic boolean expression.

Quick start using pip
---------------------

Under OSX/Linux, you first need to check that you have at least clang >= 3.5 or
GCC >= 4.9 (see above). 

Then, simply do (using python3 as Arybo is a python3-only library):

.. code:: bash

    $ pip3 install arybo

This will download and install petanque for you, and then arybo.

pytanque (libpetanque python bindings) can also be installed independently
using pip:

.. code:: bash

    $ pip3 install pytanque

Under Windows, binary distributions of pytanque are provided. They have been
compiled using Visual Studio 2015 Update 3 (see above). You can thus install
arybo/pytanque using the same commands as above.

Installation from the source using distutils
--------------------------------------------

pytanque (petanque python bindings) can be installed directly from the source:

.. code:: bash

    $ cd /path/to/arybo/petanque
    $ python3 ./setup.py install

This will compile petanque and the python bindings (pytanque) inside a unique
python module and be installed.

Then, install the arybo library:

.. code:: bash

    $ cd /path/to/arybo
    $ python3 ./setup.py install

Compilation of libpetanque and pytanque with CMake
--------------------------------------------------

CMake can be used to compile petanque and pytanque. Compiling petanque
like this allows the use of the Intel Threading Building Blocks library (if
available on your system) to parallelize some of the processing inside
petanque. This allows to have a separate petanque shared library if you
want a project just to link against this native part.

You need at least CMake 2.8. To compile it, do:

.. code:: bash

  $ cd /path/to/arybo/petanque
  $ mkdir build ; cd build
  $ cmake -DPYTHON_VERSION=X.X -DCMAKE_BUILD_TYPE=release ..
  $ make
  $ sudo make install

This will install the python bindings in the current python environment (this
takes into account virtualenv), and the petanque library system-wide.

Compilation of pytanque under Windows
-------------------------------------

Windows support can be achieved using Visual Studio 2015 Update 3.

To compile pytanque by hand, first make sure the Python 3.5 (or above) official
distribution has been downloaded from https://www.python.org/downloads/windows/.

Then, run the VS2015 developer shell and run:

.. code::

  > cd \path\to\arybo\petanque
  > python ./setup.py build
  > python ./setup.py install

This will compile and install pytanque. Then, install arybo:

.. code::

  > cd \path\to\arybo
  > python ./setup.py install

Please note that the ``python`` executable must point to a valid Python 3.5
installation.

Then, you can launch the ``iarybo`` script:

.. code::

  > cd \path\to\arybo
  > python bin\iarybo

Windows support (LLVM)
----------------------

We tried to compile petanque using Clang/LLVM 3.8.1. The petanque library
can be compiled, but the pytanque bindings compilation aborts because of
invalid LLVM IR emitted by Clang. We are investigating this to create a minimal
test case to submit the issue.

For those still interested to try and compile the pytanque bindings under
Windows, here are the instructions:

 * download the latest Microsoft Visual Studio 2015 Community edition: https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx and install it.
 * download CLang/LLVM 3.8.1: http://llvm.org/releases/3.8.1/LLVM-3.8.1-win64.exe and install it.
 * install Python3 for Windows: https://www.python.org/downloads/windows/ .
 * launch the `arybo/petanque/llvm_distutils_env.bat` script that will setup a
   command line with an environment to make distutils compiles with Clang.
 * go to the ``arybo/petanque`` directory and run ``python setup.py build``. The compiler should fail at compiling ``pytanque.cpp``.
