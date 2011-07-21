#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <cctype>
#include <cstring>
#include "math.h"
#include "iword.h"
#include "alUnit.h"

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif

using namespace std;

//******************************************************
struct ltcst
{
  string rankMethod;
  ltcst( string rnkMeth ) : rankMethod(rnkMeth) {}
  bool operator()(const pair<alUnit,vector<float> > & p1, const pair<alUnit,vector<float> > & p2) const
  {
    float val1,val2;
    if (rankMethod=="2"){
      val1=p1.second[1]; // rankMethod=="1" is the default
      val2=p2.second[1];
    }else if (rankMethod=="3"){
      val1=p1.second[2]; // rankMethod=="1" is the default
      val2=p2.second[2];
    }else if (rankMethod=="1+2"){
      val1=p1.second[0]+p1.second[1];
      val2=p2.second[0]+p2.second[1];
    }else if (rankMethod=="2+3"){
      val1=p1.second[1]+p1.second[2];
      val2=p2.second[1]+p2.second[2];
    }else if (rankMethod=="1+2+3"){
      val1=p1.second[0]+p1.second[1]+p1.second[2];
      val2=p2.second[0]+p2.second[1]+p2.second[2];
    }else{ // rankMethod=="1" is the default
      val1=p1.second[0]; 
      val2=p2.second[0];
    }
    if ( val1<val2 || ( val1==val2 && p1.first.size()<p2.first.size())){
      return true;
    }else if (val1==val2 && p1.first.size()==p2.first.size()){
      for (alUnit::size_type n=0;n<p1.first.size();++n){
	if (p1.first.at(n) < p2.first.at(n)){
	  return true;
	}else if (p1.first.at(n) > p2.first.at(n) || n==p1.first.size()-1){
	  return false;
	}
      }
    }
    return false;
  }
};
//class wrd2cst: public set< pair<wordIndex,vector<float> >, ltcst > {};
typedef set< pair<alUnit,vector<float> >, ltcst > wrd2cst;

//******************************************************

