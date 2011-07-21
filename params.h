/*
Copyright (C) TALP Research Center - Universitat Politècnica de Catalunya (UPC)
Written by Josep M. Crego
Barcelona, September 2005

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef PARAM_H
#define PARAM_H

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>
#include "defs.h"
#include <stdlib.h>

using namespace std;

class param{
 public:
  // params to be defined
  vector<unsigned int> b;
  vector<double> timeControl;
  unsigned int nLevels,n,d,f,hr,m,c,sInputType,tInputType,nms;
  float t,ct;
  vector<float> wlb,wpp,wup,wus,wut,wcl,wcn,whp,whs,wht,wzp;
  vector<float> wwda1;
  float wwda2,wwda3,wwda4,wwda5,wwda6;
  float wtga,wrk,wrks,wrkt,wrkb,wrkw,wum,wums, wumt, wf,wfs,wft;
  float w1to1,w1toN,wNto1,w1toNsum,wNtoM;
  float wmatchb,wstemb,wsynb,cmatch,cstem,csyn; //monolingual alignment
  string meshLinkCost; //monolingual alignment
  float wchfilt,wchfilts,wchfiltt;
  string wordAsTab,tagAsTab,mssTab; //word ass. table, tag ass. table, match-stem-syn table
  string sumTab, tumTab, sfertTab,tfertTab,sClFertTab,tClFertTab; //unlinked cost tables, fertility tables and class fertility tables
  vector<string> isfile,itfile;
  string ofile,ostfile, otsfile,itgsfile,itgtfile,nbestFile;
  int nbestSize;
  string outputType,range,direction,init,first,phas,phLinkWeight,exp,multipleStacks;
  bool accu,oneToOne,backTrack;
  int verbose,PORT;
  bool servermode;
  vector<int> nmodels;
  vector<vector<string> > models;
  double nNewStates;
  int doba,dOnlyBestAsso,avgeOnlyBestAsso;
  param(){
    nNewStates=0;
    timeControl.assign(10,0);
  };
  void read_params(int, char **) ;
  void error_params(string err) const;
  void print_server_params() const;
  void print_client_params() const;
  void reset();
  //void detailed_help();
  //void warranty_conditions();
};

#endif

