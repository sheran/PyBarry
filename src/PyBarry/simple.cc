///
/// \file	simple.cc
///		Python module for performing basic commands on the BlackBerry
///     Usage: from PyBarry import simple
///

/*
    Copyright (C) 2010, ZenConsult Pte. Ltd. (http://www.zenconsult.net/)
    Copyright (C) 2010, Sheran Gunasekera

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the GNU General Public License in the COPYING file at the
    root directory of this project for more details.
*/

#include <Python.h>
#include <barry/barry.h>
#include "barry/m_javaloader.h"
#include "barry/protocol.h"

using namespace Barry;
using namespace std;

PyObject *
GetPIN(PyObject * self, PyObject * args) 
{
	PyObject * result;
	
	try {
		Barry::Init(false);
		Barry::Probe probe;
		if(probe.GetFailCount()) {
			printf("%s","Errors occurred:\n");
			for(int a=0; a < probe.GetFailCount(); a++) {
				std::string perr = probe.GetFailMsg(a);
				perr += "\n";
				printf("%s",perr.c_str());
			}
		}
		if(probe.FindActive() == -1 ) {
			printf("%s","No active devices found.  Connect a BlackBerry via USB and re-run.\n");
			return Py_None;
		}
		const ProbeResult &dev = probe.Get(0);
		result = Py_BuildValue("i",dev.m_pin);
		
	} catch(std::exception &e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
		return Py_None;
	}
	return result;
	Py_DECREF(result);
}




PyMethodDef methods[] = {
  {"GetPIN",GetPIN, METH_VARARGS}
};

extern "C"
void initsimple()
{
    (void)Py_InitModule("simple", methods);
}
