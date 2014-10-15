DBApi
=====

Database ODBC API


This is written in C++ library that aims to make the connection to the database via ODBC abstracting calls the API pure ODBC, but not limited, since it can return the handlers (Handles) of the objects used by the API pure ODBC.

It is fully object-oriented facilitating the manipulation of data using intuitive methods to perform each action. Has a hierarchical structure which makes each object has its data sets of parameters or fields according to the need. Example: A query can have a set of parameters that contain each parameter and this query can return a set of fields that contain each field, thus facilitating access to information.

As one of the goals of this library is to be used with an extension (Windows DLL), it uses the concept of interfaces to instantiate objects, ie, all classes inherit from a totally abstract class that will only be used to manipulate method calls each particular class.

Considering this condition, no exception is thrown directly, but can be captured simply by a static object that will always hold the last exception that occurred.

However, nothing prevents the objects are compiled incorporated into your application dispensing with an external library for this functionality. In this case, you can use the objects directly.

#Building

The only dependency is the library of class C++ standard STD therefore this library can be portable. It may be because I have not tested on other platforms other than Windows :(.

It can be compiled using Visual Studio or MinGW with gcc for Windows. If anyone wants to collaborate to test the same on a Linux or Unix environment, I am grateful and I am willing to help with anything you need :).

This library allows manipulation and memory allocation for query fields by default, automatic, or can be controlled by the application through memory pointers to the variables that will receive the field values.
