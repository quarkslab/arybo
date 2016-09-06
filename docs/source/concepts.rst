.. _sec-theory:

Concepts
========

This section describes some of the theory used in arybo/petanque, the choices
that have been made and the various limitations that exist.

This is basically a brief summary of this paper. (TODO)

.. _sec-theory-anf:

Representation of boolean expressions and bit-vectors
-----------------------------------------------------

Boolean expressions are represented in the petanque library using the Algebric
Normal Form (ANF). The advantages are that it is complete (that is any boolean
expression can be represented using only XOR and AND operations) and it has an
algebraic structure (see :ref:`sec-theory-app`).

The ANF form is a series of AND operations xored together, with a final
potential binary XOR with 1. So, the expression tree has an addition as root,
and its children are only AND operators, and optionally one 1 in the end. For
instance, :math:`(x \land a) \oplus (x \land b) \oplus 1` is an ANF, whereas
:math:`x \land (a \oplus b) \oplus 1` is not. On the last one, the AND
operation must be expand to get the ANF version. That being said, the petanque
library is able to consider any boolean expression with OR, AND, XOR and
:ref:`ESF <sec-theory-esf>` operators (with any depth), and can compute the ANF
form of these expressions.

The petanque library considers expressions modulo 2, and is then using
additions and multiplications to respectively represent binary XOR and AND.
Here are some examples using the ``iarybo`` shell:

.. code::

    $ iarybo 1
    In [1]: x^y
    Out[1]: 
    Vec([
    (x0 + y0)
    ])

    In [2]: x&y
    Out[2]: 
    Vec([
    (x0 * y0)
    ])

    In [3]: x|y
    Out[3]: 
    Vec([
    ((x0 * y0) + x0 + y0)
    ])

What is called in the various API "simplify" is the process of expanding any supported
boolean expression into an ANF. For instance, one could create this expression:

.. code:: python

   from arybo.lib import MBA
   mba = MBA(1)
   x = mba.var('x')
   y = mba.var('y')
   z = mba.var('z')
   # (x XOR y) AND z
   e = (x[0]+y[0])*z[0]
   >>> print(e)
   ((x0 + y0) * a0)

Using :meth:`arybo.lib.MBAVariable.simplify` will transform this expression
into its ANF form:

.. code:: python

   from arybo.lib import simplify
   e_s = simplify(e)
   >>> print(e_s)
   ((x0 * a0) + (y0 * a0))

.. _sec-theory-esf:

Elementary symmetric functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Elementary symmetric functions (ESF) are functions whose outputs do not depend
on the order of their input boolean variables, which means their output values
only depend on the Hamming weight of the input vector. An elementary symmetric
function of degree :math:`d` with :math:`k` input variables is a boolean
function defined as:

.. math::

	esf_d:~ \mathbb{F}_2^k & \longrightarrow\mathbb{F}_2^k\\
	(x_1, \dots, x_k)      & \longmapsto \bigoplus_{1\le  j_1 < j_2 < \ldots < j_d \le k} x_{j_1} \dotsm x_{j_d}

Some examples:

.. math::

	esf_1(a,b,c) &= a \oplus \oplus b \oplus c\\
	esf_2(a,b) &= ab \\
	esf_2(a,b,c) &= ab \oplus ac \oplus bc

They are interesting because they occur "naturally" in many parts of arithmetic
operators expressed at their boolean levels (see an example with the addition below).

Moreover, we can prove that:

.. math::

	x_1 \lor \dotsb \lor x_n = \bigoplus\limits_{d=1}^{n} esf_d(x_1,\dotsc,x_n).

This can be useful to identify OR operations within an ANF expression, as we
can see in this example (OR identification is still experimental, and thus
needs to be explicitly imported from pytanque):

