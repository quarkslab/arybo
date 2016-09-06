=====
Usage
=====

iarybo, interactive IPython shell
---------------------------------

``iarybo`` is a IPython shell that allows to quickly use arybo inside an
interactive shell.

You can launch arybo without any command line argument and get a shell without
any variable specified. You need to use the ``set_mba(N)`` function to declare:
* an ``mba`` object which will be instanciated as ``MBA(N)`` (todo link with the library)
* a bunch of N-bits variables: ``a,b,c,d,x,y``

You can also directly specify a bit number as a command line argument, and
``set_mba(N)`` will be called for you. In some version of IPython, calling back
set_mba(N) does not change the ``mba`` object and associated variables that
have already been declared (it seems to be related to this bug in IPython
https://github.com/ipython/ipython/issues/62, if someone has a workaround for
this, any help is welcome :)).

Example session
~~~~~~~~~~~~~~~

Here's a tiny example session:

.. code:: bash

  $ iarybo 4
  Welcome to Arybo (c) Quarkslab 2014-2016!
  =========================================
  
                           `..----.`                    
                         `.------------.                 
                       `--.-.```````````.`               
                       `  ``  ```````    .+yy-           
                          ::oydmNNNNNmdysmMMMd`          
                         /mNMNdyooymmMMMMMMNNd`          
                      `/omMMNh/`.dhyNMMMMMMNm:           
                .-.   -hddmMNh.-.+sdMMMMMMd+:            
          `.:+sdmNmd/  `-omMMNdyymNMMMMNd+``-`           
      .:+hmNNMMMMMMMm.  :yNNNNNNMMMMNhs:..`-:`           
      `/shMMMMMMNNNMMy/ymmNNNMMMMNd+.```..-:-            
       `+yNNMNNddNMMMMhyssyysyhdds-  `...--.             
     .+yhhhdddhhhdNMMMMNNdosoososyyhyo---.``  ```        
   `:osyhhhhhy:-:+oysdmNNhssssydNNhssyhds.`   ``....`    
    `.-/syhho.     ``-+ssssshmMMMN+y+ohhMs        `.--`  
        `...`         `.:/+dNMMMMMMNmNmNNo``..       -.  
                           `-+hdMMMMMMMMh` ``      `..   
                               .yNMMMMMd-`      ``..     
                                `+mMMMMo.``    `--:.     
                                  .sNMMMNs`  `.:::::-    
                                   .+mMMMMo   -::::--    
                                   `-+NMMMN-  .::--`     
                                    `:mMMMMh  .::.       
                                 ..--+NMMMMm`.o/..`      
                                 `.-+mMMMMMNoyy+         
                                   :ydmMMMMN+`           
                                   `.-:/oos+.            
  
  
  
  These variables have been set for you:
  
    mba = MBA(4)
    x, y, a, b, c, d = 4-bit vars
  
  You can use set_mba(N) to change the bit count of these variables.
  WARNING: on some IPython version, this might not work!
  
  Other exposed functions are:
    - simplify, simplify_inplace, expand_esf, expand_esf_inplace
    - symbol, imm, esf
    - Vector/Matrix
  
  Other exposed modules are:
    - analyses
  
  Report any issues to qbobf@quarkslab.com.
  
  In [1]: x+1
  Out[1]: 
  Vec([
  (x0 + 1),
  (x0 + x1),
  ((x0 * x1) + x2),
  ((x0 * x1 * x2) + x3)
  ])
  
  In [2]: (x+y) - (x&y)*2
  Out[2]: 
  Vec([
  (x0 + y0),
  (x1 + y1),
  (x2 + y2),
  (x3 + y3)
  ])
  

Using the arybo library
-----------------------

Another way to use Arybo is to directly use the provided library.

A classical way to start is to import the :meth:`arybo.lib.MBA` object, and
instantiate one with the number of bits we want to work with:

.. code:: python

    from arybo.lib import MBA
    mba = MBA(8)

From here, we can declare various symbolic variable to work with:

.. code:: python

    from arybo.lib import MBA
    mba = MBA(4)
    x = mba.var('x')
    y = mba.var('y')

And play with them:

.. code:: python

    print(x^y)
    >>> Vec([
    (x0 + y0),
    (x1 + y1),
    (x2 + y2),
    (x3 + y3)
    ])
    print((x+y)-((x&y)<<1))
    >>> Vec([
    (x0 + y0),
    (x1 + y1),
    (x2 + y2),
    (x3 + y3)
    ])

Much more can be done! Have a look at the :ref:`sec-tutorials` page and the
`̀`examples`` directory for more advanced examples, and the :ref:`sec-ref-arybo`
page for the documentation of the Arybo API.
