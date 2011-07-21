#include "linkCluster.h"

// LINKCLUSTER CLASS
//***********************
linkCluster::linkCluster(int _src, int _trg){
  sPosis.push_back(_src);
  tPosis.push_back(_trg);
}
linkCluster::linkCluster(vector<int> sVec, vector<int> tVec){
  sPosis=sVec;
  tPosis=tVec;
}

bool linkCluster::isPhrase(const vector<int> & posis) const {
  // (assumes cluster is sorted and contiguous)
  if (posis.size()<=1) return false;
  //   int smallest,greatest;
  //   for (vector<int>::const_iterator p=posis.begin();p!=posis.end();++p){
  //     if (p == posis.begin()){
  //       smallest=*p;
  //       greatest=*p;
  //     }else{
  //       if (*p<smallest) smallest=*p;
  //       if (*p>greatest) greatest=*p;
  //     }
  //   }
  //   if (posis.size() == greatest-smallest+1) return true;
  //   else return false;
  else return true;
}
void linkCluster::addSposi(int _posi){
  sPosis.push_back(_posi);
}
void linkCluster::addTposi(int _posi){
  tPosis.push_back(_posi);
}
void linkCluster::insertCluster(const linkCluster & lc){
  for (vector<int>::const_iterator c=lc.sPosis.begin(); c!=lc.sPosis.end();++c){
    addSposi(*c);
  }
  for (vector<int>::const_iterator c=lc.tPosis.begin(); c!=lc.tPosis.end();++c){
    addTposi(*c);
  }
}
void linkCluster::doSort(){
  if (sPosis.size()>1){
    sort(sPosis.begin(),sPosis.end());
  }
  if (tPosis.size()>1){
    sort(tPosis.begin(),tPosis.end());
  }
}
void linkCluster::sortAndSplitIntoContiguous(vector<linkCluster> & splitvec){
  bool splitSource=false;
  if (sPosis.size()>1){
    sort(sPosis.begin(),sPosis.end());
    if ( sPosis.back()-sPosis.front() > (int) sPosis.size()-1) 
      splitSource=true;
  }
  bool splitTarget=false;
  if (tPosis.size()>1){
    sort(tPosis.begin(),tPosis.end());
    if (tPosis.back()-tPosis.front() > (int) tPosis.size()-1) 
      splitTarget=true;
  }
  if (splitSource || splitTarget){
    vector<vector<int> > srcVectors;
    vector<vector<int> > trgVectors;
    if (splitSource){
      vector<int> curPosis(1,sPosis.front());
      for (vector<int>::const_iterator v=sPosis.begin()+1;v!=sPosis.end();++v){
	if (*v-curPosis.back()>1){
	  srcVectors.push_back(curPosis);
	  curPosis.clear();
	}
	curPosis.push_back(*v);
      }
      srcVectors.push_back(curPosis);
    }else{
      srcVectors.push_back(sPosis);
    }
    if (splitTarget){
      vector<int> curPosis(1,tPosis.front());
      for (vector<int>::const_iterator v=tPosis.begin()+1;v!=tPosis.end();++v){
	if (*v-curPosis.back()>1){
	  trgVectors.push_back(curPosis);
	  curPosis.clear();
	}
	curPosis.push_back(*v);
      }
      trgVectors.push_back(curPosis);
    }else{
      trgVectors.push_back(tPosis);
    }
    for (vector<vector<int> >::const_iterator sv=srcVectors.begin();sv!=srcVectors.end();++sv){
      for (vector<vector<int> >::const_iterator tv=trgVectors.begin();tv!=trgVectors.end();++tv){
	splitvec.push_back(linkCluster(*sv,*tv));
      }
    }  
  }
}
string linkCluster::print() const {
  ostringstream oss("");
  for (vector<int>::const_iterator p=sPosis.begin();p!=sPosis.end();++p){
    if (p != sPosis.begin()){oss<<",";}
    oss<<*p;
  }
  oss<<"-";
  for (vector<int>::const_iterator p=tPosis.begin();p!=tPosis.end();++p){
    if (p != tPosis.begin()){oss<<",";}
    oss<<*p;
  }
  return oss.str();
}

