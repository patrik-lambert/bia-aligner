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
void alUnit::reduce(alUnit & reducedUnit, const vocCorrespondence & vocToRedVoc) const {
  for (alUnit::const_iterator it=unit.begin();it!=unit.end();++it){
    reducedUnit.push_back(vocToRedVoc.reduce(*it));
  }
}
