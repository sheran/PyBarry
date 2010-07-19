///
/// \file	core.cc
///		Python module for connecting to a BlackBerry device through USB
///     Usage: from PyBarry import core
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

// Classes

class DataDumpParser : public Barry::Parser
{
	uint32_t m_id;
	PyObject * result;
	PyObject * tempst;
	uint ctr;

public:
	void Init(uint recs)
	{
		result = PyTuple_New((Py_ssize_t)recs);
		tempst = PyDict_New();
		ctr = 0;
	}

	virtual void Clear() {
		tempst = PyDict_New();
	}

	virtual void SetIds(uint8_t RecType, uint32_t UniqueId)
	{
		m_id = UniqueId;
	}

	virtual void ParseHeader(const Data &, size_t &) {}

	virtual void ParseFields(const Barry::Data &data, size_t &offset,
				const IConverter *ic)
	{
		PyObject *record = PyString_FromStringAndSize((const char*)data.GetData(),data.GetSize());
		PyDict_SetItemString(tempst,"id",PyInt_FromLong(m_id));
		PyDict_SetItemString(tempst,"data",record);
	}

	virtual void Store() {
		PyTuple_SET_ITEM(result,ctr,tempst);
		ctr++;
	}

	PyObject * GetString() {
		return result;
	}
};


// Exception

class MyException : public std::runtime_error {
 public:
   MyException() : std::runtime_error("Initialization Failure") { }
 };

// Initialization

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



PyObject *
BulkWrite(PyObject * self, PyObject * args)
{
	return Py_None;
}

PyObject *
GetDBDB(PyObject * self, PyObject * args)
{
	DatabaseDatabase dbdb;
	std::stringstream ss;
	PyObject *element;
	DatabaseItem entry;
	PyObject * result;

	try {
		
		const ProbeResult dev = Initialize();
		Barry::Controller con(dev);
		Barry::Mode::Desktop desktop(con);
		desktop.Open();
		
		dbdb = desktop.GetDBDB();
		std::vector<DatabaseItem> p;
		p = dbdb.Databases;
		result = PyTuple_New(p.size());
		for(uint b = 0; b < p.size(); b++) {
			element = PyDict_New();
			entry = p[b];
			PyObject *keyNu = Py_BuildValue("s","id");
			PyObject *valNu = PyInt_FromLong(entry.Number);
			PyObject *keyRc = Py_BuildValue("s","recs");
			PyObject *valRc = PyInt_FromLong(entry.RecordCount);
			PyDict_SetItem(element,keyNu,valNu);
			PyDict_SetItemString(element,"name",PyString_FromString(entry.Name.c_str()));
			PyDict_SetItem(element,keyRc,valRc);
			PyTuple_SetItem(result,b,element);

		}
		
	} catch(std::exception &e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
		return NULL;
	}
	return result;
}

PyObject *
GetRecs(PyObject * self, PyObject * args)
{
	uint dbId;
	PyObject *result=0;

	
	try {
		if(!PyArg_ParseTuple(args,"i", &dbId))
			return NULL;
		
		const ProbeResult dev = Initialize();
		Barry::Controller con(dev);
		Barry::Mode::Desktop desktop(con);
		desktop.Open();
		uint reccnt = 0;
		DatabaseDatabase dbdb = desktop.GetDBDB();
		std::vector<DatabaseItem> p = dbdb.Databases;
		for(uint q=0; q < p.size(); q++) {
			DatabaseItem entry = p[q];
			if(entry.Number == dbId)
				reccnt = entry.RecordCount;
		}
		DataDumpParser parser;
		parser.Init(reccnt);
		desktop.LoadDatabase(dbId,parser);
		result = parser.GetString();


	
	} catch(std::exception &e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
	}
	return result;
}



PyMethodDef methods[] = {
  {"bulk_write",BulkWrite, METH_VARARGS},
  {"get_dbs",GetDBDB, METH_VARARGS},
  {"get_recs",GetRecs, METH_VARARGS},
  {NULL,0,NULL}
};

extern "C"
void initcore()
{
    (void)Py_InitModule("core", methods);
}
