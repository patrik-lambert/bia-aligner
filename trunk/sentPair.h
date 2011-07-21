#ifndef _SENTPAIR_H
#define _SENTPAIR_H

#include <iostream>
#include <fstream>
#include <sstream>
#include "params.h"
#include "iword.h"
#include "fertTable.h"
#include "alUnit.h"
#include "confusionNet.h"

typedef vector<iword> sentence;
typedef vector<vector<vector<int> > > phraseToWordMap;

class sentPair {
  
  confusionNet<iword> s,t;
  uniqAlUnitColumnCollection sAlUnits,tAlUnits;
  bool thereAreTags;
  vector<int> ssize,tsize;
  int loadWordSentence (confusionNet<iword> & cn, uniqAlUnitColumnCollection & units, string fline, vocTable & voc); //sentence input (inputType=0)
  int loadWordSentence (confusionNet<iword> & cn, uniqAlUnitColumnCollection & units, const vector<string> & flines, vocTable & voc); //confusion net input (inputType=1)
  int loadPhraseSentence (sentence & toks, string fline, phraseToWordMap & phToWdPosi,int level);
  int loadPhraseSentence (confusionNet<iword> & cn, string fline, phraseToWordMap & phToWdPosi,int level){};
  int loadPofs (confusionNet<iword> & cn, string fline, vocTable & voc);
  void loadUnlinkedModel(confusionNet<iword> & toks, const unlinkedModelTable & umtab);
  void loadFert(confusionNet<iword> & toks, const fertTable & ftab, string mode);
  void loadClFert(confusionNet<iword> & toks, const classFertTable & ftab);
  phraseToWordMap sPhToWdPosi;
  phraseToWordMap tPhToWdPosi;
 public:
  int nlevels;
  void initPhToWdMaps(){
    vector<vector<int> > vec;sPhToWdPosi.assign(nlevels,vec);tPhToWdPosi.assign(nlevels,vec);
  }
  sentPair (){
    thereAreTags=false;
  };
  int loadSrcWords (string line, vocTable & voc) { 
    int num=loadWordSentence(s,sAlUnits,line,voc);
    ssize.push_back(num);
    return num;
  }
  int loadSrcWords (const vector<string> & flines, vocTable & voc) { 
    int num=loadWordSentence(s,sAlUnits,flines,voc);
    ssize.push_back(num);
    return num;
  }
  int loadTrgWords (string line, vocTable & voc) { 
    int num=loadWordSentence(t,tAlUnits,line,voc); 
    tsize.push_back(num);
    return num;
  }
  int loadTrgWords (const vector<string> & flines, vocTable & voc) { 
    int num=loadWordSentence(t,tAlUnits,flines,voc); 
    tsize.push_back(num);
    return num;
  }
  int loadSrcPhrases (string line, int level) { 
    int num=loadPhraseSentence(s,line,sPhToWdPosi,level);
    ssize.push_back(num);
    return num;
  }
  int loadTrgPhrases (string line, int level) { 
    int num=loadPhraseSentence(t,line,tPhToWdPosi,level);
    tsize.push_back(num);
    return num;
  }
  int loadSrcTags (string line, vocTable & voc) { 
    thereAreTags=true;
    return loadPofs(s,line,voc);
  }
  int loadTrgTags (string line, vocTable & voc) { 
    return loadPofs(t,line,voc);  
  }
  void loadSrcUnlinkedModel(const unlinkedModelTable & umtab){loadUnlinkedModel(s,umtab);}
  void loadTrgUnlinkedModel(const unlinkedModelTable & umtab){loadUnlinkedModel(t,umtab);}
  void loadSrcFert(const fertTable & ftab, string mode){loadFert(s,ftab,mode);}
  void loadTrgFert(const fertTable & ftab, string mode){loadFert(t,ftab,mode);}
  void loadSrcClassFert(const classFertTable & ftab){loadClFert(s,ftab);}
  void loadTrgClassFert(const classFertTable & ftab){loadClFert(t,ftab);}
  void clear(){
    s.clear();
    t.clear();
    sAlUnits.clear();
    tAlUnits.clear();
    ssize.clear();
    tsize.clear();
    sPhToWdPosi.clear();
    tPhToWdPosi.clear();
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
  const vector<int> & srcWdPosis (int level, int phPosi) const {return sPhToWdPosi.at(level).at(phPosi);}
  const vector<int> & trgWdPosis (int level, int phPosi) const {return tPhToWdPosi.at(level).at(phPosi);}
  string display() const;
  string printAlUnits(const vocTable & svoc, const vocTable & tvoc) const;
  void addSrcFoundPhrases(const uniqAlUnitColumnCollection & coll);
  void addTrgFoundPhrases(const uniqAlUnitColumnCollection & coll);
  const uniqAlUnitColumnCollection & srcAlUnits () const {return sAlUnits; }
  const uniqAlUnitColumnCollection & trgAlUnits () const {return tAlUnits; }
  void getAllSrcAlUnits (vector<alUnit> & coll) const {
    for (uniqAlUnitColumnCollection::const_iterator sit=sAlUnits.begin(); sit != sAlUnits.end(); ++sit){
      for (cnColumn<alUnit>::const_iterator cit=sit->begin(); cit!=sit->end();++cit){
	coll.push_back(cit->first);
      }
    }
  }
  void getAllTrgAlUnits (vector<alUnit> & coll) const {
    for (uniqAlUnitColumnCollection::const_iterator sit=tAlUnits.begin(); sit != tAlUnits.end(); ++sit){
      for (cnColumn<alUnit>::const_iterator cit=sit->begin(); cit!=sit->end();++cit){
	coll.push_back(cit->first);
      }
    }
  }
};
#endif

#ifndef _BILCORPUS_H
#define _BILCORPUS_H

class bilCorpus {
  vector<ifstream*> fsFile;
  vector<ifstream*> ftFile;
  ifstream tgsFile;  //tag source
  ifstream tgtFile;  //tag trg
  long int sentPairNum,firstSentPairNum,lastSentPairNum;
  bool untilEnd;
  vocTable svoc,tvoc; // correspondance table index->string form

 public:
  bilCorpus (const vector<string> & fs, const vector<string> & ft, string ftgs, string ftgt, string range, int sInputType,int tInputType);
  bool getNextSentPair (sentPair & sp, vocTable & svoc, vocTable & tvoc, int sInputType, int tInputType, vocTable & sTagVoc, vocTable & tTagVoc);
  long int getSentPairNum (){
    return sentPairNum;
  }
  
};

#endif
