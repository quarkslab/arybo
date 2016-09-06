==============
What is Arybo?
==============

Arybo is a tool that gives a bit-level symbolic representation of expressions
involving various types of operations on bit vectors. Such a tool can be used to
gain a better understanding of complex expressions, for example expressions
that mix both arithmetic and boolean operators. It can also be useful for
optimization purposes, such as proving bit hacks easily.

It has been designed around two main components :
 * petanque: a C++ library for manipulating symbolic boolean expressions,
   designed for performance and a small memory footprint. Python bindings are provided.
 * arybo: a python library to compute symbolic boolean expressions of both
   arithmetic and boolean operations on bit-vector.

See the :ref:`sec-theory` page for more background information, and the
:ref:`sec-tutorials` page for examples.
