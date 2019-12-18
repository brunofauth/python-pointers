// https://docs.python.org/3/extending/newtypes_tutorial.html

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif

//process_vm_read and process_vm_write
#include <sys/uio.h>

#include <Python.h>
#include "structmember.h"
#include <stdio.h>


#include "memmemmask.c"


// Declares the class' fields' C representation (storage)
typedef struct {
    PyObject_HEAD
    int pid;
} ProcessObject;


// Keeps track of the class' fields
static PyMemberDef Process_members[] = {
    {
        "pid",
        T_UINT,
        offsetof(ProcessObject, pid),
        0,
        "The ID of the process we're working with."
    },
    {NULL}  /* Sentinel */
};


// Custom init for the class
static int Process_init(
    ProcessObject *self,
    PyObject *args,
    PyObject *kwds
) {
    static char *kwlist[] = {"pid", NULL}; 
    return !PyArg_ParseTupleAndKeywords(
        args, kwds, "I", kwlist, &self->pid
    );
}


// Class' "read" method
static PyObject *Process_read(
    ProcessObject *self,
    PyObject *args
) {
    void *base;
    int size;
    ssize_t read;
    
    int success = PyArg_ParseTuple(args, "Ki",
        &base, &size
    );

    if (!success)
        return NULL;
    
    char *buffer = (char *) malloc(size);

    struct iovec l = {(void *) buffer, (size_t) size};
    struct iovec r = {base, (size_t) size};
    read = process_vm_readv(
        (pid_t) self->pid, &l, 1, &r, 1, 0
    );

    if (read == -1) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    // Possibly leaks memory, i guess...
    PyObject *ret = Py_BuildValue(
        "ny#", (Py_ssize_t) read, buffer, size
    );

    free(buffer);
    return ret;
}


// Class' "write" method
static PyObject *Process_write(
    ProcessObject *self,
    PyObject *args
) {
    void *base;
    const char *buffer_ptr;
    int size;
    ssize_t written;
    
    int success = PyArg_ParseTuple(args, "Ky#",
        &base, &buffer_ptr, &size
    );

    if (!success)
        return NULL;

    struct iovec l = {buffer_ptr, (ssize_t) size};
    struct iovec r = {base, (ssize_t) size};
    written = process_vm_writev(
        (pid_t) self->pid, &l, 1, &r, 1, 0
    );

    if (written == -1) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    // Possibly leaks memory, i guess...
    return Py_BuildValue("n", (Py_ssize_t) written);
}

/*self, base: int, size: int, needle: bytes, mask: bytes*/
static PyObject *Process_scan(
    ProcessObject *self,
    PyObject *args
) {
    void *base, *match;
    char *needle, *mask;
    int hLen, nLen, mLen;

    int success = PyArg_ParseTuple(args, "Kiy#y#",
        &base, &hLen, &needle, &nLen, &mask, &mLen
    );

    if (!success)
        return NULL;

    if (nLen != mLen) {
        PyErr_SetString(PyExc_ValueError,
            "'needle' and 'mask' must be of the same size."
        );
        return NULL;
    }

    if (nLen <= 0) {
        PyErr_SetString(PyExc_ValueError,
            "'needle' must not be empty."
        );
        return NULL;
    }

    if (hLen < nLen) {
        PyErr_SetString(PyExc_ValueError,
            "'needle' must be smaller than the haystack."
        );
        return NULL;
    }

    match = (void *) memmemmask(
        base, hLen, needle, mask, nLen
    );

    if (match == NULL) {
        PyErr_SetString(PyExc_LookupError,
            "NEEDLE not found."
        );
        return NULL;
    }

    return PyLong_FromVoidPtr(match);
}


/*
This is an example of Google style.

Args:
    param1: This is the first param.
    param2: This is a second param.

Returns:
    This is a description of what is returned.

Raises:
    KeyError: Raises an exception.
*/


// Keeps track of the class' methods
// https://docs.python.org/3/c-api/structures.html
static PyMethodDef Process_methods[] = {
    {
        "read",
        (PyCFunction) Process_read,
        METH_VARARGS,
        "Reads bytes from the process and returns them.\n"
        "\nArgs:\n"
        "    (int) base_addr: of the chunk of memory to be read.\n"
        "    (int) size: of the chunk of memory to be read, in bytes.\n"
        "\nReturns:\n"
        "    A tuple, containing the number of bytes read, as an int, and a\n"
        "    bytes object, which holds the actual data read.\n"
        "\nRaises:\n"
        "    OSError: raised if the operation fails."
    },
    {
        "write",
        (PyCFunction) Process_write,
        METH_VARARGS,
        "Writes bytes to the memory of the process.\n"
        "\nArgs:\n"
        "    (int) base_addr: of the chunk of memory to be overwritten.\n"
        "    (bytes) data: which will be written.\n"
        "\nReturns:\n"
        "    The number of bytes written, as an int.\n"
        "\nRaises:\n"
        "    OSError: Raised if the operation fails."
    },
    {
        "scan",
        (PyCFunction) Process_scan,
        METH_VARARGS,
        "Searches an address range of the process' memory space for a pattern\n"
        "and returns the address corresponding to the beggining of the match.\n"
        "\nArgs:\n"
        "    (int) base_addr: of the chunk of memory to be searched (haystack).\n"
        "    (int) size: of the chunk of memory to be searched, in bytes.\n"
        "    (bytes) needle: the pattern to be matched against.\n"
        "    (bytes) mask: this parameter tells how the bytes from 'needle'\n"
        "        should be matched. Null bytes here make bytes on the\n"
        "        corresponding position on 'needle' be ignored, while any\n"
        "        other values make them be compared as usual (==). 'mask' and\n"
        "        'needle' must have the same length.\n"
        "\nReturns:\n"
        "    The memory address at which the memory matches 'needle', as int.\n"
        "\nRaises:\n"
        "    ValueError: if the sizes of 'needle' and 'mask' aren't the same;\n"
        "        if 'needle' is an empty bytes object or if 'needle' is\n"
        "        bigger than the haystack.\n"
        "    LookupError: if a match is not found."
    },
    {NULL}  /* Sentinel */
};


// Defines the class' properties
static PyTypeObject ProcessType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "memory.Process",
    .tp_doc = "Process memory manipulation wrapper",
    .tp_basicsize = sizeof(ProcessObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) Process_init,
    .tp_members = Process_members,
    .tp_methods = Process_methods // ,
    //.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE
};
