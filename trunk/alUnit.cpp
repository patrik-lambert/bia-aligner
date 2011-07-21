#include "alUnit.h"

string alUnit::print(const vocTable & voc) const {
  ostringstream oss("");
  for (vector<formType>::const_iterator it=unit.begin();it!=unit.end();++it){
    if (it != unit.begin()){oss<<" ";}
    oss<<voc.form(*it);
  }
  return oss.str();
}
string alUnit::printDelim() const {
  return " "+alUnitConst::unitDelim+" ";
  //return " ";
}

alUnit::alUnit (formType f) {
  unit.push_back(f);
}
alUnit::alUnit (vector<formType> & v) {
  unit=v;
}
void alUnit::push_back (formType f){
  unit.push_back(f);
}
bool alUnit::operator== (const alUnit & y) const {
  return unit==y.unit;
}
uniqAlUnit::uniqAlUnit (formType f, int posi) {
  this->push_back(f);
  _beg=posi;
}
string uniqAlUnit::print(const vocTable & voc) const {
  ostringstream oss("");
  oss<<_beg<<"-"<<lastPosi()<<":";
  for (vector<formType>::const_iterator it=this->begin();it!=this->end();++it){
    if (it != this->begin()){oss<<" ";}
    oss<<voc.form(*it);
  }
  return oss.str();
}
void uniqAlUnitCollection::push_back(const uniqAlUnit & u){
  unitColl.push_back(u);
}
string  uniqAlUnitCollection::print(const vocTable & voc) const {
  ostringstream oss("");
  for (vector<uniqAlUnit>::const_iterator it=unitColl.begin();it!=unitColl.end();++it){
    if (it != unitColl.begin()){oss<<", ";}
    oss<<it->print(voc);
  }
  return oss.str();
}

string uniqAlUnitColumn::print(const vocTable & voc) const {
  ostringstream oss("");
  oss<<_beg<<"-"<<lastPosi()<<":";
  for (cnColumn<alUnit>::const_iterator it=this->begin();it!=this->end();++it){
    if (it != this->begin()){oss<<" ||| ";}
    oss<< it->first.print(voc) <<","<< it->second;
  }
  return oss.str();
}

string  uniqAlUnitColumnCollection::print(const vocTable & voc) const {
  ostringstream oss("");
  int count=0;
  for (vector<uniqAlUnitColumn>::const_iterator it=collection.begin();it!=collection.end();++it){
    //    if (it != collection.begin()){oss<<", ";}
    oss<<count<<" ["<<it->print(voc)<<"] ";
    ++count;
  }
  return oss.str();
}