int main( int argc, char* argv[])
{
  // params
  vocTable svoc,tvoc;
  string iFileName;
  int verbose=1;
  int nbest=60;
  int unbest=0;
  float threshold=2.7;
  float columnThreshold=10;
  string rankingScore="2";
  // read and assign command line arguments
  if ( argc ==1 ){
    cerr << "Usage:\n"<<"prune-assoScoreTable [-nbest nbest -unbest unbest -th threshold -score rankingScore -v verbose -h] -i ifile > output_table_file\n";
    cerr << "\t-nbest INT Number of target correspondants selected for each source word, and vice-versa [default 60]\n";
    cerr << "\t           The intersection of source-target and target-source lists is taken\n";
    cerr << "\t-unbest INT (Union nbest): Minimum nb of trg correspondants selected for each src word, and vice-versa [default 0]\n";
    cerr << "\t-th FLOAT Threshold cost for pruning [default 2.7]\n";
    cerr << "\t-colth FLOAT Additional threshold cost which applies for score columns 1, 2 and 3 ( [default 0:not used]\n";
    cerr << "\t-score [1|2|3|1+2|2+3|1+2+3] Score taken into account to rank {src,trg} word pairs and for threshold pruning [default 1]\n";
    cerr << "\t\t1,2,3 represent the column's score taken into account\n";
    cerr << "Example: if unbest=10 and nbest=90, the 10 best src for each trg and the 10 best trg for each src are selected\n";
    cerr << "src-trg pairs ranked from 11 to 90 both in src-trg and trg-src lists are also selected\n\n";
    exit(EXIT_FAILURE);
  }else{
    for (int i=1;i<argc;i+=2){
      if (strcmp(argv[i],"-i")==0){
	iFileName = argv[i+1];				
      }else if (strcmp(argv[i],"-v")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>verbose;
      }else if (strcmp(argv[i],"-nbest")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>nbest;
      }else if (strcmp(argv[i],"-unbest")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>unbest;
      }else if (strcmp(argv[i],"-th")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>threshold;
      }else if (strcmp(argv[i],"-colth")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>columnThreshold;
      }else if (strcmp(argv[i],"-score")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>rankingScore;
      }else if (strcmp(argv[i],"-h")==0){
	cerr << "Usage:\n"<<"prune-assoScoreTable [-nbest nbest -unbest unbest -th threshold -score rankingScore -v verbose -h] -i ifile > output_table_file\n";
	cerr << "\t-nbest INT Number of target correspondants selected for each source word, and vice-versa [default 60]\n";
	cerr << "\t           The intersection of source-target and target-source lists is taken\n";
	cerr << "\t-unbest INT (Union nbest): Minimum nb of trg correspondants selected for each src word, and vice-versa [default 0]\n";
	cerr << "\t-th FLOAT Threshold cost for pruning [default 2.7]\n";
	cerr << "\t-colth FLOAT Additional threshold cost which applies for score columns 1, 2 and 3 ( [default 0:not used]\n";
	cerr << "\t-score [1|2|3|1+2|2+3|1+2+3] Score taken into account to rank {src,trg} word pairs and for threshold pruning [default 1]\n";
	cerr << "\t\t1,2,3 represent the column's score taken into account\n";
	cerr << "Example: if unbest=10 and nbest=90, the 10 best src for each trg and the 10 best trg for each src are selected\n";
	cerr << "src-trg pairs ranked from 11 to 90 BOTH in src-trg and trg-src lists are also selected\n\n";
	exit(EXIT_FAILURE);
      }else{
	cerr << argv[i] << ": invalid option" << endl;
	exit(EXIT_FAILURE);
      }
    }
  } 
  if (unbest>nbest){
    cerr << "ERROR: unbest must be equal or smaller than nbest !" << endl;
    exit(EXIT_FAILURE);
  }
  ifstream iFile(iFileName.c_str());
  if (! iFile){
    cerr << "Error while opening file:" << iFileName << endl;
    exit(EXIT_FAILURE);
  }

  //Parse i file and load hash
  unsigned long int lnum=0;
  string line;
  // define an instanstation of compare function for rankingScore parameter:
  ltcst cmpFunc(rankingScore);
  hash_map<alUnit, wrd2cst*, hashfalunit, hasheqalunit > cost_st;
  hash_map<alUnit, wrd2cst*, hashfalunit, hasheqalunit > cost_ts;
  if (verbose>0){cerr<< "Parsing full table...";}
  //  const string phDelim("|||");
  while (getline(iFile,line)){
    ++lnum;
    alUnit srcv,trgv;
    istringstream iss(line);
    string curtok="";
    wordIndex curInd;
    while (curtok != alUnitConst::unitDelim){
      iss >> curtok;
      curInd=svoc.insert(curtok);
      if (curtok != alUnitConst::unitDelim){
	srcv.push_back(curInd);
      }
    }
    
    curtok="";
    while (curtok != alUnitConst::unitDelim){
      iss >> curtok;
      curInd=tvoc.insert(curtok);
      if (curtok != alUnitConst::unitDelim){
	trgv.push_back(curInd);
      }
    }
    
    float cst;
    vector<float> cost;
    while (iss>>cst){
      cost.push_back(cst);
    }
    // load in cost_st
    if (rankingScore=="2"){
      cst=cost[1];
    }else if (rankingScore=="3"){
      cst=cost[2];
    }else if (rankingScore=="1+2"){
      cst=cost[0]+cost[1];
    }else if (rankingScore=="2+3"){
      cst=cost[1]+cost[2];
    }else if (rankingScore=="1+2+3"){
      cst=cost[0]+cost[1]+cost[2];
    }else{ // rankingScore=1 is the default
      cst=cost[0];
    }
    // threshold pruning first (this is equivalent to do histogram pruning first, but requires much less memory).
    if (cst<threshold && (columnThreshold==0 || cost[0]<columnThreshold && cost[1]<columnThreshold && cost[2]<columnThreshold) ){
      hash_map<alUnit, wrd2cst*, hashfalunit, hasheqalunit >::iterator srcit=cost_st.find(srcv);
      if (srcit==cost_st.end()){
 	//define a set with our instantiated compare function:
 	wrd2cst *setPt= new wrd2cst(cmpFunc);
 	setPt->insert(make_pair(trgv,cost));
 	cost_st.insert(make_pair(srcv,setPt));
      }else{
	srcit->second->insert(make_pair(trgv,cost));
      }
      // load in cost_ts
      hash_map<alUnit, wrd2cst*, hashfalunit, hasheqalunit >::iterator trgit=cost_ts.find(trgv);
      if (trgit==cost_ts.end()){
	wrd2cst *setPt= new wrd2cst(cmpFunc);
	setPt->insert(make_pair(srcv,cost));
	cost_ts.insert(make_pair(trgv,setPt));
      }else{
	trgit->second->insert(make_pair(srcv,cost));
      }
    }
    if (verbose>1){cout<<"src:"<<srcv.print(svoc)<<" trg:"<<trgv.print(tvoc)<<" cost:"<<cst<<endl;}
    if (verbose>0 && lnum % 1000000==0){cerr<< ".";}
    if (verbose>0 && lnum % 10000000==0){cerr<<lnum;}
  }
  if (verbose>1){
    cout<<endl;
    for (hash_map<alUnit, wrd2cst*, hashfalunit, hasheqalunit >::const_iterator cit=cost_st.begin();cit!=cost_st.end();++cit){
      cout << "src:"<<cit->first.print(svoc)<<endl;
      for (wrd2cst::const_iterator wit=(cit->second)->begin();wit!=(cit->second)->end();++wit){
	cout <<"trg:"<<wit->first.print(tvoc)<<alUnitConst::unitDelim<<" costs:";
	for (vector<float>::const_iterator vec=wit->second.begin(); vec!=wit->second.end(); ++vec){cout<<" "<<*vec;}
	cout<<endl;
      }
    }
    cout<<endl;
    for (hash_map<alUnit, wrd2cst*, hashfalunit, hasheqalunit>::const_iterator cit=cost_ts.begin();cit!=cost_ts.end();++cit){
      cout<<"trg:"<<cit->first.print(tvoc)<<endl;
      for (wrd2cst::const_iterator wit=(cit->second)->begin();wit!=(cit->second)->end();++wit){
	cout <<"src:"<<wit->first.print(svoc)<<alUnitConst::unitDelim<<" costs:";
	for (vector<float>::const_iterator vec=wit->second.begin(); vec!=wit->second.end(); ++vec){cout<<" "<<*vec;}
	cout<<endl;
      }
    }
  }
  // HISTOGRAM PRUNING
  //adding nbest trg for each source
  if (verbose>0){cerr<< "done!\nPruning...";}
  hash_map<pair<alUnit,alUnit >,bool , hashfpairalunit, hasheqpairalunit> notpruned_st;
  hash_map<pair<alUnit,alUnit >,bool , hashfpairalunit, hasheqpairalunit> printed_st;
  for (hash_map<alUnit, wrd2cst*, hashfalunit, hasheqalunit>::const_iterator cit=cost_st.begin();cit!=cost_st.end();++cit){
    int cnt=0;
    wrd2cst::const_iterator wit=cit->second->begin();
    while (wit!=cit->second->end() && cnt<nbest){
      if (cnt<unbest){
	cout<<cit->first.print(svoc)<<cit->first.printDelim()<<wit->first.print(tvoc)<<wit->first.printDelim();
	for (vector<float>::const_iterator vec=wit->second.begin(); vec!=wit->second.end(); ++vec){
	  if (vec != wit->second.begin()){cout<<" ";}
	  cout<<*vec;
	}
	cout<<"\n";
	printed_st.insert(make_pair(make_pair(cit->first,wit->first),true));
      }else{
	notpruned_st.insert(make_pair(make_pair(cit->first,wit->first),true));
      }
      ++cnt;
      ++wit;
    }
  }
  //adding nbest source for each target (if not already added)
  for (hash_map<alUnit, wrd2cst*, hashfalunit, hasheqalunit>::const_iterator cit=cost_ts.begin();cit!=cost_ts.end();++cit){
    int cnt=0;
    wrd2cst::const_iterator wit=cit->second->begin();
    while (wit!=cit->second->end() && cnt<nbest){
      if (cnt<unbest){
	hash_map<pair<alUnit,alUnit >,bool , hashfpairalunit, hasheqpairalunit>::const_iterator np=printed_st.find(make_pair(wit->first,cit->first));
	if (np == printed_st.end()){ // it has not been printed before so we print it
	  cout<<wit->first.print(svoc)<<wit->first.printDelim()<<cit->first.print(tvoc)<<cit->first.printDelim();
	  for (vector<float>::const_iterator vec=wit->second.begin(); vec!=wit->second.end(); ++vec){
	    if (vec != wit->second.begin()){cout<<" ";}
	    cout<<*vec;
	  }
	  cout<<"\n";
	}
      }else{
	hash_map<pair<alUnit,alUnit >,bool , hashfpairalunit, hasheqpairalunit>::const_iterator np=notpruned_st.find(make_pair(wit->first,cit->first));
	if (np != notpruned_st.end()){ // we find it in notpruned_st so we print it
	  cout<<wit->first.print(svoc)<<wit->first.printDelim()<<cit->first.print(tvoc)<<cit->first.printDelim();
	  for (vector<float>::const_iterator vec=wit->second.begin(); vec!=wit->second.end(); ++vec){
	    if (vec != wit->second.begin()){cout<<" ";}
	    cout<<*vec;
	  }
	  cout<<"\n";
	}
      }
      ++cnt;
      ++wit;
    }
  }
  if (verbose>0){cerr<< "done!\n";}
}

