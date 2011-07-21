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

void phraseDetection::searchSentence(const sentence & sent, uniqAlUnitCollection & foundPhrases){
  int count=0;
  for (sentence::const_iterator sit=sent.begin();sit != sent.end();++sit){
    formType currWord=sit->getForm();
    expand(currWord,count,sent,foundPhrases); 
    ++count;
  }
}
void phraseDetection::expand(formType word, int nWordDone, const sentence & sent, uniqAlUnitCollection & foundPhrases){
  uniqAlUnit curPh(word,nWordDone);
  //cout<<"expanding "<<word<<" ndone:"<<nWordDone<<endl;
  int cnt=nWordDone+1;
  alUnitVocMap::const_iterator vocit=phVoc.find(curPh);
  alUnitVocMap::const_iterator subvocit=subPhVoc.find(curPh);
  while ((vocit != phVoc.end() || subvocit != subPhVoc.end()) && cnt<sent.size()){
    if (vocit != phVoc.end()){
      foundPhrases.push_back(curPh);
    }
    formType nextWd=sent[cnt].getForm();
    curPh.push_back(nextWd);
    ++cnt;
    vocit=phVoc.find(curPh);
    subvocit=subPhVoc.find(curPh);
    //cout<<curPh.print()<<curPh.printDelim();
  }
  //  cout<<endl;
  if (cnt==sent.size() && vocit != phVoc.end()){
    foundPhrases.push_back(curPh);
  }
}

void phraseDetection::searchSentence(const confusionNet<iword> & cn, uniqAlUnitColumnCollection & foundPhrases){
  int count=0;
  confusionNet<iword>::const_iterator penultit=cn.end(); 
  --penultit;
  for (confusionNet<iword>::const_iterator cnit=cn.begin();cnit != penultit;++cnit){
    uniqAlUnitColumn curPhCol(count);
    for (iwordCnColumn::const_iterator cit=cnit->begin();cit != cnit->end();++cit){
      curPhCol.push_back(alUnit(cit->first.getForm()),cit->second);
    }
    expand(curPhCol,count,cn,foundPhrases); 
    ++count;
  }
}
// NOTE: we must expand all uniqAlUnits with the same (_begin, size) at the same time to put them in the same column of the uniqAlUnitColumnCollection
void phraseDetection::expand(uniqAlUnitColumn & curPhCol, int nWordDone, const confusionNet<iword> & cn, uniqAlUnitColumnCollection & foundPhrases){
  //cout<<"expanding "<<curPhCol.print()<<" ndone:"<<nWordDone<<endl;
  bool newFoundPhrases=false;
  int cnt=nWordDone+1;
  alUnit::size_type phsize=curPhCol.begin()->first.size();
  //cout<<"\tphsize:"<<phsize<<endl;
  uniqAlUnitColumn toExpand(cnt-phsize);
  uniqAlUnitColumnCollection::iterator foundit;
  confusionNet<iword>::const_iterator cnit=cn.begin()+cnt;
  for (uniqAlUnitColumn::const_iterator unitcolit=curPhCol.begin();unitcolit!=curPhCol.end();++unitcolit){
    for (iwordCnColumn::const_iterator wdcolit=cnit->begin();wdcolit != cnit->end();++wdcolit){
      // add next word to current alUnit, make product of probabilities and see if it exists in Voc and subVoc
      alUnit curPh(unitcolit->first);
      curPh.push_back(wdcolit->first.getForm());
      float curProb=unitcolit->second * wdcolit->second;
      //cout<<"\t hyp:"<<curPh.print()<<","<<curProb<<endl;
      alUnitVocMap::const_iterator vocit=phVoc.find(curPh);
      if (vocit != phVoc.end()){
	if (!newFoundPhrases){
	  foundPhrases.push_back(uniqAlUnitColumn(curPhCol.firstPosi()));
	  newFoundPhrases=true;
	  foundit=foundPhrases.end()-1;
	}
	foundit->push_back(curPh,curProb);
	//cout<<"\t is a phrase\n";
      }
      alUnitVocMap::const_iterator subvocit=subPhVoc.find(curPh);
      if (subvocit != subPhVoc.end()){
	toExpand.push_back(curPh,curProb);
	//cout<<"\t is a sub phrase\n";
      }
      //cout<<curPh.print()<<curPh.printDelim()<<endl;
    }
  }
  if ( toExpand.size()>0 && cnt<cn.size()-1){
    expand(toExpand,cnt,cn,foundPhrases); 
  }
}

void phraseDetection::pruneVoc(int minOccur){
  alUnitVocMap::iterator it=phVoc.begin();
  alUnitVocMap noPrune;
  for (alUnitVocMap::iterator it=phVoc.begin();it!=phVoc.end();++it){
    if (it->second>=minOccur){
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
