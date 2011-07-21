#include "iword.h"

/*iword::iword( int p, wordIndex f, int _pofs ) {
  pos.push_back(p);
  form.push_back(f);
  pofs.push_back(_pofs);
  }*/
void iword::setPos( int p, int lev){
  pos[lev] = p;
}
void iword::addPos( int p){
  pos.push_back(p);
}
int iword::getPos(int level) const {
  return pos[level];
}
void iword::setForm( wordIndex f, int lev){
  form[lev] = f;
}
void iword::addForm( wordIndex f){
  form.push_back(f);
}
wordIndex iword::getForm(int level) const {
  return form[level];
}
 
void iword::setStform( string _stform, int lev){
  stform[lev] = _stform;
}
void iword::addStform( string _stform){
  stform.push_back(_stform);
}
string iword::getStform(int level) const {
  return stform[level];
}
void iword::setPofs( int _pofs){
  pofs = _pofs;
}
int iword::getPofs() const{
  return pofs;
}
void iword::setNullCst(cost _cst) {
  nullCst=_cst;
}
cost iword::getNullCst() const {
  return nullCst;
}
void iword::setFertCostsVec(const vector<cost> & vec){
  vfert=vec;
}
cost iword::getFertCst(unsigned int fert) const {
  if (fert > vfert.size()-1) return vfert.at(vfert.size()-1);
  else return vfert.at(fert);
}
unsigned int iword::getFertSize() const{
  return vfert.size();
}
string iword::print() const {
  ostringstream oss("");
  oss<<"pos:";
  for (vector<int>::const_iterator it=pos.begin(); it!=pos.end();++it){oss<<*it<<" ";}
  oss<<"form:";
  for (vector<wordIndex>::const_iterator it=form.begin(); it!=form.end();++it){oss<<*it<<" ";}
  oss<<"stform:";
  for (vector<string>::const_iterator it=stform.begin(); it!=stform.end();++it){oss<<*it<<" ";}
  oss<<"pOfs:"<<pofs;
  oss<<" nullCst:"<<nullCst;
  oss<<" fert:";
  for (vector<cost>::const_iterator it=vfert.begin(); it!=vfert.end();++it){oss<<*it<<" ";}
  return oss.str();
}

/* vocTable class
********************/

// void vocTable::load (string fname){
//   ifstream ffile(fname.c_str());
//   // open input and target (word forms) files:
//   if (! ffile){
//     cerr << "ERROR while opening file:" << fname << endl;
//     exit(EXIT_FAILURE);
//   }
//   string line;
//   if (ffile.eof()){
//     cerr << "file "<< fname << " is empty" << endl;
//     exit(EXIT_FAILURE);
//   }
//   while (getline(ffile,line)){
//     //line format: wordIndex wordform num_occurencies
//     //cout << line << endl;
//     istringstream iss(line);
//     wordIndex ind;
//     string form;
//     iss >> ind >> form;
//     jash_voc[ind]= form;
//   }
// }

void iwordCnColumn::merge(string mode){
  if (mode == "best"){
    iwordCnColumn::const_iterator bestit;
    float best=0;
    for (iwordCnColumn::const_iterator cit=this->begin(); cit!=this->end();++cit){
      if ( cit->second > best ){
	bestit=cit;
	best=cit->second;
      }
    }
    nullCst=bestit->first.getNullCst();
    for (unsigned int i=0; i<bestit->first.getFertSize();++i){
      if (vfert.size()>i){
	vfert[i]=bestit->first.getFertCst(i);
      }else{
	vfert.push_back(bestit->first.getFertCst(i));
      }
    }
  }else{
    // Interpolation
    iwordCnColumn::const_iterator begit=this->begin();
    nullCst=begit->first.getNullCst() * begit->second;
    for (unsigned int i=0; i<begit->first.getFertSize();++i){
      if (vfert.size()>i){
	vfert[i] = begit->first.getFertCst(i) * begit->second;
      }else{
	vfert.push_back(begit->first.getFertCst(i) * begit->second);
      }
    }
    ++begit;
    for (iwordCnColumn::const_iterator cit=begit; cit!=this->end();++cit){
      nullCst += cit->first.getNullCst() * cit->second;
      for (unsigned int i=0; i<cit->first.getFertSize();++i){
	vfert[i] += cit->first.getFertCst(i) * cit->second;
      }
    }
  }
}

cost iwordCnColumn::getNullCst() const {
  return nullCst;
}
cost iwordCnColumn::getFertCst(unsigned int fert) const {
  if (fert > vfert.size()-1) return vfert.at(vfert.size()-1);
  else return vfert.at(fert);
}
unsigned int iwordCnColumn::getFertSize() const{
  return vfert.size();
}
