#include "phrase.h"

void phraseDetection::loadPhrase(alUnit & _alUnit){
  alUnitVocMap::iterator vocit=phVoc.find(_alUnit);
  if (vocit == phVoc.end()){
    phVoc.insert(make_pair(_alUnit,1));
  }else{
    ++vocit->second;
  }
  alUnit subph;
  for (alUnit::const_iterator p=_alUnit.begin();p!=_alUnit.end()-1;++p){
    subph.push_back(*p);
    alUnitVocMap::iterator subvocit=subPhVoc.find(subph);
    if (subvocit == subPhVoc.end()){
      subPhVoc.insert(make_pair(subph,1));
    }else{
      ++subvocit->second;
    }
  }
}

void phraseDetection::searchSentence(const vector<jword> & sent, phraseDetection::mapType & foundPhrases){
  int count=0;
  for (vector<jword>::const_iterator sit=sent.begin();sit != sent.end();++sit){
    formType currWord=sit->form();
    expand(currWord,count,sent,foundPhrases); 
    ++count;
  }
}

void phraseDetection::expand(formType word, int nWordDone, const vector<jword> & sent, phraseDetection::mapType & foundPhrases){
  alUnit curPh;
  //cout<<"expanding "<<word<<" ndone:"<<nWordDone<<endl;
  curPh.push_back(word);
  unsigned int cnt=nWordDone+1;
  alUnitVocMap::const_iterator vocit=phVoc.find(curPh);
  alUnitVocMap::const_iterator subvocit=subPhVoc.find(curPh);
  while ( (vocit != phVoc.end() && cnt<=sent.size()) || (subvocit != subPhVoc.end() && cnt<sent.size()) ){
    if (vocit != phVoc.end()){
      mapType::iterator fpit=foundPhrases.find(curPh);
      if (fpit != foundPhrases.end()){ ++fpit->second;}
      else{foundPhrases.insert(make_pair(curPh,1));}
    }
    if (cnt<sent.size()){
      formType nextWd=sent.at(cnt).form();
      curPh.push_back(nextWd);
      vocit=phVoc.find(curPh);
      subvocit=subPhVoc.find(curPh);
    //       cout<<curPh.print()<<curPh.printDelim();
    }
    ++cnt;
  }
  //  cout<<endl;
}
// void phraseDetection::expand(formType word, int nWordDone, const vector<jword> & sent, phraseDetection::mapType & foundPhrases){
//   alUnit curPh;
//   //cout<<"expanding "<<word<<" ndone:"<<nWordDone<<endl;
//   curPh.push_back(word);
//   unsigned int cnt=nWordDone+1;
//   alUnitVocMap::const_iterator vocit=phVoc.find(curPh);
//   alUnitVocMap::const_iterator subvocit=subPhVoc.find(curPh);
//   while (vocit != phVoc.end() || subvocit != subPhVoc.end()){
//     if (vocit != phVoc.end()){
//       mapType::iterator fpit=foundPhrases.find(curPh);
//       if (fpit != foundPhrases.end()){ ++fpit->second;}
//       else{foundPhrases.insert(make_pair(curPh,1));}
//     }
//     formType nextWd=sent[cnt].form();
//     curPh.push_back(nextWd);
//     ++cnt;
//     vocit=phVoc.find(curPh);
//     subvocit=subPhVoc.find(curPh);
//     //       cout<<curPh.print()<<curPh.printDelim();
//   }
//   //  cout<<endl;
//   if (cnt==sent.size() && vocit != phVoc.end()){
//     mapType::iterator fpit=foundPhrases.find(curPh);
//     if (fpit != foundPhrases.end()){ ++fpit->second;}
//     else{foundPhrases.insert(make_pair(curPh,1));}
//   }
// }
void phraseDetection::pruneVoc(unsigned int minOccur){
  alUnitVocMap::iterator it=phVoc.begin();
  alUnitVocMap noPrune;
  for (alUnitVocMap::iterator it=phVoc.begin();it!=phVoc.end();++it){
    if (it->second >= minOccur){
      noPrune.insert(make_pair(it->first,it->second));
    }
  }
  phVoc=noPrune;
  alUnitVocMap noPruneSub;
  for (alUnitVocMap::iterator it=subPhVoc.begin();it!=subPhVoc.end();++it){
    if (it->second>=minOccur){
      noPruneSub.insert(make_pair(it->first,it->second));
    }
  }
  subPhVoc=noPruneSub;
}
void phraseDetection::pruneVocAsReduced(const phraseDetection & redPhSearch, const vocCorrespondence & vocToRedVoc){
  alUnitVocMap::iterator it=phVoc.begin();
  alUnitVocMap noPrune;
  for (alUnitVocMap::iterator it=phVoc.begin();it!=phVoc.end();++it){
    alUnit reducedUnit;
    for (alUnit::const_iterator unit=it->first.begin();unit!=it->first.end();++unit){
      reducedUnit.push_back(vocToRedVoc.reduce(*unit));
    }
    if (redPhSearch.isInVoc(reducedUnit)){
      noPrune.insert(make_pair(it->first,it->second));
    }
  }
  phVoc=noPrune;
  alUnitVocMap noPruneSub;
  for (alUnitVocMap::iterator it=subPhVoc.begin();it!=subPhVoc.end();++it){
    alUnit reducedUnit;
    for (alUnit::const_iterator unit=it->first.begin();unit!=it->first.end();++unit){
      reducedUnit.push_back(vocToRedVoc.reduce(*unit));
    }
    if (redPhSearch.isInSubVoc(reducedUnit)){
      noPruneSub.insert(make_pair(it->first,it->second));
    }
  }
  subPhVoc=noPruneSub;
}
bool phraseDetection::isInVoc(const alUnit & _alUnit) const {
    alUnitVocMap::const_iterator vocit=phVoc.find(_alUnit);
    return (vocit != phVoc.end());
}

bool phraseDetection::isInSubVoc(const alUnit & _alUnit) const {
    alUnitVocMap::const_iterator vocit=subPhVoc.find(_alUnit);
    return (vocit != subPhVoc.end());
}
string phraseDetection::printVoc(const vocTable & voc) const {
  ostringstream oss("");
  for (alUnitVocMap::const_iterator it=phVoc.begin();it!=phVoc.end();++it){
    oss<<it->first.print(voc)<<" ("<<it->second<<")"<<endl;
  }
  return oss.str();
}
string phraseDetection::printSubVoc(const vocTable & voc) const {
  ostringstream oss("");
  for (alUnitVocMap::const_iterator it=subPhVoc.begin();it!=subPhVoc.end();++it){
    oss<<it->first.print(voc)<<" ("<<it->second<<")"<<endl;
  }
  return oss.str();
}
