#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <cstring>
#include "math.h"

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif

using namespace std;
typedef unsigned long int wordIndex;

// DEFINE HASH-MAP FUNCTION

#define MODUL 1000000                                                                                  
#ifndef HASHPAIRWDIND
#define HASHPAIRWDIND

class hashfpairwdind : public unary_function<pair<wordIndex,wordIndex>, size_t>{
 public:
  size_t operator()(pair<wordIndex,wordIndex> const &p) const{
    wordIndex res=0;
    res += (size_t) p.first;
    res += (size_t) pow((double)p.second,double(2));
    return res%MODUL;
  }
};
                                                                                    
class hasheqpairwdind{
 public:
  bool operator()(pair<wordIndex,wordIndex> const &p1, pair<wordIndex,wordIndex> const &p2) const{
    return p1==p2;
  }
};

#endif
  
int main( int argc, char* argv[])
{
  // params
  string sntFileName;
  int verbose=1;
  bool doPhi=true;
  bool doLLR=true;
  bool pfeCondition=true;
  bool countMultiple=false;
  // read and assign command line arguments
  if ( argc ==1 ){
    cerr << "Usage:\n"<<"assoScoreTable_from-snt [-nopfe -multi -h -v verbose] -snt sntfile > output_table_file\n";
    cerr << "\t-nopfe    (s,t) word pairs don't have to satisfy the condition: p(s,t)>p(s)p(t)\n";
    cerr << "\t-multi    Every occurence of each word (instead of one per sentence) is counted and cooc(w1,w2)=min( occ(w1), occ(w2) )\n\n";
    exit(EXIT_FAILURE);
  }else{
    int i=1;
    while (i<argc){
      if (strcmp(argv[i],"-snt")==0){
	sntFileName = argv[i+1];				
	i+=2;
      }else if (strcmp(argv[i],"-nopfe")==0){
	pfeCondition = false;				
	++i;
      }else if (strcmp(argv[i],"-multi")==0){
	countMultiple = true;				
	++i;
      }else if (strcmp(argv[i],"-v")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>verbose;
 	i+=2;
     }else if (strcmp(argv[i],"-h")==0){
	cerr << "Usage:\n"<<"assoScoreTable_from-snt [-nopfe -multi -h -v verbose] -snt sntfile > output_table_file\n";
	cerr << "\t-nopfe    (s,t) word pairs don't have to satisfy the condition: p(s,t)>p(s)p(t)\n";
	cerr << "\t-multi    Every occurence of each word (instead of one per sentence) is counted and cooc(w1,w2)=min( occ(w1), occ(w2) )\n\n";
	exit(EXIT_FAILURE);
 	++i;
     }else{
	cerr << argv[i] << ": invalid option" << endl;
	exit(EXIT_FAILURE);
      }
    }
  } 

  ifstream sntFile(sntFileName.c_str());
  if (! sntFile){
    cerr << "Error while opening file:" << sntFileName << endl;
    exit(EXIT_FAILURE);
  }

  //Parse snt file and 
  unsigned long int lnum=0;
  unsigned long int sentPairNum=0;
  string line;
  vector<wordIndex> sent1,sent2;
  sent1.reserve(200);
  sent2.reserve(200);
  int freq=0;
  hash_map<wordIndex,int> words1;
  hash_map<wordIndex,int> words2;
  hash_map<wordIndex,pair<unsigned long int,unsigned long int> > count1; // pair: countunique, countmultiple
  hash_map<wordIndex,pair<unsigned long int,unsigned long int> > count2;
  hash_map<pair<wordIndex,wordIndex>,pair<unsigned long int,unsigned long int>, hashfpairwdind, hasheqpairwdind> count1_2;
  while (getline(sntFile,line)){
    ++lnum;
    if (lnum % 3 == 1){
      sent1.clear();
      sent2.clear();
      words1.clear();
      words2.clear();
      istringstream iss(line);
      iss >> freq;
      sentPairNum+=freq;
    }else if (lnum % 3 ==2){
      int buf;
      istringstream iss(line);
      while (iss >> buf){
	hash_map<wordIndex,int>::iterator w=words1.find(buf);
	if (w == words1.end()){
	  words1.insert(make_pair(buf,1));
	  sent1.push_back(buf);
	}else{
	  if (countMultiple) ++w->second;
	}
      }
    }else{
      int buf;
      istringstream iss(line);
      while (iss >> buf){
	hash_map<wordIndex,int>::iterator w=words2.find(buf);
	if (w == words2.end()){
	  words2.insert(make_pair(buf,1));
	  sent2.push_back(buf);
	}else{
	  //countmultiple: we count all occurencies of each word in the same sentence; number of coocurrences between a and b is min( occ(a), occ(b) )
	  //otherwise we count only one occurency of each word in the same sentence
	  if (countMultiple) ++w->second;
	}
      }
      if (verbose>2){
	cout<<"freq:"<<freq<<"\n";
	cout<<"sentPair1:";
	for(unsigned int i=0;i<sent1.size();++i){cout<<sent1[i]<<" ";}
	cout<<"\nsentPair2:";
 	for(unsigned int i=0;i<sent2.size();++i){cout<<sent2[i]<<" ";}
	cout<<"\n";
      }
      for (vector<wordIndex>::const_iterator i1=sent1.begin();i1!=sent1.end();++i1){
	// add count to word count table 1
	hash_map<wordIndex,pair<unsigned long int,unsigned long int> >::iterator c1=count1.find(*i1);
	if (c1 == count1.end()){
	  if (countMultiple) count1.insert(make_pair(*i1,make_pair(freq,freq*words1[*i1])));
	  else count1.insert(make_pair(*i1,make_pair(freq,0)));
	}else{
	  c1->second.first += freq;
	  if (countMultiple) c1->second.second += freq*words1[*i1];
	}
      }
      for (vector<wordIndex>::const_iterator i2=sent2.begin();i2!=sent2.end();++i2){
	// add count to word count table 2
	hash_map<wordIndex,pair<unsigned long int,unsigned long int> >::iterator c2=count2.find(*i2);
	if (c2 == count2.end()){
	  if (countMultiple) count2.insert(make_pair(*i2,make_pair(freq,freq*words2[*i2])));
	  else count2.insert(make_pair(*i2,make_pair(freq,0)));
	}else{
	  c2->second.first += freq;
	  if (countMultiple) c2->second.second += freq*words2[*i2];
	}
      }
      for (vector<wordIndex>::const_iterator i1=sent1.begin();i1!=sent1.end();++i1){
	for (vector<wordIndex>::const_iterator i2=sent2.begin();i2!=sent2.end();++i2){
	  // add count to word count table 1_2
	  hash_map<pair<wordIndex,wordIndex>,pair<unsigned long int,unsigned long int>, hashfpairwdind, hasheqpairwdind>::iterator c1_2=count1_2.find(make_pair(*i1,*i2));
	  int cooc(1);
	  if (countMultiple){
	    int occ1=words1[*i1];
	    int occ2=words2[*i2];
	    if (occ1 <= occ2) cooc=occ1;
	    else cooc=occ2;
	  }
	  if (c1_2 == count1_2.end()){
	    count1_2.insert(make_pair(make_pair(*i1,*i2),make_pair(freq,freq*cooc)));
	  }else{
	    c1_2->second.first += freq;
	    if (countMultiple) c1_2->second.second += freq*cooc;
	  }
	}
      } 

      if (verbose>2){
	for (hash_map<wordIndex,pair<unsigned long int,unsigned long int> >::const_iterator c1=count1.begin();c1!=count1.end();++c1){      
	  for (hash_map<wordIndex,pair<unsigned long int,unsigned long int> >::const_iterator c2=count2.begin();c2!=count2.end();++c2){
	    hash_map<pair<wordIndex,wordIndex>,pair<unsigned long int,unsigned long int>, hashfpairwdind, hasheqpairwdind>::const_iterator c1_2=count1_2.find(make_pair(c1->first,c2->first));
	    if (c1_2 != count1_2.end()){
	      cout<<"1:"<<c1->first<<"->unique "<<c1->second.first<<" multi "<<c1->second.second<<endl;
	      cout<<"2:"<<c2->first<<"->unique "<<c2->second.first<<" multi "<<c2->second.second<<endl;
	      if (countMultiple) cout<<" 1_2:"<<c1_2->second.second<<endl;
	      else cout<<" 1_2:"<<c1_2->second.first<<endl;
	    }
	  }
	}
	/*for (hash_map<pair<wordIndex,wordIndex>,pair<unsigned long int,unsigned long int>  , hashfpairwdind, hasheqpairwdind>::const_iterator c1_2=count1_2.begin();c1_2!=count1_2.end();++c1_2){
	  cout<<(c1_2->first).first<<"-"<<(c1_2->first).second<<":"<<c1_2->second<<endl;
	  
	  }*/
      }// if verbose>0
      
      //    jash_wdas[make_pair(src,trg)]= qscores;
    }
    if (verbose>0 && lnum % 10000==0){cerr<< ".";}
    if (verbose>0 && lnum % 100000==0){cerr<<lnum;}
  }

  // CALCULATE PHI-SQUARE SCORES
  const double INFIN=9999999999.0;
  const double TINY=1 - INFIN / (INFIN + 1);
  const float logTen=log(10.0);
  //const float logTINY=log(TINY);
  const float MinusLOGTINY=-1.0*log(TINY)/logTen;
  double N = sentPairNum;
  //hash_map<pair<wordIndex,wordIndex>,vector<double> , hashfpairwdind, hasheqpairwdind> scores1_2;
  double maxLLR=0; //keep max LLR score for posterior renormalization
  for (hash_map<pair<wordIndex,wordIndex>,pair<unsigned long int,unsigned long int>, hashfpairwdind, hasheqpairwdind>::const_iterator c1_2=count1_2.begin();c1_2!=count1_2.end();++c1_2){
    // First, calculate countUnique contingency table cells
    double a = c1_2->second.first;
    double b = count1[(c1_2->first).first].first-a;
    double c = count2[(c1_2->first).second].first-a;
    double d = N-a-b-c; // d is equal for countMultiple or countUnique: but it must be calculuted with countUnique counts
    if (countMultiple){
      a = c1_2->second.second;
      b = count1[(c1_2->first).first].second-a;
      c = count2[(c1_2->first).second].second-a;
      N = a+b+c+d;
    }
    double log_a_b = log(a+b);
    double log_a_c = log(a+c);
    double log_b_d = log(b+d);
    double log_c_d = log(c+d);
    double log_N;
    double pfe=a/N;
    double pf_pe=(a+b)*(a+c)/(N*N);
    if (doLLR){log_N= log(N);}
    //double phiSquare;
    double logPhi;
    double LLR;
    /*
      if ((c1_2->first).first==1097&&(c1_2->first).second==459 || (c1_2->first).first==9&&(c1_2->first).second==7){
      verbose=2;
      }
      else{
      verbose=0;
      doPhi=false;
      }
    */
    if (verbose>1){
      cout<<"\nLLR("<<(c1_2->first).first<<","<<(c1_2->first).second<<"):\n";
      cout<<"a:"<<a<<"\tb:"<<b<<"\t\ta+b:"<<(a+b)<<endl;
      cout<<"c:"<<c<<"\td:"<<d<<"\t\tc+d:"<<(c+d)<<endl;
      cout<<"a+c:"<<(a+c)<<"\tb+d:"<<(b+d)<<"\tN:"<<N<<"\n\n";
      cout<<"log(a):"<<log(a)<<"\t\tlog(b):"<<log(b)<<"\t\tlog(a+b):"<<log_a_b<<endl;
      cout<<"log(c):"<<log(c)<<"\t\tlog(d):"<<log(d)<<"\t\tlog(c+d):"<<log_c_d<<endl;
      cout<<"log(a+c):"<<log_a_c<<"\tlog(b+d):"<<log_b_d<<"\tlog(N):"<<log_N<<endl;
    }
    if (doPhi){
      double ad_bc = a*d-b*c;
      if (ad_bc<0){ad_bc=-1.0*ad_bc;} // gcc 4: abs don't work with double
      if ((a+b)==0 || (a+c)==0 || (b+d)==0 || (c+d)==0 || (ad_bc)==0 || (pfeCondition && pfe<pf_pe) ){
	//phiSquare = TINY;
	logPhi=MinusLOGTINY;
      }else{
	//phiSquare = (a*d-b*c)*(a*d-b*c) / ( (a+b)*(a+c)*(b+d)*(c+d) );
	logPhi = -1.0*(2*log(ad_bc)-log_a_b-log_a_c-log_b_d-log_c_d)/logTen;
	if (logPhi<0){logPhi=0;}
      }
      //cout<<" "<<phiSquare;
      //double logPhi = -1.0*log(phiSquare)/logTen;
      //if (! doLLR){
      cout<<(c1_2->first).first<<" "<<(c1_2->first).second<<" "<<logPhi;
      //}else{
      //scores1_2[c1_2->first].push_back(logPhi);
      //}
    }
    if (doLLR){
      if ((a+b)==0 || (a+c)==0 || (b+d)==0 || (c+d)==0 || (pfeCondition && pfe<pf_pe) ){
	LLR=TINY;
      }else{
	LLR=0;
	if (a!=0){LLR+=a*(log_N+log(a)-log_a_b-log_a_c);}
	if (verbose>1){cout <<"\na:"<<a<<"*("<<log_N<<"+"<<log(a)<<"-"<<log_a_b<<"-"<<log_a_c<<")="<<a<<"*"<<(log_N+log(a)-log_a_b-log_a_c)<<"="<<(a*(log_N+log(a)-log_a_b-log_a_c))<<" =>LLR:"<<LLR<<"\n";}
	if (b!=0){LLR+=b*(log_N+log(b)-log_a_b-log_b_d);}
	if (verbose>1){cout <<"\nb:"<<b<<"*("<<log_N<<"+"<<log(b)<<"-"<<log_a_b<<"-"<<log_b_d<<")="<<b<<"*"<<(log_N+log(b)-log_a_b-log_b_d)<<"="<<(b*(log_N+log(b)-log_a_b-log_b_d))<<" =>LLR:"<<LLR<<"\n";}
	if (c!=0){LLR+=c*(log_N+log(c)-log_c_d-log_a_c);}
	if (verbose>1){cout <<"\nc:"<<c<<"*("<<log_N<<"+"<<log(c)<<"-"<<log_c_d<<"-"<<log_a_c<<")="<<c<<"*"<<(log_N+log(c)-log_c_d-log_a_c)<<"="<<(c*(log_N+log(c)-log_c_d-log_a_c))<<" =>LLR:"<<LLR<<"\n";}
	if (d!=0){LLR+=d*(log_N+log(d)-log_c_d-log_b_d);}
	if (verbose>1){cout <<"\nd:"<<d<<"*("<<log_N<<"+"<<log(d)<<"-"<<log_c_d<<"-"<<log_b_d<<")="<<d<<"*"<<(log_N+log(d)-log_c_d-log_b_d)<<"="<<(d*(log_N+log(d)-log_c_d-log_b_d))<<" =>LLR:"<<LLR<<"\n";}
	LLR=LLR/logTen; //this is actually useless if afterwoods the scores will be normalized to form probabilities
	if (verbose>1){cout<<"LLR/ln(10):"<<LLR<<endl;}
	if (LLR>maxLLR){maxLLR=LLR;}	  
	//scores1_2[c1_2->first].push_back(LLR);
	//LLR=a*(log_N+log(a)-log_a_b-log_a_c)+b*(log_N+log(b)-log_a_b-log_b_d)+c*(log_N+log(c)-log_c_d-log_a_c)+d*(log_N+log(d)-log_c_d-log_b_d);
      }
      cout<<" "<<LLR;
      //print also occurences:
      //if (verbose>1){cout<<" "<<LLR<<" "<<a<<" "<<(a+b)<<" "<<(a+c);}
      //cout<<" "<<LLR<<" "<<a<<" "<<(a+b)<<" "<<(a+c);
      /*
      cout<<" "<<LLR<<" "<<pfe<<" "<<pf_pe;
      if (pfe<pf_pe){cout<<" <<<<<";}
      */
    }
    //if (verbose>1){cout<<"\n";}
    cout<<"\n";
  }
  cout<<"# maxLLR "<<maxLLR<<"\n";
  return 0;  
}

