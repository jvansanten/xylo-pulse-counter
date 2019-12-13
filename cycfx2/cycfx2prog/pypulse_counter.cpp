
#include <Python.h>
#include "pulse_counter.cpp"

typedef struct{
	PyObject_HEAD
	PulseCounter *data;
} pycounter;

static void
pycounter_dealloc(pycounter* self){
	delete self->data;
}

static PyObject*
pycounter_new(PyTypeObject* type, PyObject* args, PyObject* kwds){
	pycounter* self;
	
	self = (pycounter*)type->tp_alloc(type, 0);
	if(self){
		try{
			self->data=new PulseCounter();
		}catch(std::exception& ex){
			PyErr_SetString(PyExc_Exception,
			                (std::string("Unable to allocate PulseCounter: ")+ex.what()).c_str());
			return(NULL);
		}catch(...){
			PyErr_SetString(PyExc_Exception, "Unable to allocate PulseCounter");
			return(NULL);
		}
	}
	
	return (PyObject*)self;
}

static PyObject*
pycounter_count(pycounter* self){
	
	count_statistics counts = self->data->count();
	return Py_BuildValue("Oi", Py_BuildValue("iiii", counts.counts[0], counts.counts[1], counts.counts[2], counts.counts[2]), counts.cycles);
}

static PyMethodDef pycounter_methods[] = {
	{"count", (PyCFunction)pycounter_count, METH_NOARGS,
	 "Read out and reset the FPGA scalers\n\n"
	 ":returns: a tuple (counts,ticks), where the `counts` are the number of\n"
	 "          pulses detected on each channel in the last `ticks` clock\n"
	 "          cycles. The clock frequency is nominally 48 MHz, but you\n"
	 "          should probably calibrate it with a function generator"},
	 NULL
};

static PyTypeObject pycounterType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pulse_counter.PulseCounter", /* tp_name */
    sizeof(pycounter),         /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)pycounter_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "Pulse counter driver",    /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    pycounter_methods,         /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    pycounter_new,             /* tp_new */
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "pulse_counter",     /* m_name */
    "Front-end for an FPGA-based pulse counter",  /* m_doc */
    -1,                  /* m_size */
    NULL,                /* m_methods */
    NULL,                /* m_reload */
    NULL,                /* m_traverse */
    NULL,                /* m_clear */
    NULL,                /* m_free */
};
#endif

#if PY_MAJOR_VERSION >= 3
#define MOD_DEF(ob, name, doc, methods) \
    static struct PyModuleDef moduledef = { \
        PyModuleDef_HEAD_INIT, name, doc, -1, methods, }; \
    ob = PyModule_Create(&moduledef);
#else
#define MOD_DEF(ob, name, doc, methods) \
    ob = Py_InitModule3(name, methods, doc);
#endif

static PyObject *
moduleinit(void)
{
    PyObject *module;

    if (PyType_Ready(&pycounterType) < 0)
        return NULL;
    MOD_DEF(module, "pulse_counter",
            "Front-end for an FPGA-based pulse counter",
            NULL)

    if (module == NULL)
        return NULL;

    Py_INCREF(&pycounterType);
    PyModule_AddObject(module, "PulseCounter", (PyObject*)&pycounterType);

    usb_init();

    return module;
}

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initpulse_counter(void)
{
    moduleinit();
}
#else
PyMODINIT_FUNC PyInit_pulse_counter(void)
{
    return moduleinit();
}
#endif
