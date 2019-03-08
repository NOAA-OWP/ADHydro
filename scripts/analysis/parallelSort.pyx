# cython: wraparound = False
# cython: boundscheck = False
import numpy as np
cimport numpy as np
import cython
cimport cython

"""
 Found this: http://stackoverflow.com/questions/27551363/parallel-in-place-sort-for-numpy-arrays/28663374#28663374
"""
ctypedef fused real:
    cython.char
    cython.uchar
    cython.short
    cython.ushort
    cython.int
    cython.uint
    cython.long
    cython.ulong
    cython.longlong
    cython.ulonglong
    cython.float
    cython.double

cdef extern from "<parallel/algorithm>" namespace "__gnu_parallel":
    cdef void sort[T](T first, T last) nogil

def numpyParallelSort(real[:] a):
    "In-Place parallel sort for numpy types"
    #Appears to sort up to a.shape[0]-1, but not sort the final value
    #sort(&a[0], &a[a.shape[0]-1])
    sort(&a[0], &a[a.shape[0]])
