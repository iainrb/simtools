//
// Egt.h
//
// Author: Iain Bancarz <ib5@sanger.ac.uk>
//
// Copyright (c) 2014 Genome Research Ltd.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, 
// this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright 
// notice, this list of conditions and the following disclaimer in the 
// documentation and/or other materials provided with the distribution.
// 3. Neither the name of Genome Research Ltd nor the names of the 
// contributors may be used to endorse or promote products derived from 
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR 
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL GENOME RESEARCH LTD. BE LIABLE FOR ANY DIRECT, INDIRECT, 
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef _EGT_H
#define _EGT_H

// TODO check if all of these includes are needed
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <stdint.h>

using namespace std;

class Egt {

 public:
  Egt();
  void open(char *filename);
  void open(string filename);
  string filename;
  int NUMERIC_BYTES;
  // EGT header fields
  int fileVersion;
  string gcVersion;
  string clusterVersion;
  string callVersion;
  string normalizationVersion;
  string dateCreated;
  char mode;
  string manifest;

private:
  int readInteger(ifstream &file, bool littleEndian=true);

};

#endif	// _EGT_H
