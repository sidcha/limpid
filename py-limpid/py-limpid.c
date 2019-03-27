/******************************************************************************

                  Copyright (c) 2017 Siddharth Chandrasekaran

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

    Author : Siddharth Chandrasekaran
    Email  : siddharth@embedjournal.com
    Date   : Sun Sep 23 22:12:43 IST 2018

******************************************************************************/

#include <Python.h>
#include <limpid/cli.h>

static PyObject* limpid_send(PyObject *self, PyObject *args)
{
    PyObject *resp;
    char *limpid_trig, *limpid_args, *limpid_resp;

    if (!PyArg_ParseTuple(args, "ss", &limpid_trig, &limpid_args))
        return NULL;

    if (limpid_send_cli_cmd(limpid_trig, limpid_args, &limpid_resp) != 0)
        Py_RETURN_NONE;

    resp = Py_BuildValue("s", limpid_resp);
    free(limpid_resp);

    return resp;
}

#if PY_MAJOR_VERSION < 3
static char limpid_send_docs[] = "limpid.send(String limpid_tigger, String cli_args)\n";

static PyMethodDef limpid_funcs[] = {
    {"send", (PyCFunction)limpid_send, METH_VARARGS, limpid_send_docs},
    {NULL}
};

void initlimpid(void)
{
    Py_InitModule3("limpid", limpid_funcs, "Limpid CLI python extension!");
}
#else
static PyMethodDef limpid_methods[] = {
    {
        "send",
        (PyCFunction)limpid_send,
        METH_VARARGS,
        "limpid.send(String limpid_tigger, String cli_args)"
    },
    {NULL, NULL, 0, NULL} /* sentinel */
};

static struct PyModuleDef limpid_module = {
    PyModuleDef_HEAD_INIT,
    "limpid",
    NULL,
    -1,
    limpid_methods
};

PyMODINIT_FUNC PyInit_limpid(void)
{
    return PyModule_Create(&limpid_module);
}
#endif