// LINKCLUSTERDIV CLASS
//***********************
void linkClusterDiv::addLink(int sposi, int tposi){
  hash_map<int,int>::iterator sc=scomp.find(sposi);
  hash_map<int,int>::iterator tc=tcomp.find(tposi);
  if (sc != scomp.end() || tc != tcomp.end()){
    if (sc != scomp.end() && tc != tcomp.end()){
      if (sc->second != tc->second){
	// merge clusters
	int clustIndex1, clustIndex2;
	if (sc->second < tc->second){
	  clustIndex1=sc->second;
	  clustIndex2=tc->second;
	}else{
	  clustIndex1=tc->second;
	  clustIndex2=sc->second;
	}
	//add elems of cluster clustIndex2 into cluster clustIndex1
	this->at(clustIndex1).insertCluster(this->at(clustIndex2));
	for (hash_map<int,int>::iterator c=scomp.begin();c!=scomp.end();++c){
	  if (c->second==clustIndex2){c->second=clustIndex1;}
	  if (c->second>clustIndex2){--c->second;}
	}
	for (hash_map<int,int>::iterator c=tcomp.begin();c!=tcomp.end();++c){
	  if (c->second==clustIndex2){c->second=clustIndex1;}
	  if (c->second>clustIndex2){--c->second;}
	}
	this->erase(this->begin()+clustIndex2);
	--nClust;
      }
    }else if (sc != scomp.end()){
      tcomp.insert(make_pair(tposi,sc->second));
      this->at(sc->second).addTposi(tposi);
    }else if (tc != tcomp.end()){
      scomp.insert(make_pair(sposi,tc->second));
      this->at(tc->second).addSposi(sposi);
    }
  }else{
    this->push_back(linkCluster(sposi,tposi));
    scomp.insert(make_pair(sposi,nClust));
    tcomp.insert(make_pair(tposi,nClust));
    ++nClust;
  }
}
void linkClusterDiv::sortAndSplitIntoContiguous() {
  vector<linkCluster> newClusters;
  vector<vector<linkCluster>::iterator> toDelete;
  for (vector<linkCluster>::iterator v=this->begin();v!=this->end();++v){
    vector<linkCluster> newVec;
    v->sortAndSplitIntoContiguous(newVec);
    if (newVec.size()>0){
      newClusters.insert(newClusters.end(),newVec.begin(),newVec.end());
      toDelete.push_back(v);
    }
  }
  int offset=0;
  for (vector<vector<linkCluster>::iterator>::const_iterator it=toDelete.begin();it!=toDelete.end();++it){
    this->erase(*it-offset);
    ++offset;
  }
  this->insert(this->end(),newClusters.begin(),newClusters.end());
}

string linkClusterDiv::print() const {
  ostringstream oss("");
  for (vector<linkCluster>::const_iterator v=this->begin();v!=this->end();++v){
    if (v!=this->begin()){oss<<" ";}
    oss<<v->print();
  }
  return oss.str();
}

// void linkClusterDiv::storePhrases(phraseDetection & sphDetect, phraseDetection & tphDetect, vector<jword> & sent1, vector<jword> & sent2) {
//   for (vector<linkCluster>::iterator cl=this->begin();cl!=this->end();++cl){
//     if (cl->srcIsPhrase()){
//       vector<int> seq;
//       cl->sPhrase(seq);
//       alUnit ph;
//       for (vector<int>::iterator v=seq.begin();v!=seq.end();++v){
// 	ph.push_back(sent1[*v].form());
// 	//cout<<sent1[*v].form<<" ";
//       }
//       //cout<<endl;
//       sphDetect.loadPhrase(ph);
//     }
//     if (cl->trgIsPhrase()){
//       vector<int> seq;
//       cl->tPhrase(seq);
//       alUnit ph;
//       for (vector<int>::iterator v=seq.begin();v!=seq.end();++v){
//  	ph.push_back(sent2[*v].form());
//       }
//       tphDetect.loadPhrase(ph);
//     }
//   }
// }
