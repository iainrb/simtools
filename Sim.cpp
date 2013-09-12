// Sim.cpp
//
// Author: Jennifer Liddle (js10)
//
// $Id: Sim.cpp 1354 2010-11-11 16:20:09Z js10 $
//
// Copyright (c) 2009 - 2010 Genome Research Ltd. 
//
// Author: Jennifer Liddle <js10@sanger.ac.uk, jennifer@jsquared.co.uk>
//
// Redistribution and use in source and binary forms, with or without modification, 
// are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, this 
// list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation and/or 
// other materials provided with the distribution.
// 3. Neither the name of the Genome Research Ltd nor the names of its contributors 
// may be used to endorse or promote products derived from software without specific 
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR WARRANTIES, 
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. EVENT SHALL GENOME RESEARCH LTD. BE LIABLE 
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
// (INCLUDING, LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.
//
//
#include "Sim.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <errno.h>
#include <stdio.h> // added for low-level file reading by ib5

using namespace std;

Sim::Sim(void) 
{
	version=0;
	filename = "";
}

void Sim::open(string fname) 
{

        char *f = new char[fname.length()+1];
	strcpy(f, fname.c_str());
        openLowLevel(f);

	char buff[256];

	errorMsg = "";

	this->filename = fname;
	_openFile(fname);
	infile->get(buff,4);

	if (strcmp(buff,"sim")) {
		throw("File " + filename + " has invalid header [" + buff + "]");
	}
	magic = buff;

	this->filename = fname;
	// read SIM header
	infile->read((char*)&version,1);
	infile->read((char*)&sampleNameSize,2);
	infile->read((char*)&numSamples,4);
	infile->read((char*)&numProbes,4);
	infile->read((char*)&numChannels,1);
	infile->read((char*)&numberFormat,1);

	// calculate and store record length
	switch (numberFormat) {
	case FLOAT:	      numericBytes= 4;	break;
	case INTEGER:	      numericBytes = 2; break;
	case SCALED_INTEGER:  numericBytes = 2; break;
	default:	cerr << "Invalid number format " << numberFormat;
					exit(1);
	}
	sampleIntensityTotal = numProbes * numChannels;
	recordLength = numProbes * numChannels;
	recordLength *= numericBytes;
	recordLength += sampleNameSize;
	_closeFile();
}

void Sim::openLowLevel(char *fname) {
  inFileRaw = fopen(fname, "r");
  fseek(inFileRaw, HEADER_LENGTH, 0);
  if (ferror(inFileRaw)!=0) {
    cerr << "Error positioning low-level file stream to:" << fname << endl;
    exit(1);
  }
}

void Sim::close(void)
{
	if (filename != "-") fout.close();	// don't close stdout!	
}

void Sim::createFile(string fname)
{
	filename = fname;
}

void Sim::reset(void)
{
  // return to starting point of .sim file (immediately after header)
  // use for multiple passes through .sim file in QC metrics
  if (filename == "-") {
    throw "Cannot reset file position in standard input!";
  }
  infile->seekg(HEADER_LENGTH);
  fseek(inFileRaw, HEADER_LENGTH, 0);

}

void Sim::writeHeader(uint32_t _numSamples, uint32_t _numProbes, uint8_t _numChannels, uint8_t _numberFormat)
{
	version = VERSION;
	sampleNameSize = SAMPLE_NAME_SIZE;
	numSamples = _numSamples;
	numProbes = _numProbes;
	numChannels = _numChannels;
	numberFormat = _numberFormat;

	_openFile(filename,true);
	outfile->write("sim", 3);
	outfile->write((char*)&version, sizeof(version));
	outfile->write((char*)&sampleNameSize, sizeof(sampleNameSize));
	outfile->write((char*)&numSamples, sizeof(numSamples));
	outfile->write((char*)&numProbes, sizeof(numProbes));
	outfile->write((char*)&numChannels, sizeof(numChannels));
	outfile->write((char*)&numberFormat, sizeof(numberFormat));

	outfile->flush();
}

string Sim::dump(void)
{
	ostringstream s;
	s << "Number of SNPs:   " << numSamples << endl
	  << "Magic:            " << magic << endl
	  << "version:          " << version << endl
	  << "Sample Name Size: " << sampleNameSize << endl
	  << "Num Probes:       " << numProbes << endl
	  << "Num Channels:     " << numChannels << endl
	  << "Number Format:    " << numberFormat << endl
	  << endl;
	return s.str();
}

void Sim::__openin(istream &f) 
{
	infile = &f;
}

void Sim::__openout(ostream &f) 
{
	outfile = &f;
}

void Sim::_openFile(string filename, bool writing)
{
	if (filename == "-") {
		if (writing) { __openout(cout); }
		else         { __openin(cin); }
	} else {
		if (writing) { fout.open(filename.c_str(),ios::binary | ios::trunc | ios::in | ios::out); __openout(fout); }
		else         { 
			fin.open(filename.c_str(),ios::binary | ios::in); 
			__openin(fin); 
		}
	}
	if (!writing && (!infile || !fin)) {
		cerr << "Can't open " << filename << " for reading : " << strerror(errno) << endl;
	}
	if (writing && (!outfile || !fout)) {
		cerr << "Can't open " << filename << " for writing : " << strerror(errno) << endl;
	}
}

void Sim::_closeFile(void)
{
//	file.close();
//	file.clear();
}

void Sim::getNextRecord(char *sampleName, vector<uint16_t> *intensity)
{
	unsigned int n;
	char *p;
	char *buff = new char[recordLength];

	infile->read(buff,recordLength);
	memcpy(sampleName,buff,sampleNameSize);
	for (n=0, p=buff+sampleNameSize; n < numProbes*numChannels; n++, p+=2) {
		uint16_t v = *(uint16_t*)p;
		intensity->push_back(v);
	}
	delete buff;
}

void Sim::getNextRecord(char *sampleName, vector<float> *intensity)
{
	unsigned int n;
	char *p;
	char *buff = new char[recordLength];
	infile->read(buff,recordLength);
	memcpy(sampleName,buff,sampleNameSize);
	for (n=0, p=buff+sampleNameSize; n < numProbes*numChannels; n++, p+=4) {
		float v = *(float*)p;
		intensity->push_back(v);
	}
	delete buff;
}

void Sim::getNextRecord(char *sampleName, uint16_t *intensity) {
  char *status = fgets(sampleName, sampleNameSize+1, inFileRaw);
  if (status==NULL) throw("Error reading sample name from .sim file!");
  int items = fread(intensity, numericBytes, sampleIntensityTotal, inFileRaw);
  if (items != sampleIntensityTotal || ferror(inFileRaw)) {
    throw("Error reading intensities from .sim file!");
  }
}

void Sim::getNextRecord(char *sampleName, float *intensity) {
  char *status = fgets(sampleName, sampleNameSize+1, inFileRaw);
  if (status==NULL) throw("Error reading sample name from .sim file!");
  int items = fread(intensity, numericBytes, sampleIntensityTotal, inFileRaw);
  if (items != sampleIntensityTotal || ferror(inFileRaw)) {
    throw("Error reading intensities from .sim file!");
  }
}

void Sim::write(void *buffer, int length)
{
	outfile->write((const char*)buffer,length);
}