.. code::

   $ iarybo 1
   In [1]: e=a|b|c|d

   In [2]: e
   Out[2]: 
   Vec([
   ((a0 * b0) + (a0 * c0) + (a0 * d0) + (b0 * c0) + (b0 * d0) + (c0 * d0) + (a0 * b0 * c0) + (a0 * b0 * d0) + (a0 * c0 * d0) + (b0 * c0 * d0) + (a0 * b0 * c0 * d0) + a0 + b0 + c0 + d0)
   ])

   In [3]: find_esfs(e[0])
   Out[3]: [ESF(3, a0, b0, c0, d0), ESF(2, a0, b0, c0, d0)]

   In [4]: e
   Out[4]: 
   Vec([
   (ESF(2, a0, b0, c0, d0) + ESF(3, a0, b0, c0, d0) + (a0 * b0 * c0 * d0) + a0 + b0 + c0 + d0)
   ])

   In [5]: import pytanque

   In [6]: pytanque.identify_ors_inplace(e[0])
   Out[6]: True

   In [7]: e
   Out[7]: 
   Vec([
   (a0 | b0 | c0 | d0)
   ])

.. _sec-theory-solver:

Boolean expression solver
~~~~~~~~~~~~~~~~~~~~~~~~~

A naive boolean expression solver has been implemented. It basically takes as
input a boolean expression containing a given number of symbolic values, and
produces (potentially symbolic) bit-vectors that make the boolean expression
true or false (according to the user's demand).

A usage example is described :ref:`here <sec-tutorial-dirac>`.

Integer arithmetic operations
-----------------------------

Addition/substraction
~~~~~~~~~~~~~~~~~~~~~

Addition is computed symbolically using the algorithm behind a 1-bit logical
adder. Basically, for an n-bit addition, :math:`n-1` carry bits are computed
one after the other, according to the previous results. More formally, :math:`R
= x+y` is computed like this (with :math:`R`, :math:`x` and :math:`y` n-bit
variables):

.. math::
    :nowrap:

    \begin{eqnarray}
    &R_i = x_i \oplus y_i \oplus c_i\\
    &\text{with }
    \begin{cases}
    c_0 = 0\\
    c_{i+1} = x_i \cdot y_i \oplus c_i \cdot (x_i \oplus y_i)\\
    \end{cases} \label{eq:carry}
    \end{eqnarray}

Using the ESF described above, :math:`c_i` can be rewritten as this:

.. math::

    c_{i+1} = esf_2(x_i, y_i, c_i)

An optimization can be done if :math:`y` is a constant known at runtime. It uses the fact that:

.. math::

    x+y = (x \oplus y) + ((x \land y) \ll 1)


By applying recursively this formula and because :math:`x+0 = x`, we can write
the following recursive algorithm:

.. code:: python

    def add(x,y):
      if (y == 0): return x
      return add(x^y, (x&y)<<1)

For instance, if

.. math::

    y = (0 \dots 0~1)^\intercal
   
then the addition will be reduced to only one XOR in one loop iteration, while
the original algorithm would have gone through the computation of every carry
bit.

Multiplication
~~~~~~~~~~~~~~

The multiplication is using the fact that:

.. math::

	x \times y &= x \times (\sum\limits_{i=0}^n 2^{i}y_{i}) \\
        &= \sum\limits_{i=0}^n x\times 2^{i}y_{i} \\
		&= \sum\limits_{i=0}^n (x \ll i) \times y_{i}

An n-bit multiplication is thus performed using :math:`n` multiplication.

Division by a known constant
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Only a division by a known constant at runtime is supported in Arybo for
the moment. The main idea is to transform a division by a :math:`n`-bit constant into
a multiplication by a :math:`2n`-bit constant and a right logical shift.

The details of the complete algorithm are in the `Hacker's Delight
<http://www.hackersdelight.org/>`_ book. It also can be found in some
optimization libraries, for instance in `libdivide
<http://www.libdivide.org>`_.

.. _sec-theory-app:

Application
-----------

Applications are functions that take a :math:`m`-bit vector as input and
produce an `n`-bit vector. They are represented within petanque in two parts:

 * a non-linear part called `NL`
 * an affine part composed of a :math:`m*n` matrix `M` and a constant vector `V`

This construction is possible because of the ANF form.

In petanque, a process called "vectorial decomposition" allows the creation
of such application from a bit-vector and list of symbolic inputs to consider.
Here is an example that creates the application associated with the operation
:math:`x+1`, for a 4-bit input :math:`x`:

.. code:: python

    from mba.lib import MBA
    mba = MBA(4)
    x = mba.var('x')
    r = x+1
    F = x.vectorial_decomp([x])
    >>> print(F)
    App NL = Vec([
    0,
    0,
    (_0 * _1),
    (_0 * _1 * _2)
    ])
    AffApp matrix = Mat([
    [1, 0, 0, 0]
    [1, 1, 0, 0]
    [0, 0, 1, 0]
    [0, 0, 0, 1]
    ])
    AffApp cst = Vec([
    1,
    0,
    0,
    0
    ])

Inverse of an application
~~~~~~~~~~~~~~~~~~~~~~~~~

Arybo is able to inverse a subset of the invertible applications, without
computing the whole truth table and inverting it (which can be really memory
and computation intensive for application over 32-bits input for instance).

The two kinds of invertible application Arybo is able to invert are:

 * affine/linear application with an invertible `M` matrix
 * application with a non-linear part which is a `T function
   <http://link.springer.com/chapter/10.1007%2F3-540-36400-5_34>`_. Every
   arithmetic operation supported by Arybo falls into that category. The main
   idea is to resolve the non-linear system using a classical substitution
   technic.

The example above is a good candidate:

.. code:: python
    
    from mba.lib import MBA, app_inverse, simplify
    mba = MBA(4)
    x = mba.var('x')
    r = x+1
    F = x.vectorial_decomp([x])
    Finv = app_inverse(F)
    >>> print(simplify(F(Finv(x.vec))))
    Vec([
    x0,
    x1,
    x2,
    x3
    ])

A random permutation is a good example of an application Arybo can't invert
(yet). Indeed, chances are very low to fall into one of the two categories
mentioned above:

.. code:: python

    import random
    from mba.lib import MBA, app_inverse, simplify
    mba = MBA(4)
    P = list(range(16))
    random.shuffle(P)
    E,X = mba.permut2expr(P)
    F = E.vectorial_decomp([X])
    >>> print(F)
    App NL = Vec([
    ((_0 * _1) + (_0 * _2) + (_1 * _2)),
    ((_0 * _1) + (_0 * _3) + (_2 * _3) + (_0 * _1 * _2) + (_0 * _1 * _3) + (_0 * _2 * _3)),
    ((_0 * _1) + (_0 * _2) + (_1 * _2) + (_1 * _3) + (_0 * _2 * _3)),
    ((_1 * _3) + (_2 * _3) + (_1 * _2 * _3))
    ])
    AffApp matrix = Mat([
    [0, 1, 1, 0]
    [0, 1, 1, 1]
    [1, 1, 0, 1]
    [1, 1, 1, 0]
    ])
    AffApp cst = Vec([
    0,
    0,
    1,
    1
    ])
    >>> print(app_inverse(F))
    None
   

What could be improved
----------------------

* test different ways of storing boolean expressions within petanque. We are
  currently using a "sorted vector" (that is a vector whose elements are always
  sorted), which has the advantage of consuming less memory than a tree but has
  a more important cost when inserting and removing elements (as we need to
  move the other elements each time).

* implement a C++ version of the Arybo library (while keeping the Python
  version for testing/fallback purposes), so that it could be used natively in
  other libraries, or in other languages through various bindings

Interesting idea/algorithms to implement
----------------------------------------

* the algorithm described by Alex Biryukov, Christophe De Cannière, An Braeken
  and Bart Preneel in `this paper
  <http://www.iacr.org/cryptodb/archive/2003/EUROCRYPT/2059/2059.ps>`_ that
  allows to find, for two arbitrary permutations :math:`S_1` and :math:`S_2`,
  two invertible linear functions :math:`L_1` and :math:`L_2` such as
  :math:`S_2 = L_1 \circ S_1 \circ S_2`.

* find interesting equalities involving ESF that would make the
  canonicalisation of some MBA much faster and less memory-consuming (as we
  would simplify ESFs directly, without expanding them)
