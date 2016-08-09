.. index:: Use-Auto Transform

==================
Use-Auto Transform
==================

The Use-Auto Transform is responsible for using the ``auto`` type specifier for
variable declarations to *improve code readability and maintainability*. The
transform is enabled with the :option:`-use-auto` option of
:program:`clang-modernize`. For example:

.. code-block:: c++

  std::vector<int>::iterator I = my_container.begin();

  // transforms to:

  auto I = my_container.begin();

The ``auto`` type specifier will only be introduced in situations where the
variable type matches the type of the initializer expression. In other words
``auto`` should deduce the same type that was originally spelled in the source.
However, not every situation should be transformed:

.. code-block:: c++

  int val = 42;
  InfoStruct &I = SomeObject.getInfo();

  // Should not become:

  auto val = 42;
  auto &I = SomeObject.getInfo();

In this example using ``auto`` for builtins doesn't improve readability. In
other situations it makes the code less self-documenting impairing readability
and maintainability. As a result, ``auto`` is used only introduced in specific
situations described below.

Iterators
=========

Iterator type specifiers tend to be long and used frequently, especially in
loop constructs. Since the functions generating iterators have a common format,
the type specifier can be replaced without obscuring the meaning of code while 
improving readability and maintainability.

.. code-block:: c++

  for (std::vector<int>::iterator I = my_container.begin(),
                                  E = my_container.end();
       I != E; ++I) {
  }

  // becomes

  for (auto I = my_container.begin(), E = my_container.end(); I != E; ++I) {
  }

The transform will only replace iterator type-specifiers when all of the
following conditions are satisfied:
* The iterator is for one of the standard container in ``std`` namespace:

  * ``array``

  * ``deque``

  * ``forward_list``

  * ``list``

  * ``vector``

  * ``map``

  * ``multimap``

  * ``set``

  * ``multiset``

  * ``unordered_map``

  * ``unordered_multimap``

  * ``unordered_set``

  * ``unordered_multiset``

  * ``queue``

  * ``priority_queue``

  * ``stack``

* The iterator is one of the possible iterator types for standard containers:

  * ``iterator``

  * ``reverse_iterator``

  * ``const_iterator``

  * ``const_reverse_iterator``

* In addition to using iterator types directly, typedefs or other ways of
  referring to those types are also allowed. However, implementation-specific
  types for which a type like ``std::vector<int>::iterator`` is itself a
  typedef will not be transformed. Consider the following examples:

.. code-block:: c++

  // The following direct uses of iterator types will be transformed.
  std::vector<int>::iterator I = MyVec.begin();
  {
    using namespace std;
    list<int>::iterator I = MyList.begin();
  }

  // The type specifier for J would transform to auto since it's a typedef
  // to a standard iterator type.
  typedef std::map<int, std::string>::const_iterator map_iterator;
  map_iterator J = MyMap.begin();

  // The following implementation-specific iterator type for which
  // std::vector<int>::iterator could be a typedef would not be transformed.
  __gnu_cxx::__normal_iterator<int*, std::vector> K = MyVec.begin();

* The initializer for the variable being declared is not a braced initializer
  list. Otherwise, use of ``auto`` would cause the type of the variable to be
  deduced as``std::initializer_list``.

Known Limitations
=================
* If the initializer is an explicit conversion constructor, the transform will
  not replace the type specifier even though it would be safe to do so.
* User-defined iterators are not handled at this time.
