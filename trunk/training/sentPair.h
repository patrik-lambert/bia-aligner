#ifndef _SENTPAIR_H
#define _SENTPAIR_H

#include <iostream>
#include <fstream>
#include <sstream>
#include "iword.h"
#include "alUnit.h"
#include "confusionNet.h"

typedef vector<iword> sentence;
typedef vector<vector<vector<int> > > phraseToWordMap;

class sentPair {
  
  confusionNet<iword> s,t;
  vector<int> ssize,tsize;
  int loadWordSentence (confusionNet<iword> & cn, string fline, vocTable & voc); //sentence input (inputType=0)
  int loadWordSentence (confusionNet<iword> & cn, const vector<string> & flines, vocTable & voc); //confusion net input (inputType=1)
 public:
  int nlevels;
  sentPair (){};
  int loadSrcWords (string line, vocTable & voc) { 
    int num=loadWordSentence(s,line,voc);
    ssize.push_back(num);
    return num;
  }
  int loadSrcWords (const vector<string> & flines, vocTable & voc) { 
    int num=loadWordSentence(s,flines,voc);
    ssize.push_back(num);
    return num;
  }
  int loadTrgWords (string line, vocTable & voc) { 
    int num=loadWordSentence(t,line,voc); 
    tsize.push_back(num);
    return num;
  }
  int loadTrgWords (const vector<string> & flines, vocTable & voc) { 
    int num=loadWordSentence(t,flines,voc); 
    tsize.push_back(num);
    return num;
  }
  void clear(){
    s.clear();
    t.clear();
    ssize.clear();
    tsize.clear();
  }
  int numSrcToks (int level=0) const {return ssize.at(level);}
  int numTrgToks (int level=0) const {return tsize.at(level);}
  confusionNet<iword> & srcNoconst () { return s; }
  confusionNet<iword> & trgNoconst () { return t; }
  const confusionNet<iword> & src () const { return s; }
  const confusionNet<iword> & trg () const { return t; }
  const iword & srcat (int posi, int colPosi=0) const { 
    //    return s.at(posi).at(0).first; 
    confusionNet<iword>::const_iterator it=s.begin()+posi;
    iwordCnColumn::const_iterator c=it->begin()+colPosi;
    return c->first; 
  } //independant from input type
  const iword & trgat (int posi, int colPosi=0) const { 
    //    return t.at(posi).at(0).first; 
    confusionNet<iword>::const_iterator it=t.begin()+posi;
    iwordCnColumn::const_iterator c=it->begin()+colPosi;
    return c->first; 
  } //independant from input type
  string display() const;
};
#endif

#ifndef _BILCORPUS_H
#define _BILCORPUS_H

class bilCorpus {
  ifstream* fsFile;
  ifstream* ftFile;
  long int sentPairNum,firstSentPairNum,lastSentPairNum;
  bool untilEnd;
  vocTable svoc,tvoc; // correspondance table index->string form

 public:
  bilCorpus (const string & fs, const string & ft, string range, int sInputType,int tInputType);
  bool getNextSentPair (sentPair & sp, vocTable & svoc, vocTable & tvoc, int sInputType, int tInputType);
  long int getSentPairNum (){
    return sentPairNum;
  }
  
};

#endif
