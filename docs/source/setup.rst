============
Installation
============

Requirements
------------

petanque is compatible with OSX, various flavors of Linux/Unix and Windows.
Python bindings can be compiled for Python 2 or 3, with the exception of Python
2 not available under Windows (see below).

Requirements for compiling petanque is clang >= 3.5 or GCC >= 4.9 (under
Unixes/OSX), and Visual Studio 2015 Update 3 under Windows.

Under Ubuntu >= 16.04, one can simply do:

.. code:: bash

    $ sudo apt-get install build-essentials g++

Arybo is a Python 2 and 3 library. For now, only pytanque can be used as a
backend for handling symbolic boolean expression.

Quick start using pip
---------------------

Under Linux, you first need to check that you have at least clang >= 3.5 or
GCC >= 4.9 (see above). 

Then, under Windows/OSX/Linux, simply do:

.. code:: bash

    $ pip install arybo

This will download and install petanque for you, and then arybo.

pytanque (libpetanque python bindings) can also be installed independently
using pip:

.. code:: bash

    $ pip install pytanque

Under OSX and Windows, binary distributions of pytanque are provided.

Please note that pytanque support for Python 2 isn't available under Windows,
as the official Python 2.7 release is compiled with Visual Studio 2010, which
can't compile petanque. We are investigating to fix this issue.

Notes for Ubuntu 14.04
~~~~~~~~~~~~~~~~~~~~~~

Ubuntu 14.04 is shipped with GCC 4.8, which does not support some C++14
features libpetanque is using.

Fortunately, a PPA allows to install GCC 6 and a more recent libstdc++.
To install pytanque through pip under Ubuntu 14.04, follow these instructions:

.. code:: bash

    $ sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    $ sudo apt-get update
    $ sudo apt-get install g++-6
    $ CC=gcc-6 CXX=g++-6 pip install pytanque

This will add the necessary PPA, install GCC 6 and compile the pytanque Python
bindings using GCC 6.

You can then install arybo through pip:

.. code:: bash

    $ pip install arybo

Installation from the source using distutils
--------------------------------------------

pytanque (petanque python bindings) can be installed directly from the source:

.. code:: bash

    $ cd /path/to/arybo/petanque
    $ python ./setup.py install

This will compile petanque and the python bindings (pytanque) inside a unique
python module and be installed.

Then, install the arybo library:

.. code:: bash

    $ cd /path/to/arybo
    $ python ./setup.py install

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

LLVM Windows support
~~~~~~~~~~~~~~~~~~~~

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

llvmlite support
----------------

If you want to use features that needs LLVM (like the expression assembler, see
:meth:`arybo.lib.exprs_asm.asm_binary`), you need to have llvmlite installed.
This library isn't installed automatically though pip because it is not trivial
to install on every platform Arybo supports.  This allows Arybo to still be
easily installable for many setup in a quick way.

An easy way to install llvmlite under Debian-like system is::

  $ sudo apt-get install llvm-3.8-dev
  $ LLVM_CONFIG=/usr/lib/llvm-config-3.8 pip install llvmlite

For other OS, please refer to the documentation of llvmlite here:
https://llvmlite.readthedocs.io/en/latest/install/index.html.
