.. _sec-tutorials:

Tutorials
=========

This tutorial is a small example of how Arybo can be used to solve mathematical
/ reverse engineering issues.

Symbloc evaluation of a complex function
----------------------------------------

*(based on the example in examples/xor_5C.py)*

From a reverse engineering point of view, it can be helpful to understand what
this function is doing (`extracted
<https://recon.cx/2014/slides/recon2014-21-mougey-camille-francis-gabriel-DRM-obfuscation-versus-auxiliary-attacks-slides.pdf>`_
by Camille Mouget and Francis Gabriel):

.. code:: python

   def f(x):
       v0 = x*0xe5 + 0xF7
       v0 = v0&0xFF
       v3 = (((((v0*0x26)+0x55)&0xFE)+(v0*0xED)+0xD6)&0xFF )
       v4 = ((((((- (v3*0x2))+0xFF)&0xFE)+v3)*0x03)+0x4D)
       v5 = (((((v4*0x56)+0x24)&0x46)*0x4B)+(v4*0xE7)+0x76)
       v7 = ((((v5*0x3A)+0xAF)&0xF4)+(v5*0x63)+0x2E)
       v6 = (v7&0x94)
       v8 = ((((v6+v6+(- (v7&0xFF)))*0x67)+0xD))
       res = ((v8*0x2D)+(((v8*0xAE)|0x22)*0xE5)+0xC2)&0xFF
       return (0xed*(res-0xF7))&0xff

This function takes an 8-bit integer as an input and produces an 8-bit integer
in the end. It looks like it's doing complex operations, mixing both classical
and boolean arithmetics. Let's use Arybo to figure out what's going on!

Let's start by importing Arybo and create an 8-bit MBA space and an associated
symbolic variable. Just print it to verify that this is a pure symbolic variable:

.. code:: python
    
   from arybo.lib import MBA
   mba = MBA(8)
   x = mba.var('x')
   print(x)

Running this code gives the following output:

.. code::

   $ python ./tuto1.py
   Vec([
   x0,
   x1,
   x2,
   x3,
   x4,
   x5,
   x6,
   x7
   ])

We can now compute the symbolic boolean expressions associated with the
function `f`. For this, simply copy-paste its code into your script, call it and
print the output:

.. code:: python

   from arybo.lib import MBA
   mba = MBA(8)
   x = mba.var('x')
   def f(x):
       [...]
   ret = f(x)
   print(ret)

Running this script gives the following output:

.. code::

   Vec([
   X0,
   X1,
   (X2 + 1),
   (X3 + 1),
   (X4 + 1),
   X5,
   (X6 + 1), 
   X7
   ]) 

As stated in the :ref:`theory section <sec-theory-anf>`, petanque expressions
are boolean expressions using additions and multiplications modulo 2. This means
than the addition is equivalent to a binary XOR, and the multiplication to a
binary AND. Thus, what we see in the output above is basically the input X
xored with the 8-bit constant `(0 0 1 1 1 0 1 0)` (with the LSB bit on the
left).

We can ask Arybo to find this constant for us thanks to the
:meth:`arybo.lib.MBAVariable.vectorial_decomp` function:

.. code:: python

   from arybo.lib import MBA
   mba = MBA(8)
   x = mba.var('x')
   def f(x):
       [...]
   ret = f(x)
   app = ret.vectorial_decomp([x])
   print(app)
   print(hex(app.cst().get_int_be()))

The output is the following:

.. code::

   App NL = Vec([
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0
   ])
   AffApp matrix = Mat([
   [1, 0, 0, 0, 0, 0, 0, 0]
   [0, 1, 0, 0, 0, 0, 0, 0]
   [0, 0, 1, 0, 0, 0, 0, 0]
   [0, 0, 0, 1, 0, 0, 0, 0]
   [0, 0, 0, 0, 1, 0, 0, 0]
   [0, 0, 0, 0, 0, 1, 0, 0]
   [0, 0, 0, 0, 0, 0, 1, 0]
   [0, 0, 0, 0, 0, 0, 0, 1]
   ])
   AffApp cst = Vec([
   0,
   0,
   1,
   1,
   1,
   0,
   1,
   0
   ])
   0x5c

We can see in the end the constant `0x5c`. The vectorial decomposition confirms
that this function is in the end simply a binary XOR of an 8-bit integer with
the `0x5c` constant.

.. _sec-tutorial-dirac:

Dirac function
--------------

*(based on the example in examples/dirac.py)*

