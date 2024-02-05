%module (package="pyhvac") irhvac
#define SWIGPYTHON_BUILTIN
%include <std_string.i>
%include "stdint.i"
%{
    #include <vector>
    #include "IRac.h"
    #include "IRremoteESP8266.h"
    #include "IRsend.h"
%}
%include <std_vector.i>
%template(VectorOfInt) std::vector<int>;
%typemap(out) std::vector<int> (PyObject* _outer) %{
    // Allocate a PyList object of the requested size.
    _outer = PyList_New($1.size());
    // Populate the PyList.  PyLong_FromLong converts a C++ "long" to a
    // Python PyLong object.
    for(int x = 0; x < $1.size(); x++) {
        PyList_SetItem(_outer,x,PyLong_FromLong($1[x]));
    }
   $result = SWIG_Python_AppendOutput($result,_outer);
%}
%include <IRsend.h>
%typemap(out) std::vector<int> (PyObject* _outer) %{
    // Allocate a PyList object of the requested size.
    _outer = PyList_New($1.size());
    // Populate the PyList.  PyLong_FromLong converts a C++ "long" to a
    // Python PyLong object.
    for(int x = 0; x < $1.size(); x++) {
        PyList_SetItem(_outer,x,PyLong_FromLong($1[x]));
    }
   $result = SWIG_Python_AppendOutput($result,_outer);
   %}
%include <IRremoteESP8266.h>
%include <IRac.h>

