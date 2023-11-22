#include <Python.h>

long sum_list(PyObject *list) {

    int i, n;
    long total = 0;
    PyObject *item;
    n = PyList_Size(list);
    if (n < 0)
        return -1; /* Not a list */ /* Caller should use PyErr_Occurred() if a - 1 is returned. */
    for (i = 0; i < n; i++) { /* PyList_GetItem does not INCREF “item”. "item" is unprotected.*/
        item = PyList_GetItem(list, i); /* Can't fail */
        total += PyLong_AsLong(item);
    }
    Py_DECREF(item); /* BUG! */
    return total;
//    PyObject *item = PyLong_FromLong(1);
//    Py_DECREF(item);
//    return 0;
}