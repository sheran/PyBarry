///
/// \file	javaloader.cc
///		Python module for performing JavaLoader tasks
///     Usage: from PyBarry import javaloader
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

using namespace Barry;
using namespace std;

// Exception

class MyException : public std::runtime_error {
 public:
   MyException() : std::runtime_error("Initialization Failure") { }
 };


// Common Init Functions

const ProbeResult 
Initialize()
{
		
		
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
			throw MyException();
		}
		const ProbeResult &dev = probe.Get(0);
		return dev;
}


// GetEventLog() -- Retrieve Event Log from the BlackBerry

static PyObject *
GetEventLog(PyObject *self, PyObject *args)
{
	JLEventlogEntry el;
	JLEventlog log;
	PyObject *result;
	
	static const char *SeverityNames[] = { "Always Log", "Severe Error", "Error",
		"Warning", "Information", "Debug Info"};
	static const char *ViewerTypes[] = { "", "Number", "String", "Exception" };
	
	try {
		
		const ProbeResult dev = Initialize();
		Barry::Controller con(dev);
		Barry::Mode::JavaLoader javaloader(con);
		javaloader.Open();
		javaloader.StartStream();
		javaloader.GetEventlog(log);
		result = PyTuple_New(log.size());
		for(uint k=0; k<log.size();k++) {
			el = log[k];
			PyObject *element = PyDict_New();
			PyDict_SetItemString(element,"guid",PyString_FromString(el.Guid.c_str()));
			PyObject *key = Py_BuildValue("s","time");
			PyObject *val = PyLong_FromUnsignedLongLong((unsigned long long)el.MSTimestamp);
			PyDict_SetItem(element,key,val);
			PyDict_SetItemString(element,"severity",PyString_FromString(SeverityNames[el.Severity]));
			PyDict_SetItemString(element,"type",PyString_FromString(ViewerTypes[el.Type]));
			PyDict_SetItemString(element,"app",PyString_FromString(el.App.c_str()));
			PyDict_SetItemString(element,"data",PyString_FromString(el.Data.c_str()));
			PyTuple_SET_ITEM(result, k, element);
		}
		javaloader.StopStream();
		
	}catch(std::exception &e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
		return NULL;
	}
	return result;
}

// Dir - Get listing of currently installed modules on the handheld.  Does not list sibling modules.
// TODO: Add sibling module listing switch.
static PyObject *
Dir(PyObject * self, PyObject * args)
{
	
	JLDirectory dir;
	PyObject * result;
	JLDirectoryEntry entry(0);
	
	
	try {
		const ProbeResult dev = Initialize();
		Barry::Controller con(dev);
		Barry::Mode::JavaLoader javaloader(con);
		javaloader.Open();
		javaloader.StartStream();
		javaloader.GetDirectory(dir, false);
		result = PyTuple_New(dir.size());
		for(uint k=0; k < dir.size(); k++) {
			PyObject *element = PyDict_New();
			entry = dir[k];
			PyObject *id_val = PyInt_FromSsize_t((unsigned int)entry.Id);
			PyObject *id_key = Py_BuildValue("s","id");
			PyObject *sz_val = PyLong_FromUnsignedLong((unsigned long)entry.CodSize);
			PyObject *sz_key = Py_BuildValue("s","size");
			PyObject *tm_val = PyLong_FromUnsignedLongLong((unsigned long long)entry.Timestamp);
			PyObject *tm_key = Py_BuildValue("s","time");
			PyDict_SetItem(element,id_key,id_val);
			PyDict_SetItem(element,sz_key,sz_val);
			PyDict_SetItem(element,tm_key,tm_val);
			PyDict_SetItemString(element,"name",PyString_FromString(entry.Name.c_str()));
			PyDict_SetItemString(element,"version",PyString_FromString(entry.Version.c_str()));
			PyTuple_SET_ITEM(result, k, element);
			
		}
		javaloader.StopStream();
		
		
	} catch(std::exception &e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
		return NULL;
	}
	return result;
	
	
}


// GetScreenshot - Takes a screenshot of the existing screen display and returns an object.  This object can then be written in
// Python as follows:
//
// bmp = javaloader.get_screenshot()
// f = open('test.bmp','w')
// f.write(bmp)
// f.close()
//

PyObject *
GetScreenshot(PyObject * self, PyObject * args)
{
	PyObject * result;
	try {
		const ProbeResult dev = Initialize();
		Barry::Controller con(dev);
		Barry::Mode::JavaLoader javaloader(con);
		javaloader.Open();
		javaloader.StartStream();
		JLScreenInfo info;
		Data image;
		javaloader.GetScreenshot(info, image);
		Data bitmap(-1, GetTotalBitmapSize(info));
		ScreenshotToBitmap(info, image, bitmap);
		result = Py_BuildValue("s#",bitmap.GetData(),bitmap.GetSize());
		javaloader.StopStream();
	} catch (std::exception &e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
		return NULL;
	}

	return result;
}

// SaveModule - Saves the specified module name into an object.  This can then be either worked on or saved to disk
//
// mod = javaloader.get_module("net_rim_os")
//

PyObject *
SaveModule(PyObject * self, PyObject * args)
{
	ostringstream s;
	const char* mod;
	try{
		if(!PyArg_ParseTuple(args,"s", &mod))
			return NULL;
		const ProbeResult dev = Initialize();
		Barry::Controller con(dev);
		Barry::Mode::JavaLoader javaloader(con);
		javaloader.Open();
		javaloader.StartStream();
		javaloader.Save(mod,s);
		javaloader.StopStream();
	} catch(std::exception &e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
		return NULL;
	}

	return Py_BuildValue("s#",s.str().c_str(),s.str().length());
}

PyMethodDef methods[] = {
  {"get_event_log",GetEventLog, METH_VARARGS},
  {"get_dir", Dir, METH_VARARGS},
  {"get_screenshot", GetScreenshot, METH_VARARGS},
  {"get_module",SaveModule,METH_VARARGS},
  {NULL,0,NULL}
};

extern "C"
void initjavaloader()
{
    (void)Py_InitModule("javaloader", methods);
}
