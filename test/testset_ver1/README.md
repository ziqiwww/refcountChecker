### TEST1

- reference：scipy ；scipy/sparse/linalg/_dsolve/_superlu_utils.c

- link：https://github.com/scipy/scipy/issues/16232
- 当46行obj申请成功时，在53行调用“借用”类型的API PyDict_SetItemString不会对其引用计数造成影响，但返回时却未减少引用计数

### TEST2

- reference: numpy; numpy/f2py/src/fortranobject.c
- link: https://github.com/numpy/numpy/issues/19573
- 30,373,379,450行均使用了PyDict_SetItemString “借用”类型的API，但在函数返回时未对借用的参数引用计数decrease

### TEST3,4,5,6,7

- reference：numpy
    - numpy/numpy/core/src/multiarray/calculation.c
    - numpy/numpy/core/src/multiarray/descriptor.c
    - numpy/numpy/core/src/multiarray/getset.c
    - numpy/numpy/core/src/multiarray/methods.c
    - numpy/numpy/core/src/umath/umathmodule.c
- link：https://github.com/numpy/numpy/issues/21532
- 对于新申请的变量引用计数，在后续执行流中因错误分支直接返回NULL时没有对其引用计数decrease
    - 3.c
        - 370行分支为true，新申请的变量引用计数将可能在379行泄露
        - 390行分支为true，新申请的变量引用计数将可能在410行泄露
    - 4.c
        - 424行新申请的变量可能在435行返回NULL时未减少引用计数
        - 2707行新申请的变量可能在2714行返回NULL时未减少引用计数
        - 2716行增加的引用计数在2756行返回NULL时未减少引用计数
        - 2810行新申请的变量可能在2827行返回NULL时未减少引用计数
    - 5.c
        - 648行新申请的变量可能在在656行返回NULL时未减少引用计数
    - 6.c
        - 2081行新申请的变量可能在在2145,2123行返回NULL时未减少引用计数
    - 7.c
        - 298、299、300、301、302行使用的API PyModule_AddObject返回值没有做检查，当失败时期第三个参数会引用计数泄露