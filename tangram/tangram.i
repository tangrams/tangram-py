%module tangram
%{
    #define SWIG_FILE_WITH_INIT
    #include "src/tangram-proxy.h"
%}

%typemap(constcode) int {
  PyObject *val = PyInt_FromLong(($type)($value));
  SWIG_Python_SetConstant(d, "$1", val);
  const char *name = "$typemap(enum_realname,$1_type)";
  PyObject *e = PyDict_GetItemString(d, name);
  if (!e) PyDict_SetItemString(d, name, e = PyDict_New());
  PyDict_SetItemString(e, "$value", val);
}
#define PYTHON_ENUM(x) \
        %typemap(enum_realname) int "x"; \
        %pythoncode %{ \
        x = _tangram.x\
        %} \
        enum x

%include "src/tangram-proxy.h"
