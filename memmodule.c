#include <Python.h>
#include "structmember.h"


#include "processclass.c"


static PyModuleDef memoryModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "memory",
    .m_doc = "foreign process memory processing module",
    .m_size = -1,
};


PyMODINIT_FUNC PyInit_memory(void) {

    PyObject *m;
    if (PyType_Ready(&ProcessType) < 0)
        return NULL;

    m = PyModule_Create(&memoryModule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&ProcessType);
    PyModule_AddObject(
        m,
        "Process",
        (PyObject *) &ProcessType
    );

    return m;
}
