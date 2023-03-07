#define PY_SSIZE_T_CLEAN
#include <Python.h>
// https://docs.python.org/zh-cn/3/extending/extending.html
/**
 * 由于 Python
可能会定义一些能在某些系统上影响标准头文件的预处理器定义，因此在包含任何标准头文件之前，你
必须 先包含 Python.h。 推荐总是在 Python.h 前定义 PY_SSIZE_T_CLEAN 。查看
提取扩展函数的参数 来了解这个宏的更多内容。
*/

static PyObject *spam_system(PyObject *self, PyObject *args);

static PyMethodDef SpamMethods[] = {
    {"system", spam_system, METH_VARARGS,
     /*如果单独使用 METH_VARARGS ，函数会等待Python传来tuple格式的参数，
     并最终使用 PyArg_ParseTuple() 进行解析。*/
     /*METH_KEYWORDS 值表示接受关键字参数。这种情况下C函数需要接受第三个 PyObject * 对象，
     表示字典参数，使用 PyArg_ParseTupleAndKeywords() 来解析出参数。*/
     "Execute a shell command."},
    {NULL, NULL, 0, NULL} /*Sentinel*/
};

static PyModuleDef spammodule = {
    PyModuleDef_HEAD_INIT,
    "spam",                      /*module name*/
    "Python interface for spam", /*module document*/
    -1,                          /* size of per-interpreter state of the module,
                                          or -1 if the module keeps state in global variables. */
    SpamMethods};

static PyObject *Spam_Error;

// constructor for the model
PyMODINIT_FUNC
PyInit_spam() { // the init function must be of format PyInit_<module name> (not file name)
    PyObject *module = PyModule_Create(&spammodule);
    if (module == NULL)
        return NULL;
    Spam_Error = PyErr_NewException("spam.error", NULL,
                                    NULL); //异常对象的Python名字是 spam.error
    Py_XINCREF(Spam_Error);                // add a new error object
    if (PyModule_AddObject(module, "error", Spam_Error) < 0) {
        Py_XDECREF(Spam_Error);
        Py_CLEAR(Spam_Error);
        Py_DECREF(module);
        return NULL;
    }
    // Py_RETURN_NONE if void
    return module;
}

static PyObject *spam_system(PyObject *self, PyObject *args) {
    /**
     * 有个直接翻译参数列表的方法(举个例子，单独的 "ls -l"
     * )到要传递给C函数的参数。C函数总是有两个参数，通常名字是 self 和 args 。
     */
    const char *command;
    int sts;
    /**
     * args 参数是指向一个 Python 的 tuple 对象的指针，其中包含参数。
     * 每个 tuple 项对应一个调用参数。 这些参数也全都是 Python 对象 --- 要在我们的
     * C 函数中使用它们就需要先将其转换为 C 值。 Python API 中的函数
     * PyArg_ParseTuple() 会检查参数类型并将其转换为 C 值。
     * 它使用模板字符串确定需要的参数类型以及存储被转换的值的 C 变量类型。
     * 细节将稍后说明。
     */
    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;

    printf("%s\n", command);
    sts = system(command);
    if (sts < 0) {
        // throw error
        PyErr_SetString(Spam_Error, "System command failed");
        return NULL;
    }
    return PyLong_FromLong(sts);
}
