#include "pymodule.h"
#include <pybind11/eigen.h>
#include <mymodule/dummy.h>

PYBIND11_PLUGIN(mymodule) {
        py::module m("mymodule", R"pbdoc(
        MY_MODULE
        -----------------------
        .. currentmodule:: mymodule
        .. autosummary::
           :toctree: _generate
           dummy
           dummyM
    )pbdoc");

        // Test functions
        m.def("dummy", &dummy, R"pbdoc(
        Testing function. Add two numbers.
    )pbdoc");
        m.def("dummyM", &dummyM, R"pbdoc(
        Testing function. Add two matrices.
    )pbdoc");

#ifdef VERSION_INFO
        m.attr("__version__") = py::str(VERSION_INFO);
#else
        m.attr("__version__") = py::str("dev");
#endif

        return m.ptr();
}