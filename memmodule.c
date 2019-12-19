#include <Python.h>
#include "structmember.h"

#include "processclass.c"
#include "memsegmgenclass.c"


#ifndef MEMMODULE_C
#define MEMMODULE_C


static PyModuleDef memoryModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "memory",
    .m_doc = "foreign process memory processing module",
    .m_size = -1,
};


PyMODINIT_FUNC PyInit_memory(void) {

    if (PyType_Ready(&ProcessType) < 0)
        return NULL;

    if (PyType_Ready(&MemorySegmentGeneratorType) < 0)
        return NULL;

    PyObject *m = PyModule_Create(&memoryModule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&ProcessType);
    PyModule_AddObject(
        m,
        "Process",
        (PyObject *) &ProcessType
    );

    Py_INCREF(&MemorySegmentGeneratorType);
    PyModule_AddObject(
        m,
        "MemorySegmentGenerator",
        (PyObject *) &MemorySegmentGeneratorType
    );

    return m;
}


#endif

