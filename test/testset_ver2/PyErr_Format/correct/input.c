/*
   Copyright 2011 David Malcolm <dmalcolm@redhat.com>
   Copyright 2011 Red Hat, Inc.

   This is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#include <Python.h>

/*
  Test of correct usage of PyErr_Format
*/

PyObject *
test(PyObject *self, PyObject *args) {
    /* 
       It's more idiomatic to do a "return PyErr_Format".
       However, this approach exercises this case, and verifies that the
       checker doesn't erroneously think that it returns an object:
    */
    PyErr_Format(PyExc_RuntimeError,
                 "This test method always fails");
    return NULL;
}

static PyMethodDef test_methods[] = {
        {"test_method", test, METH_VARARGS, NULL},
        {NULL,          NULL, 0,            NULL} /* Sentinel */
};

/*
  PEP-7
Local variables:
c-basic-offset: 4
indent-tabs-mode: nil
End:
*/
