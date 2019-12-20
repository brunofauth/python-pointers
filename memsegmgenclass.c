#include <stdlib.h>

//process_vm_readv
#include <sys/uio.h>
 
#include <Python.h>
#include "structmember.h"


#ifndef MEMSEGMGENCLASS_C
#define MEMSEGMGENCLASS_C


// This struct holds the object's data to be worked with in C
typedef struct {
    PyObject_HEAD
    long long pid;
    char *haystackBase;
    Py_ssize_t haystackLen;
    Py_buffer * needle;
    char *needleBase;
    Py_ssize_t needleLen;
    Py_buffer * mask;
    char *maskBase;
    Py_ssize_t maskLen;
    char *buffer;
    Py_ssize_t offset;
} MemorySegmentGeneratorObject;


// Keeps track of the class' fields
static PyMemberDef MemorySegmentGenerator_members[] = {
    {
        "pid",
        T_LONGLONG,
        offsetof(MemorySegmentGeneratorObject, pid),
        READONLY,
        "An int, holding the PID of the haystack's owner process."
    },
    {
        "base_addr",
        T_ULONGLONG,
        offsetof(MemorySegmentGeneratorObject, haystackBase),
        READONLY,
        "An int, holding the base address of the haystack."
    },
    {
        "size",
        T_PYSSIZET,
        offsetof(MemorySegmentGeneratorObject, haystackLen),
        READONLY,
        "An int, holding the size, in bytes, of the haystack."
    },
    {
        "needle",
        T_OBJECT_EX,
        offsetof(MemorySegmentGeneratorObject, needle),
        READONLY,
        "A bytes object, holding the pattern to be matched against."
    },
    {
        "mask",
        T_OBJECT_EX,
        offsetof(MemorySegmentGeneratorObject, mask),
        READONLY,
        "A bytes object, which indicates how the data from needle\n"
        "    should be matched against: if 'mask' holds a null\n"
        "    byte, the byte at the same position in needle will\n"
        "    be ignored. Must be of the same length as 'needle'."
    },
    {NULL}  /* Sentinel */
};


// https://docs.python.org/3/extending/newtypes_tutorial.html
// "Unlike the tp_new handler, there is no guarantee that tp_init
// is called at all and it can also be called multiple times."
// Thus we use __new__ for initializing this object, as it is
// supposed to be immutable (missing ref counting tho).
static PyObject * MemorySegmentGenerator_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds
) {
    MemorySegmentGeneratorObject *self;
    self = (MemorySegmentGeneratorObject *) type->tp_alloc(type, 0);
    if (self == NULL)
        return PyErr_NoMemory();

    static char *kwlist[] = {
        "pid",
        "base_addr",
        "size",
        "needle",
        "mask",
        NULL
    };

    // https://docs.python.org/3/c-api/bytes.html#c.PyBytes_AsStringAndSize
    // The bits commented here would've been used had
    // PyBytes_AsStringAndSize not segfaulted.
    int resUnpParams = PyArg_ParseTupleAndKeywords(
        // args, kwds, "Lkny*y*", kwlist,
        args, kwds, "Lkny#y#", kwlist,
        &(self->pid),
        &(self->haystackBase),
        &(self->haystackLen),
        // &(self->needle),
        &(self->needleBase),
        &(self->needleLen),
        // &(self->mask));
        &(self->maskBase),
        &(self->maskLen));
    if (!resUnpParams)
        return NULL;


    // CANT USE THIS BC PyBytes_AsStringAndSize SEGFAULTS
    // int resUnpNeedle = PyBytes_AsStringAndSize(
    //     (PyObject *) self->needle,
    //     &(self->needleBase),
    //     &(self->needleLen));
    // if (resUnpNeedle == -1)
    //     return NULL;

    self->needle = (Py_buffer *) PyBytes_FromStringAndSize(
        self->needleBase,
        self->needleLen);
    if (self->needle == NULL)
        return NULL;

    // CANT USE THIS BC PyBytes_AsStringAndSize SEGFAULTS
    // int resUnpMask = PyBytes_AsStringAndSize(
    //     (PyObject *) self->mask,
    //     &(self->maskBase),
    //     &(self->maskLen));
    // if (resUnpMask == -1)
    //     return NULL;

    self->mask = (Py_buffer *) PyBytes_FromStringAndSize(
        self->maskBase,
        self->maskLen);
    if (self->mask == NULL)
        return NULL;

    if (self->needleLen != self->maskLen) {
        PyErr_SetString(PyExc_ValueError,
            "'needle' and 'mask' must be of the same size.");
        return NULL;
    }

    if (self->needleLen <= 0) {
        PyErr_SetString(PyExc_ValueError,
            "'needle' must not be empty.");
        return NULL;
    }

    if (self->haystackLen < self->needleLen) {
        PyErr_SetString(PyExc_ValueError,
            "'needle' must be smaller than the haystack.");
        return NULL;
    }

    self->buffer = (char *) PyMem_RawMalloc(self->haystackLen);
    if (self->buffer == NULL)
        return PyErr_NoMemory();

    struct iovec l = {(void *) self->buffer, (size_t) self->haystackLen};
    struct iovec r = {self->haystackBase, (size_t) self->haystackLen};
    ssize_t read = process_vm_readv(
        (pid_t) self->pid, &l, 1, &r, 1, 0
    );

    self->offset = 0;

    return (PyObject *) self;
}