A "dirac" function is a function that is always null in its domain except for
one value. These functions are interesting because reverse engineers could
bruteforce them and think after some moment that they are always returning
the same value. This could make the reverse engineer produce false code and/or
slow her down in her whole understanding of the program.

In such a case, Arybo allows us to prove on the whole input domain that the
function isn't constant, and to find which input produces a different value.

An example of a dirac function is this one:

.. code:: python

    def f(X):
      T = ((X+1)&(~X))
      C = ((T | 0x7AFAFA697AFAFA69) & 0x80A061440A061440)\
          + ((~T & 0x10401050504) | 0x1010104)
      return C

It takes a 64-bit input and produces a 64-bit input.

Trying some values output:

.. code:: python

    >>> print(f(0))
    45142941144388932
    >>> print(f(1))
    45142941144388932
    >>> print(f(10))
    45142941144388932
    >>> print(f(1<<32))
    45142941144388932

Bruteforcing this function that takes a 64-bit integer as input could take
months. Using Arybo, we can output the boolean symbolic expressions associated with this
function:

.. code:: python

    from arybo.lib import MBA
    mba = MBA(64)
    x = mba.var('x')
    def f(X):
      T = ((X+1)&(~X))
      C = ((T | 0x7AFAFA697AFAFA69) & 0x80A061440A061440)\
          + ((~T & 0x10401050504) | 0x1010104)
      return C
    print(f(x))
    >>> Vec([
    0,
    0,
    1,
    0,
    0,
    0,
    1,
    0,
    1,
    0,
    1,
    0,
    1,
    0,
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    1,
    0,
    1,
    0,
    0,
    0,
    0,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    ((X0 * X1 * X2 * X3 * X4 * X5 * X6 * X7 * X8 * X9 * X10 * X11 * X12 * X13 * X14 * X15 * X16 * X17 * X18 * X19 * X20 * X21 * X22 * X23 * X24 * X25 * X26 * X27 * X28 * X29 * X30 * X31 * X32 * X33 * X34 * X35 * X36 * X37 * X38 * X39 * X40 * X41 * X42 * X43 * X44 * X45 * X46 * X47 * X48 * X49 * X50 * X51 * X52 * X53 * X54 * X55 * X56 * X57 * X58 * X59 * X60 * X61 * X62) + (X0 * X1 * X2 * X3 * X4 * X5 * X6 * X7 * X8 * X9 * X10 * X11 * X12 * X13 * X14 * X15 * X16 * X17 * X18 * X19 * X20 * X21 * X22 * X23 * X24 * X25 * X26 * X27 * X28 * X29 * X30 * X31 * X32 * X33 * X34 * X35 * X36 * X37 * X38 * X39 * X40 * X41 * X42 * X43 * X44 * X45 * X46 * X47 * X48 * X49 * X50 * X51 * X52 * X53 * X54 * X55 * X56 * X57 * X58 * X59 * X60 * X61 * X62 * X63))

What we can observe is that every output bit is a constant except for the last
one. According to the values we computed earlier, this last bit seems to be mostly zero:

.. code:: python

   print(45142941144388932 & (1<<63))
   >>> 0

We thus can use the :ref:`boolean expression solver <sec-theory-solver>` to
figure out which values would make this boolean expression true, thanks to the
:meth:`arybo.lib.boolean_expr_solve` function:

.. code:: python

    from arybo.lib import MBA, boolean_expr_solve
    mba = MBA(64)
    x = mba.var('x')
    def f(X):
      T = ((X+1)&(~X))
      C = ((T | 0x7AFAFA697AFAFA69) & 0x80A061440A061440)\
          + ((~T & 0x10401050504) | 0x1010104)
      return C
    r = f(x)
    print(boolean_expr_solve(r[63], x, 1))
    >>> [Vec([
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     0
     ])]

We see that only one value makes this last boolean expression true. Let's
convert it to an integer and test the final result:

.. code:: python

    from arybo.lib import MBA, boolean_expr_solve
    mba = MBA(64)
    x = mba.var('x')
    def f(X):
      T = ((X+1)&(~X))
      C = ((T | 0x7AFAFA697AFAFA69) & 0x80A061440A061440)\
          + ((~T & 0x10401050504) | 0x1010104)
      return C
    r = f(x)
    sols = boolean_expr_solve(r[63], x, 1)
    C0 = sols[0].get_int_be()
    print(hex(C0))
    >>> 0x7fffffffffffffff
    print(hex(f(0)))
    >>> 0xa061440b071544
    print(hex(f(C0)))
    >>> 0x80a061440b071544
