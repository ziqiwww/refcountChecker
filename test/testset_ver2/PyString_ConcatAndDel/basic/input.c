/*
   Copyright 2012 David Malcolm <dmalcolm@redhat.com>
   Copyright 2012 Red Hat, Inc.

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
  Test coverage for our handler for PyString_ConcatAndDel
*/

PyObject *
test(PyObject **lhs, PyObject *rhs) {
    PyString_ConcatAndDel(lhs, rhs);
    /* 
       There are 3 expected errors:
         - it steals a reference to rhs when it succeeds
         - when rhs is NULL, an exception ought to have been set
         - when it succeeds, *lhs has a refcount of 1, but is referenced in
           two places (return value and *lhs)
    */
    return *lhs;
}

/*
  PEP-7
Local variables:
c-basic-offset: 4
indent-tabs-mode: nil
End:
*/
