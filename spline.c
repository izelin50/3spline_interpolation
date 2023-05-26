#include <stdio.h>
#include <math.h>
#include <Python.h>
/*#include "numpy/ndarraytypes.h"*/

static double *parse(PyObject *seq)
{
    double *dbar;
    int seqlen;
    int i;

    seq = PySequence_Fast(seq, "argument must be iterable");
    if(!seq)
        return 0;


    seqlen = PySequence_Fast_GET_SIZE(seq);
    dbar = malloc(seqlen*sizeof(double)); /* выделяем блок памяти для списка */

    /* не удалось выделить достаочно памяти */
    if(!dbar) {
        Py_DECREF(seq); /* обнуляем ссылку на объект seq */
        return PyErr_NoMemory();
    }

    for(i=0; i < seqlen; i++) {
        PyObject *fitem;
        PyObject *item = PySequence_Fast_GET_ITEM(seq, i);

        if(!item) {
            Py_DECREF(seq);
            free(dbar);
            return 0;
        }
        fitem = PyNumber_Float(item);
        if(!fitem) {
            Py_DECREF(seq);
            free(dbar);
            PyErr_SetString(PyExc_TypeError, "all items must be numbers");
            return 0;
        }

        dbar[i] = PyFloat_AS_DOUBLE(fitem);
        Py_DECREF(fitem);
    }

    Py_DECREF(seq);

    return dbar;

}


static int get_size(PyObject *seq)
{
    seq = PySequence_Fast(seq, "argument must be iterable");
    return PySequence_Fast_GET_SIZE(seq);
}

void capsule_cleanup(PyObject *capsule) {
    void *memory = PyCapsule_GetPointer(capsule, NULL);
    // Use your specific gc implementation in place of free if you have to
    free(memory);
}
static PyObject *interpolate(PyObject *self, PyObject *args)
{
    PyObject* xSeq;
    PyObject* ySeq;
    PyObject* tSeq;

    if(!PyArg_ParseTuple(args, "OOO", &xSeq,&ySeq,&tSeq)){
        PyErr_SetString(PyExc_TypeError, "argument must be iterable");
        return 0;
    }
   double *x = parse(xSeq);
   double *y = parse(ySeq);
   double *test = parse(tSeq);
   int n=get_size(xSeq);
   int test_size=get_size(tSeq);
   int i; int j;

   /*частичные суммы*/
   double *h = (double**)malloc((n-1) * sizeof(double*));
   double *dy = (double**)malloc((n-1) * sizeof(double*));
   double *h_sums = (double**)malloc((n-2) * sizeof(double*));

   for(i=0; i<n-1; i++)
       h[i]=x[i+1]-x[i];

   for(i=0; i<n-1; i++)
       dy[i]=(y[i+1]-y[i])/h[i];
   for(i=0; i<n-2; i++)
       h_sums[i]=h[i]+h[i+1];

   /*матрица для прогонки*/

   double *f = (double**)malloc((n-2) * sizeof(double*));
   double *a = (double**)malloc(n * sizeof(double*));
   double *b = (double**)malloc(n * sizeof(double*));
   double *c = (double**)malloc(n * sizeof(double*));


    for(i=0; i<n-2; i++)
    {
        if (i!=0)
            f[i-1]=dy[i]-dy[i-1];
        a[i]=h[i];
        b[i]=2*h_sums[i];
        if (i!=n-3)
           c[i]=h[i+1];
    }
    a[0]=c[n-2]=0;


    /*прогонка*/
    double *ta = (double**)malloc((n+1)* sizeof(double*));
    double *tb = (double**)malloc((n+1) * sizeof(double*));
    double *coeff = (double**)malloc(n * sizeof(double*));
    ta[0] = -c[0]/b[0];
    tb[0] = f[0]/b[0];
    for (i=0;i<n-3;i++)
    {
            ta[i+1]=-c[i]/(a[i]*ta[i]+b[i]);
            tb[i+1]=(f[i]-a[i]*tb[i])/(a[i]*ta[i]+b[i]);
    }
    coeff[n-3]=(f[n-3]-a[n-3]*tb[n-3])/(b[n-3]+a[n-3]*ta[n-3]);
    for (i=n-4;i>=0;i--)
        coeff[i]=ta[i+1]*coeff[i+1]+tb[i+1];


    /*подсчет значений для заказанных аргументов*/
    int now = 0;

    double *answer = (double**)malloc(test_size * sizeof(double*));
    for(i=0;i<test_size;i++)
    {        j=0;
        while(test[i]>x[j]+h[j]&& j<n-1) j++;
        printf("%d %lf %lf \n",j,test[i],x[j]+h[j] );
        answer[i]= 1/h[j]*(y[j]*(x[j+1]-test[i])-y[j+1]*(x[j]-test[i]))+
                    coeff[j]/(6*h[j])*(powf(x[j+1]-test[i],3)-powf(h[j],2)*(x[j]-test[i]))+
                    coeff[j+1]/(6*h[j])*(powf(test[i]-x[j],3)-powf(h[j],2)*(test[i]-x[j]));

    }

    free(x);
    free(y);
    free(test);
    free(ta);
    free(tb);
    free(coeff);
    free(a);
    free(b);
    free(c);
    free(f);
    PyObject *my_list = PyList_New(test_size); /* Создает массив объектов типа PyObject длины n, точнее указатель на первый элемент массива длины n*/
    for (i=0; i<test_size; i++)
        PyList_SetItem(my_list, i,  PyFloat_FromDouble(answer[i])); /* Устанавливает элемент в список my_list на индекс i, перед этим  создаем объект PyFloatObject из a[i]*/
   return my_list;
   free(answer);
    /*PyObject *arr;
    int nd = 1;
    int[] dims = {test_size};
    //double *data = some_function_that_returns_a_double_star(x, y, z);

    arr = PyArray_SimpleNewFromData(nd, dims, NPY_DOUBLE, (void *)answer);
    PyObject *capsule = PyCapsule_New(data, NULL, capsule_cleanup);
    // NULL can be a string but use the same string while calling PyCapsule_GetPointer inside capsule_cleanup
    PyArray_SetBaseObject((PyArrayObject *) arr, capsule);
    return arr;*/

}



/*массив С -> список Python*/
PyObject* array_to_list(int* arr, int size) {
    PyObject* list = PyList_New(size); /* Создаем новый пустой список*/
    if (list == NULL) {
        return NULL; /*В случае ошибки выходим из функции*/
    }
    for (int i = 0; i < size; i++) {
        PyObject* item = PyLong_FromLong(arr[i]); /* Создаем новый объект Python на основе текущего элемента массива*/
        if (item == NULL) {
            Py_DECREF(list); /* В случае ошибки освобождаем память, выходим из функции*/
            return NULL;
        }
        PyList_SET_ITEM(list, i, item); /* Добавляем объект в список*/
    }
    return list; /* Возвращаем список*/
}


/* Инициализация модуля */
static PyMethodDef ownmod_methods[] = {

{   "interpolate",
     interpolate,
     METH_VARARGS,
     "interpolation"
}
,
{ NULL, NULL, 0, NULL }

};
static PyModuleDef spline = {
    PyModuleDef_HEAD_INIT,
    "spline",
    "a module for the spline interpolation",
    -1,
    ownmod_methods
};
PyMODINIT_FUNC PyInit_spline(void) {
      PyObject* m;
      m = PyModule_Create(&spline);
      if (m == NULL)
          return NULL;
      return m;
}