// Returns true if two chunks of memory are identical (except for
// masked bytes, which are masked with '\0') and false otherwise.
int memmemmask(char *mem1, char *mem2, char *mask, int len) {
    for (int i = 0; i < len; i++) {
        if (mask[i] != '\0' && mem1[i] != mem2[i])
            return 0;
    }
    return 1;
}


// So that it becomes an iterator
static PyObject * MemorySegmentGenerator_iternext(
    MemorySegmentGeneratorObject *self
) {
    for (; self->offset < self->haystackLen - self->needleLen + 1; self->offset++) {
        if (memmemmask(self->buffer + self->offset, self->needleBase, self->maskBase, self->maskLen)) {
            // Does it leak memory? I don't know!
            PyObject *ret = Py_BuildValue(
                "k", self->haystackBase + self->offset
            );
            self->offset += 1;
            return ret;
        }
    }
    return NULL; // raises StopIteration.
}


// https://docs.python.org/3/extending/newtypes.html#finalization-and-de-allocation
// Maybe there's more to do here
// use tp_finalize if error occur i guess
static void MemorySegmentGenerator_dealloc(
    MemorySegmentGeneratorObject *self
) {
    PyMem_RawFree(self->buffer);
    Py_TYPE(self)->tp_free((PyObject *) self);
}


// Keeps track of the class' methods
// https://docs.python.org/3/c-api/structures.html
static PyMethodDef MemorySegmentGenerator_methods[] = {
    // {name, funcpointer, funcsignature, docstring},
    {NULL}  /* Sentinel */
};


// Defines the class' properties
// https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_alloc
static PyTypeObject MemorySegmentGeneratorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "memory.MemorySegmentGenerator",
    .tp_doc = ""
        "This class describes an iterator object, which copies a segment of\n"
        "    memory from another process and then compares it against a\n"
        "    pattern (needle), taking a mask into account. It, then, lazily,\n"
        "    returns the addresses which match the pattern."
        "\nArgs:\n"
        "    (int) pid: of haystack's owner process.\n"
        "    (int) base_addr: of the haystack to be searched.\n"
        "    (int) size: of the haystack, in bytes.\n"
        "    (bytes) needle: the pattern to be matched against.\n"
        "    (bytes) mask: must have the same length as 'needle'. Null bytes\n"
        "        in this sequence make the corresponding byte from the needle\n"
        "        to be ignored when matching the pattern."
        "\nYields at iteration:\n"
        "    Memory addresses at which the memory matches 'needle', as int.\n"
        "\nRaises:\n"
        "    ValueError: if the sizes of 'needle' and 'mask' aren't the same;\n"
        "        if 'needle' is an empty bytes object or if 'needle' is\n"
        "        bigger than the haystack.\n",
    .tp_basicsize = sizeof(MemorySegmentGeneratorObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = (newfunc) MemorySegmentGenerator_new,
    .tp_iter = PyObject_SelfIter,
    .tp_iternext = (iternextfunc) MemorySegmentGenerator_iternext,
    .tp_dealloc = (destructor) MemorySegmentGenerator_dealloc,
    .tp_members = MemorySegmentGenerator_members,
    .tp_methods = MemorySegmentGenerator_methods
};


#endif

