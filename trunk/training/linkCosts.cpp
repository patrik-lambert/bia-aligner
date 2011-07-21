#include <time.h>
#include <sys/times.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <cstring>
#include "math.h"
#include "linkCluster.h"
#include "iword.h"
#include "phrase.h"

using namespace std;
struct tms *time_buffer = new tms;
float clocks_per_sec = sysconf (_SC_CLK_TCK);
//static double hashfTime;
//static double hasheqTime;
//static double nEq;

struct iphrase {
public:
  alUnit unit;
  unsigned int nocc;
  iphrase(formType _form, unsigned int _nocc):unit(_form), nocc(_nocc){};
  iphrase(const alUnit & _unit, unsigned int _nocc):unit(_unit), nocc(_nocc){};
  //    void addElem (wordIndex _form){
  //	unit.push_back(_form);
  //   }
};
struct wcount {
public:
  unsigned long int occs;
  vector<unsigned long int> vfert;
  wcount(unsigned long int _occs, unsigned int maxfert): occs(_occs), vfert(vector<unsigned long int>(maxfert+1,0)) {};
};

// assoCount: stores co-occurrence and link counts. To ensure that the number of links is never greater than the occurrence or co-occurrence count,
// the number of links taken into account depends on the occurrences of the source part (lnks1), the target part (lnks2) or the co-occurrences (lnks12)
struct assoCount {
public:
  unsigned long int cooc;
  unsigned long int lnks1;  
  unsigned long int lnks2;
  unsigned long int lnks12;
  assoCount(unsigned long int _cooc, unsigned long int _lnks1=0, unsigned long int _lnks2=0, unsigned long int _lnks12=0): cooc(_cooc), lnks1(_lnks1), lnks2(_lnks2), lnks12(_lnks12) {};
};

// DEFINE HASH FUNCTION   
#ifndef HASHPAIRCLFORM
#define HASHPAIRCLFORM

#define CLMODUL 5000000

class hashfpairclform : public unary_function<pair<clType,formType>, size_t>{
public:
  size_t operator()(pair<clType,formType> const &p) const{
    unsigned long long int res=0;
    res += (size_t) p.first*1277;
    res += (size_t) p.second*12781;
    return res%CLMODUL;
  }
};
class hasheqpairclform{
public:
  bool operator()(pair<clType,formType> const &p1, pair<clType,formType> const &p2) const{
    return p1==p2;
  }
};
#endif

int main( int argc, char* argv[])
{
  // params
  string sFileName,tFileName,stLinksFileName;
  string sRedFileName(""), tRedFileName(""),scFileName(""),tcFileName("");
  vocTable voc1,voc2,clVoc1,clVoc2, redVoc1, redVoc2;
  vocCorrespondence vocToRedVoc1,vocToRedVoc2; // for each voc index, give the correspondent reduced voc index
  string outSrcFertFileName(""),outTrgFertFileName(""),outSrcClassFertFileName(""),outTrgClassFertFileName("");
  bool fert(false),classFert(false);
  bool noSplitPhrases(false),probsAsReduced(false);
  int verbose=1;
  unsigned int maxfert=4;
  int backoffth=0;
  int minOccurPhrase=3;
  float discount=0.0;

  // read and assign command line arguments
  if ( argc ==1 ){
    cerr << "Usage:\n"<<"linkCosts [-sc srcClFile -tc trgClFile -osf outputSrcFert -otf outputTrgFert -oscf outputSrcClFert -otcf outputTrgClFert -maxf maxfert -v verbose -h] -s srcFile -t trgFile -st stLinksFile > output_table_file\n";
    cerr << "-s    f: input source file\n";
    cerr << "-t    f: input target file\n";
    cerr << "-sred    f: input source reduced file (e.g. lowercase or stems)\n";
    cerr << "-tred    f: input target reduced file (e.g. lowercase or stems)\n";
    cerr << "-st    f: input source-target links  file (links are: i-j separated by white space, i and j begin at 0)\n";
    cerr << "-sc    f: input source classes file\n";
    cerr << "-tc    f: input target classes file\n";
    cerr << "-osf    f: output source fertility table\n";
    cerr << "-otf    f: output target fertility table\n";
    cerr << "-oscf    f: output source fertility table depending on word's previous class\n";
    cerr << "-otcf    f: output target fertility table depending on word's previous class\n";
    cerr << "-maxf    i: maximum allowed fertility [default 4]\n";
    cerr << "-backoffth    i: min occurrences of {class, word} to use class fert costs instead of fert costs [default 0: not used]\n";
    cerr << "-minOccurPhrase    i: mininmum occurrence of a phrase to be considered as a candidate in second pass [default 3]\n";
    cerr << "-noSplitPhrases    In case of n-to-m links, considers one link for the link relative probs, instead of all links between subparts. [default: not used]\n";
    cerr << "                     If we have the links 0-1 1-1 and 0,1 are positions of a valid source phrase, considers '0 1'-1 only and not 0-1 neither 1-1\n";
    cerr << "-probsAsReduced   Output model with input source and target phrase entries but probabilities based on the counts in the reduced input files\n";
    cerr << "-d       float: discounting parameter for relative probabilities (P=links-d/cooc)\n";
    cerr << "-v    i: verbose level\n";
    exit(EXIT_FAILURE);
  }else{
    int i=1;
    while (i<argc){
      if (strcmp(argv[i],"-s")==0){
	sFileName = argv[i+1];				
	i+=2;
      }else if (strcmp(argv[i],"-t")==0){
	tFileName = argv[i+1];				
	i+=2;
      }else if (strcmp(argv[i],"-sred")==0){
	sRedFileName = argv[i+1];			
	i+=2;
      }else if (strcmp(argv[i],"-tred")==0){
	tRedFileName = argv[i+1];				
	i+=2;
      }else if (strcmp(argv[i],"-st")==0){
	stLinksFileName = argv[i+1];				
	i+=2;
      }else if (strcmp(argv[i],"-sc")==0){
	scFileName = argv[i+1];				
	i+=2;
      }else if (strcmp(argv[i],"-tc")==0){
	tcFileName = argv[i+1];				
	i+=2;
      }else if (strcmp(argv[i],"-osf")==0){
	outSrcFertFileName = argv[i+1];				
	fert=true;
	i+=2;
      }else if (strcmp(argv[i],"-otf")==0){
	outTrgFertFileName = argv[i+1];				
	fert=true;
	i+=2;
      }else if (strcmp(argv[i],"-oscf")==0){
	outSrcClassFertFileName = argv[i+1];				
	classFert=true;
	i+=2;
      }else if (strcmp(argv[i],"-otcf")==0){
	outTrgClassFertFileName = argv[i+1];				
	classFert=true;
	i+=2;
      }else if (strcmp(argv[i],"-maxf")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>maxfert;
	i+=2;
      }else if (strcmp(argv[i],"-backoffth")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>backoffth;
	i+=2;
      }else if (strcmp(argv[i],"-minOccurPhrase")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>minOccurPhrase;
	i+=2;
      }else if (strcmp(argv[i],"-noSplitPhrases")==0){
	noSplitPhrases=true;
	i++;
      }else if (strcmp(argv[i],"-probsAsReduced")==0){
	probsAsReduced=true;
	i++;
      }else if (strcmp(argv[i],"-d")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>discount;
 	i+=2;
      }else if (strcmp(argv[i],"-v")==0){
	istringstream sin;
	sin.str(argv[i+1]);
	sin>>verbose;
	i+=2;
      }else if (strcmp(argv[i],"-h")==0){
	cerr << "Usage:\n"<<"linkCosts [-sc srcClFile -tc trgClFile -osf outputSrcFert -otf outputTrgFert -oscf outputSrcClFert -otcf outputTrgClFert -maxf maxfert -v verbose -h] -s srcSntFile -t trgSntFile -st stLinksFile > output_table_file\n";
	cerr << "-s    f: input source file\n";
	cerr << "-t    f: input target file\n";
	cerr << "-sred    f: input source reduced file (e.g. lowercase or stems)\n";
	cerr << "-tred    f: input target reduced file (e.g. lowercase or stems)\n";
	cerr << "-st    f: input source-target links  file (links are: i-j separated by white space, i and j begin at 0)\n";
	cerr << "-sc    f: input source classes file\n";
	cerr << "-tc    f: input target classes file\n";
	cerr << "-osf    f: output source fertility table\n";
	cerr << "-otf    f: output target fertility table\n";
	cerr << "-oscf    f: output source fertility table depending on word's previous class\n";
	cerr << "-otcf    f: output target fertility table depending on word's previous class\n";
	cerr << "-maxf    i: maximum allowed fertility [default 4]\n";
	cerr << "-backoffth    i: min occurrences of {class, word} to use class fert costs instead of fert costs [default 0: not used]\n";
	cerr << "-minOccurPhrase    i: mininmum occurrence of a phrase to be considered as a candidate in second pass [default 3]\n";
	cerr << "-noSplitPhrases    In case of n-to-m links, considers one link for the link relative probs, instead of all links between subparts. [default: not used]\n";
	cerr << "                     If we have the links 0-1 1-1 and 0,1 are positions of a valid source phrase, considers '0 1'-1 only and not 0-1 neither 1-1\n";
	cerr << "-probsAsReduced   Output model with input text entries but probabilities based on the counts in the reduced input files [default: not used]\n";
	cerr << "-d       float: discounting parameter for relative probabilities (P=links-d/cooc) [default 0]\n";
	cerr << "-v    i: verbose level\n";
	exit(EXIT_FAILURE);
	++i;
      }else{
	cerr << argv[i] << ": invalid option" << endl;
	exit(EXIT_FAILURE);
      }
    }
  } 
  if (probsAsReduced && (sRedFileName=="" || tRedFileName=="")){
    cerr << "Error: 'probsAsReduced' option is set but one of the reduced input file is missing\n";
    exit(EXIT_FAILURE);
  }

  ifstream sFile(sFileName.c_str());
  if (! sFile){
    cerr << "Error while opening file:" << sFileName << endl;
    exit(EXIT_FAILURE);
  }
  ifstream tFile(tFileName.c_str());
  if (! tFile){
    cerr << "Error while opening file:" << tFileName << endl;
    exit(EXIT_FAILURE);
  }
  ifstream sRedFile(sRedFileName.c_str());
  if (sRedFileName != "" && ! sRedFile){
    cerr << "Error while opening file:" << sRedFileName << endl;
    exit(EXIT_FAILURE);
  }
  ifstream tRedFile(tRedFileName.c_str());
  if (tRedFileName != "" && ! tRedFile){
    cerr << "Error while opening file:" << tRedFileName << endl;
    exit(EXIT_FAILURE);
  }
  ifstream stFile(stLinksFileName.c_str());
  if (! stFile){
    cerr << "Error while opening file:" << stLinksFileName << endl;
    exit(EXIT_FAILURE);
  }
  ifstream scFile(scFileName.c_str());
  if (scFileName != "" && !scFile){
    cerr << "Error while opening file:" << scFileName << endl;
    exit(EXIT_FAILURE);
  }
  ifstream tcFile(tcFileName.c_str());
  if (tcFileName != "" && !tcFile){
    cerr << "Error while opening file:" << tcFileName << endl;
    exit(EXIT_FAILURE);
  }
  ofstream foutSrcFert;
  if ( outSrcFertFileName!="" ){
    foutSrcFert.open( outSrcFertFileName.data() );
    if( !foutSrcFert.is_open() ) {
      cerr << "LINKCOSTS: Error opening output file=" << outSrcFertFileName << endl;
      exit(0);
    }
  }
  ofstream foutTrgFert;
  if ( outTrgFertFileName!="" ){
    foutTrgFert.open( outTrgFertFileName.data() );
    if( !foutTrgFert.is_open() ) {
      cerr << "LINKCOSTS: Error opening output file=" << outTrgFertFileName << endl;
      exit(0);
    }
  }
  ofstream foutSrcClassFert;
  if ( outSrcClassFertFileName!="" ){
    foutSrcClassFert.open( outSrcClassFertFileName.data() );
    if( !foutSrcClassFert.is_open() ) {
      cerr << "LINKCOSTS: Error opening output file=" << outSrcClassFertFileName << endl;
      exit(0);
    }
  }
  ofstream foutTrgClassFert;
  if ( outTrgClassFertFileName!="" ){
    foutTrgClassFert.open( outTrgClassFertFileName.data() );
    if( !foutTrgClassFert.is_open() ) {
      cerr << "LINKCOSTS: Error opening output file=" << outTrgClassFertFileName << endl;
      exit(0);
    }
  }

  (void) times (time_buffer);
  double tstart = time_buffer->tms_utime / clocks_per_sec;


  // PARSE FILES TO LOAD PHRASE AND SUB-PHRASE HASHES
  // ************************************************
  if (verbose>0){cerr<<"Loading phrase and sub-phrase hashes...\n";}
  phraseDetection sPhSearch,tPhSearch;
  phraseDetection sRedPhSearch, tRedPhSearch;
  string line,tline,stline,buf,lbuf;
  string sRedLine,tRedLine;
  wordIndex bufInd;
  vector<jword> sent1,sent2; // the real sentence: useful to keep track of position
  vector<jword> redSent1,redSent2; // the real reduced sentence: useful to keep track of position
  
  // parse link file to get phrases
  //int ll=0;
  //while (getline(stFile,stline) && ll<10){
  while (getline(stFile,stline)){
    //++ll;
    getline(sFile,line);
    istringstream iss(line);
    sent1.clear();
    while (iss >> buf){
      bufInd=voc1.insert(buf);
      sent1.push_back(jword(bufInd,1));
    }
    getline(tFile,tline);
    istringstream isst(tline);
    sent2.clear();
    while (isst >> buf){
      bufInd=voc2.insert(buf);
      sent2.push_back(jword(bufInd,1));
    }

    // if probsAsReduced is true, we store normal and reduced phrases, and prune the normal phrases according to the reduced phrase counts
    if (probsAsReduced){
      getline(sRedFile,sRedLine);
      istringstream issreds(sRedLine);
      redSent1.clear();
      unsigned int cnt=0;
      while (issreds >> buf){
	bufInd=redVoc1.insert(buf);
	redSent1.push_back(jword(bufInd,1));
	vocToRedVoc1.insert(sent1.at(cnt).form(),bufInd);
	++cnt;
      }
      getline(tRedFile,tRedLine);
      istringstream issredt(tRedLine);
      redSent2.clear();
      cnt=0;
      while (issredt >> buf){
	bufInd=redVoc2.insert(buf);
	redSent2.push_back(jword(bufInd,1));
	vocToRedVoc2.insert(sent2.at(cnt).form(),bufInd);
	++cnt;
      }
    }
    linkClusterDiv clusts;
    istringstream issl(stline);
    while (issl >> lbuf){
      string::size_type sepInd=lbuf.find("-",0);
      int source=atoi(lbuf.substr(0,sepInd).c_str());
      int target=atoi(lbuf.substr(sepInd+1).c_str());
      clusts.addLink(source,target);
    }
    //cout<<clusts.print()<<endl;
    clusts.sortAndSplitIntoContiguous();
    clusts.storePhrases(sPhSearch,tPhSearch, sent1, sent2);
    if (probsAsReduced){
      clusts.storePhrases(sRedPhSearch,tRedPhSearch, redSent1, redSent2);
    }
  }

  if (verbose>2){
    cout<<"Source Phrase voc:\n"<<sPhSearch.printVoc(voc1)<<endl;
    cout<<"Target Phrase voc:\n"<<tPhSearch.printVoc(voc2)<<endl;
    cout<<"Source sub-phrase voc:\n"<<sPhSearch.printSubVoc(voc1)<<endl;
    cout<<"Target sub-phrase voc::\n"<<tPhSearch.printSubVoc(voc2)<<endl;
    cout<<"Reduced Source Phrase voc:\n"<<sRedPhSearch.printVoc(redVoc1)<<endl;
    cout<<"Reduced Target Phrase voc:\n"<<tRedPhSearch.printVoc(redVoc2)<<endl;
    cout<<"Reduced Source phrase voc:\n"<<sRedPhSearch.printSubVoc(redVoc1)<<endl;
    cout<<"Reduced Target sub-phrase voc::\n"<<tRedPhSearch.printSubVoc(redVoc2)<<endl;
  }
  if (probsAsReduced){
    sRedPhSearch.pruneVoc(minOccurPhrase);
    tRedPhSearch.pruneVoc(minOccurPhrase);
    sPhSearch.pruneVocAsReduced(sRedPhSearch,vocToRedVoc1); // prune voc to keep the entries which reduced form has not been pruned
    tPhSearch.pruneVocAsReduced(tRedPhSearch,vocToRedVoc2);
  }else{
    sPhSearch.pruneVoc(minOccurPhrase);
    tPhSearch.pruneVoc(minOccurPhrase);
  }
  if (verbose>2){
    cout<<"Source Phrase voc:\n"<<sPhSearch.printVoc(voc1)<<endl;
    cout<<"Target Phrase voc:\n"<<tPhSearch.printVoc(voc2)<<endl;
    cout<<"Source sub-phrase voc:\n"<<sPhSearch.printSubVoc(voc1)<<endl;
    cout<<"Target sub-phrase voc::\n"<<tPhSearch.printSubVoc(voc2)<<endl;
  }

  stFile.clear();
  stFile.seekg(0);
  sFile.clear();
  sFile.seekg(0);
  tFile.clear();
  tFile.seekg(0);
  // PARSE FILES AGAIN TO STORE COUNTS
  //**********************************
  if (verbose>0){cerr<<"Storing counts...\n";}
  unsigned long int lnum=0;
  // sentence-based containers (cleared at each sentence pair)
  vector<jword> uniqSent1,uniqSent2; // unique words contained in sentence (useful for fertility counts)
  vector<iphrase> uniqPhSent1,uniqPhSent2; // unique phrases contained in sentence (used for coocurrence counts)
  hash_map<formType,unsigned int> words1,words2;
  hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit> multiwords1,multiwords2; // phrases of more than one word found in sentences 1 and 2
  hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit> units1,units2; // hash with the uniq units of the sentence (words+multiwords)
  hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit> redUnits1,redUnits2; // these hashes are used to be able to count reduced co-occurrences
  hash_map<pair<alUnit,alUnit>,unsigned int, hashfpairalunit, hasheqpairalunit> sentLinks,redSentLinks; // link hash map reset at each sentence pair
  // global containers
  hash_map<formType,wcount> count1,count2;  // counts of unique words (for fertility)
  hash_map<formType,wcount> redCount1,redCount2;  
  hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit> phCount1,phCount2;  //counts of unique phrases (for cooccurrence)
  hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit> redPhCount1,redPhCount2;
  hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform> clCount1,clCount2,redClCount1,redClCount2;
  hash_map<pair<alUnit,alUnit>,assoCount, hashfpairalunit, hasheqpairalunit> count1_2, redCount1_2;
  
  //  double parseSentTime, countTime, linkCountTime, fertCountTime, mkpairTime, insertTime, findTime;
  //  while (getline(sFile,line) && lnum<1){
  while (getline(sFile,line)){
    ++lnum;
    //    (void) times (time_buffer); double parseSentBeg = time_buffer->tms_utime / clocks_per_sec;
//     if (lnum==16){verbose=2;}
//     else {verbose=1;}
    // PARSE SOURCE SENTENCE
    sentLinks.clear(); redSentLinks.clear();
    uniqSent1.clear();
    uniqPhSent1.clear();
    sent1.clear();
    words1.clear();
    multiwords1.clear();
    wordIndex bufInd;
    string buf;
    istringstream iss(line);
    unsigned int ind=0;
    //IMPORTANT: we count all occurencies of each word in the same sentence; number of coocurrences between a and b is cooc=min( occ(a), occ(b) )
    // since this would imply having probabilities > 1 when the number of links > min(occ(a),occ(b)), cooc(a,b)=max( nlinks(a,b) , min(occ(a),occ(b)) )
    while (iss >> buf){
      bufInd=voc1.insert(buf);
      hash_map<formType,unsigned int>::iterator w=words1.find(bufInd);
      if (w == words1.end()){
	words1.insert(make_pair(bufInd,ind));
	uniqSent1.push_back(jword(bufInd,1));
	uniqPhSent1.push_back(iphrase(bufInd,1));
	++ind;
      }else{
	++uniqSent1[w->second].nocc;
	++uniqPhSent1[w->second].nocc;
      }
      sent1.push_back(jword(bufInd,1));
    }
    // CALCULATE SOURCE PHRASES
    sPhSearch.searchSentence(sent1,multiwords1);
    for (hash_map<alUnit, unsigned int, hashfalunit,hasheqalunit>::const_iterator pit=multiwords1.begin();pit!=multiwords1.end();++pit){
      uniqPhSent1.push_back(iphrase(pit->first,pit->second));
    }
    // load source phrase hash
    units1.clear();
    for (vector<iphrase>::const_iterator i1=uniqPhSent1.begin();i1!=uniqPhSent1.end();++i1){
      hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::iterator it=units1.find(i1->unit);
      if (it==units1.end()){
	units1.insert(make_pair(i1->unit,i1->nocc));
      }else{
	it->second+=i1->nocc;
      }
    }

    // load reduced source phrases (necessary to count reduced co-occurrences in this phrase pair)
    if (probsAsReduced){
      redUnits1.clear();
      for (vector<iphrase>::const_iterator i1=uniqPhSent1.begin();i1!=uniqPhSent1.end();++i1){
	alUnit redUnit;
	i1->unit.reduce(redUnit,vocToRedVoc1);
	hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::iterator rit=redUnits1.find(redUnit);
	if (rit==redUnits1.end()){
	  redUnits1.insert(make_pair(redUnit,i1->nocc));
	}else{
	  rit->second+=i1->nocc;
	}
      }
    }

    // PARSE TARGET SENTENCE
    string tline;
    getline(tFile,tline);
    uniqSent2.clear();
    uniqPhSent2.clear();
    sent2.clear();
    words2.clear();
    multiwords2.clear();
    istringstream isst(tline);
    ind=0;
    while (isst >> buf){
      bufInd=voc2.insert(buf);
      hash_map<formType,unsigned int>::iterator w=words2.find(bufInd);
      if (w == words2.end()){
	words2.insert(make_pair(bufInd,ind));
	uniqSent2.push_back(jword(bufInd,1));
	uniqPhSent2.push_back(iphrase(bufInd,1));
	++ind;
      }else{
	++uniqSent2[w->second].nocc;
	++uniqPhSent2[w->second].nocc;
      }
      sent2.push_back(jword(bufInd,1));
    }
    // CALCULATE TARGET PHRASES
    tPhSearch.searchSentence(sent2,multiwords2);
    for (hash_map<alUnit, unsigned int, hashfalunit,hasheqalunit>::const_iterator pit=multiwords2.begin();pit!=multiwords2.end();++pit){
      uniqPhSent2.push_back(iphrase(pit->first,pit->second));
    }
    // load target phrase hash
    units2.clear();
    for (vector<iphrase>::const_iterator i2=uniqPhSent2.begin();i2!=uniqPhSent2.end();++i2){
      hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::iterator it=units2.find(i2->unit);
      if (it==units2.end()){
	units2.insert(make_pair(i2->unit,i2->nocc));
      }else{
	it->second+=i2->nocc;
      }
    }

    // load reduced target phrases
    if (probsAsReduced){
      redUnits2.clear();
      for (vector<iphrase>::const_iterator i2=uniqPhSent2.begin();i2!=uniqPhSent2.end();++i2){
	alUnit redUnit;
	i2->unit.reduce(redUnit,vocToRedVoc2);
	hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::iterator rit=redUnits2.find(redUnit);
	if (rit==redUnits2.end()){
	  redUnits2.insert(make_pair(redUnit,i2->nocc));
	}else{
	  rit->second+=i2->nocc;
	}
      }
    }

    if (verbose>1){
      //       cout<<"Unique sentPair1:";
      //       for(int i=0;i<uniqSent1.size();++i){cout<<uniqSent1[i].form()<<" ("<<uniqSent1[i].nocc<<") ";}
      cout<<"\nSentence pair:"<<lnum<<endl;
      cout<<"Sentence1:";
      for(unsigned int i=0;i<sent1.size();++i){cout<<i<<"|"<<voc1.form(sent1[i].form())<<" ";}
      cout<<"\nSource phrase unique sentence:"<<endl;
      for (vector<iphrase>::const_iterator p=uniqPhSent1.begin();p!=uniqPhSent1.end();++p){
	cout<<p->unit.print(voc1)<<"("<<p->nocc<<")|";
      }
      // cout<<"\nSource phrase hash:"<<endl;
      // for ( hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator p=units1.begin();p!=units1.end();++p){
      // 	cout<<p->first.print(voc1)<<"("<<p->second<<")|";
      // }
      cout<<endl;
      //       cout<<"\nUnique sentPair2:";
      //       for(int i=0;i<uniqSent2.size();++i){cout<<uniqSent2[i].form()<<" ("<<uniqSent2[i].nocc<<") ";}
      cout<<"\nSentence2:";
      for(unsigned int i=0;i<sent2.size();++i){cout<<i<<"|"<<voc2.form(sent2[i].form())<<" ";}
      cout<<"\nTarget phrase unique sentence:"<<endl;
      for (vector<iphrase>::const_iterator p=uniqPhSent2.begin();p!=uniqPhSent2.end();++p){
	cout<<p->unit.print(voc2)<<"("<<p->nocc<<")|";
      }
      // cout<<"\nTarget phrase hash:"<<endl;
      // for ( hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator p=units2.begin();p!=units2.end();++p){
      // 	cout<<p->first.print(voc2)<<"("<<p->second<<")|";
      // }
      cout<<endl;
    }
    //    (void) times (time_buffer); double parseSentEnd = time_buffer->tms_utime / clocks_per_sec; parseSentTime+=parseSentEnd-parseSentBeg; double countBeg = time_buffer->tms_utime / clocks_per_sec;

   // ADD COUNTS TO WORD COUNT TABLES (for fertility counts)
    for (vector<jword>::const_iterator i1=uniqSent1.begin();i1!=uniqSent1.end();++i1){
      if (probsAsReduced){
	wordIndex redInd=vocToRedVoc1.reduce(i1->form());
	hash_map<formType,wcount>::iterator rc1=redCount1.find(redInd);
	if (rc1 == redCount1.end()){
	  redCount1.insert(make_pair(redInd,wcount(i1->nocc,maxfert)));
	}else{
	  rc1->second.occs+=i1->nocc;
	}
      }
      // add count to word count table 1
      hash_map<formType,wcount>::iterator c1=count1.find(i1->form());
      if (c1 == count1.end()){
	count1.insert(make_pair(i1->form(),wcount(i1->nocc,maxfert)));
      }else{
	c1->second.occs+=i1->nocc;
      }
    }
    for (vector<jword>::const_iterator i2=uniqSent2.begin();i2!=uniqSent2.end();++i2){
      if (probsAsReduced){
	wordIndex redInd=vocToRedVoc2.reduce(i2->form());
	hash_map<formType,wcount>::iterator rc2=redCount2.find(redInd);
	if (rc2 == redCount2.end()){
	  redCount2.insert(make_pair(redInd,wcount(i2->nocc,maxfert)));
	}else{
	  rc2->second.occs+=i2->nocc;
	}
      }
      // add count to word count table 2
      hash_map<formType, wcount>::iterator c2=count2.find(i2->form());
      if (c2 == count2.end()){
	count2.insert(make_pair(i2->form(),wcount(i2->nocc,maxfert)));
      }else{
	c2->second.occs+=i2->nocc;
      }
    }
    // ADD COUNTS TO PHRASE COUNT TABLES (for cooccurrence counts)
    if (probsAsReduced){   // sum-up the reduced counts first, then insert the reduced counts in the original (not reduced table).
      for (hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator i1=redUnits1.begin();i1!=redUnits1.end();++i1){
	hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::iterator rc1=redPhCount1.find(i1->first);
	if (rc1 == redPhCount1.end()){
	  redPhCount1.insert(make_pair(i1->first,i1->second));
	}else{
	  rc1->second += i1->second;
	}
      }
    }
    for (vector<iphrase>::const_iterator i1=uniqPhSent1.begin();i1!=uniqPhSent1.end();++i1){
      // add count to phrase count table 1
      hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::iterator c1=phCount1.find(i1->unit);
      if (probsAsReduced){
	// alUnit reducedUnit;
	// i1->unit.reduce(reducedUnit,vocToRedVoc1);
	// hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::iterator rc1=redPhCount1.find(reducedUnit);

	// store empty entries in original (not reduced) table
	if (c1 == phCount1.end()){
	  phCount1.insert(make_pair(i1->unit,0)); 
	}
      }else{
	if (c1 == phCount1.end()){
	  phCount1.insert(make_pair(i1->unit,i1->nocc));
	}else{
	  c1->second+=i1->nocc;
	}
      }
    }
    if (probsAsReduced){   
      // sum-up the reduced counts first, then insert the reduced counts in the original (not reduced table).
      for (hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator i2=redUnits2.begin();i2!=redUnits2.end();++i2){
	hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::iterator rc2=redPhCount2.find(i2->first);
	if (rc2 == redPhCount2.end()){
	  redPhCount2.insert(make_pair(i2->first,i2->second));
	}else{
	  rc2->second += i2->second;
	}
      }
    }
    for (vector<iphrase>::const_iterator i2=uniqPhSent2.begin();i2!=uniqPhSent2.end();++i2){
      // add count to phrase count table 2
      hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::iterator c2=phCount2.find(i2->unit);
      if (probsAsReduced){
	//	alUnit reducedUnit;
	//	i2->unit.reduce(reducedUnit,vocToRedVoc2);
	//	hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::iterator rc2=redPhCount2.find(reducedUnit);

	// store empty entries in original (not reduced) table
	if (c2 == phCount2.end()){
	  phCount2.insert(make_pair(i2->unit,0)); 
	}
      }else{
	if (c2 == phCount2.end()){
	  phCount2.insert(make_pair(i2->unit,i2->nocc));
	}else{
	  c2->second+=i2->nocc;
	}
      }
    }

    if (verbose>1){
      cout<<"\nCounts of unique words, for fertility (count1):"<<endl;
      for (hash_map<formType, wcount>::const_iterator c1=count1.begin();c1!=count1.end();++c1){      
	cout<<voc1.form(c1->first)<<"("<<c1->second.occs<<") ";
      }
      cout<<endl;
      cout<<"Counts of unique phrases, for co-occurrence (phCount1):"<<endl;
      for (hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::const_iterator c1=phCount1.begin();c1!=phCount1.end();++c1){      
	cout<<c1->first.print(voc1)<<"("<<c1->second<<") ";
      }
      cout<<endl;
      cout<<"Counts of unique words, for fertility (count2):"<<endl;
      for (hash_map<formType, wcount>::const_iterator c2=count2.begin();c2!=count2.end();++c2){
	cout<<voc2.form(c2->first)<<"("<<c2->second.occs<<") ";
      }
      cout<<endl;
      cout<<"Counts of unique phrases, for co-occurrence (phCount2):"<<endl;
      for (hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::const_iterator c2=phCount2.begin();c2!=phCount2.end();++c2){
	cout<<c2->first.print(voc2)<<"("<<c2->second<<") ";
      }
      cout<<endl;
    }

    // CO-OCCURRENCE COUNTS
    if (probsAsReduced){
      // load counts in reduced co-occurrence hash
      for (hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator i1=redUnits1.begin();i1!=redUnits1.end();++i1){
	for (hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator i2=redUnits2.begin();i2!=redUnits2.end();++i2){
	  unsigned int cooc;
	  if (i1->second <= i2->second) cooc=i1->second;
	  else cooc=i2->second;
	  pair<alUnit,alUnit> pp(i1->first,i2->first);
	  hash_map<pair<alUnit,alUnit>,assoCount , hashfpairalunit, hasheqpairalunit>::iterator c1_2=redCount1_2.find(pp);
	  if (c1_2 == redCount1_2.end()){
	    redCount1_2.insert(make_pair(pp,assoCount(cooc)));
	  }else{
	    c1_2->second.cooc+=cooc;
	  }
	}
      }      
    }
    //(void) times (time_buffer); //double countEnd = time_buffer->tms_utime / clocks_per_sec; countTime+=countEnd-countBeg; double linkCountBeg = time_buffer->tms_utime / clocks_per_sec;
    for (vector<iphrase>::const_iterator i1=uniqPhSent1.begin();i1!=uniqPhSent1.end();++i1){
      for (vector<iphrase>::const_iterator i2=uniqPhSent2.begin();i2!=uniqPhSent2.end();++i2){
	unsigned int cooc;
	if (i1->nocc <= i2->nocc) cooc=i1->nocc;
	else cooc=i2->nocc;
	// add count to phrase count table 1_2
	pair<alUnit,alUnit> pp(i1->unit,i2->unit);
	//	(void) times (time_buffer);findBeg = time_buffer->tms_utime / clocks_per_sec;
	hash_map<pair<alUnit,alUnit>,assoCount , hashfpairalunit, hasheqpairalunit>::iterator c1_2=count1_2.find(pp);
	//(void) times (time_buffer);double findEnd = time_buffer->tms_utime / clocks_per_sec;findTime+=findEnd-findBeg;
	if (c1_2 == count1_2.end()){
	  //(void) times (time_buffer); double insertBeg = time_buffer->tms_utime / clocks_per_sec;
	  count1_2.insert(make_pair(pp,assoCount(cooc)));
	  //(void) times (time_buffer); double insertEnd = time_buffer->tms_utime / clocks_per_sec; insertTime+=insertEnd-insertBeg;
	}else{
	  c1_2->second.cooc+=cooc;
	}
      } 
    } 
	
    // LINK COUNTS
    linkClusterDiv clusts;
    string stline,lbuf;
    getline(stFile,stline);
    istringstream issl(stline);
    while (issl >> lbuf){
      string::size_type sepInd=lbuf.find("-",0);
      int source=atoi(lbuf.substr(0,sepInd).c_str());
      int target=atoi(lbuf.substr(sepInd+1).c_str());
      if (verbose>2){cout<<"lbuf:"<<lbuf<<" src:"<<source<<"("<<voc1.form(sent1[source].form())<<") trg:"<<target<<"("<<voc2.form(sent2[target].form())<<")"<<"("<<voc1.form(sent1[source].form())<<"-"<<voc2.form(sent2[target].form())<<")"<<endl;}
      clusts.addLink(source,target);
      if (fert || classFert){
	++(sent1[source].fert);
	++(sent2[target].fert);
      }
    }
    if (verbose>2){cout<<clusts.print()<<endl;}
    clusts.sortAndSplitIntoContiguous();
    if (verbose>1){cout<<clusts.print()<<endl;}
    if (verbose>1){cout<<"Links: ";}
    for (linkClusterDiv::iterator cl=clusts.begin(); cl!=clusts.end(); ++cl){
      vector<int> seq1, seq2;
      (*cl).sPhrase(seq1);
      (*cl).tPhrase(seq2);
      vector<alUnit> v1,v2;
      if (seq1.size()==1 || noSplitPhrases){
	alUnit u1;
	for (vector<int>::const_iterator i=seq1.begin();i!=seq1.end();++i){
	  u1.push_back(sent1[*i].form());
	}
	v1.push_back(u1);
      }else{
	vector<jword> clustsent1;
	hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit> clustphrases1;
	for (vector<int>::const_iterator i=seq1.begin();i!=seq1.end();++i){
	  clustsent1.push_back(sent1[*i]);
	  v1.push_back(alUnit(sent1[*i].form()));
	}
	sPhSearch.searchSentence(clustsent1,clustphrases1);
	for (hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator phit=clustphrases1.begin();phit!=clustphrases1.end();++phit){
	  v1.push_back(phit->first);
	}
      }
      if (seq2.size()==1 || noSplitPhrases){
	alUnit u2;
	for (vector<int>::const_iterator i=seq2.begin();i!=seq2.end();++i){
	  u2.push_back(sent2[*i].form());
	}
	v2.push_back(u2);
      }else{
	vector<jword> clustsent2;
	hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit> clustphrases2;
	for (vector<int>::const_iterator i=seq2.begin();i!=seq2.end();++i){
	  clustsent2.push_back(sent2[*i]);
	  v2.push_back(alUnit(sent2[*i].form()));
	}
	tPhSearch.searchSentence(clustsent2,clustphrases2);
	for (hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator phit=clustphrases2.begin();phit!=clustphrases2.end();++phit){
	  v2.push_back(phit->first);
	}
      }
      // We first store link counts in sentence-based hashes
      // Then we load them in the global hash depending on occurrence and co-occurrence counts (to ensure that lnks<=occ and lnks<=cooc)
      for (vector<alUnit>::const_iterator vit1=v1.begin();vit1!=v1.end();++vit1){
	for (vector<alUnit>::const_iterator vit2=v2.begin();vit2!=v2.end();++vit2){
	  if (probsAsReduced){
	    alUnit redUnit1,redUnit2;
	    vit1->reduce(redUnit1,vocToRedVoc1);
	    vit2->reduce(redUnit2,vocToRedVoc2);
	    hash_map<pair<alUnit,alUnit>, unsigned int, hashfpairalunit, hasheqpairalunit>::iterator c1_2=redSentLinks.find(make_pair(redUnit1,redUnit2));
	    if (verbose>1){cout<<redUnit1.print(voc1)<<"-"<<redUnit2.print(voc2)<<" | ";}
	    if (c1_2 == redSentLinks.end()){
	      redSentLinks.insert(make_pair(make_pair(redUnit1,redUnit2),1));
	    }else{
	      ++c1_2->second;
	    }
	  }else{
	    hash_map<pair<alUnit,alUnit>, unsigned int, hashfpairalunit, hasheqpairalunit>::iterator c1_2=sentLinks.find(make_pair(*vit1,*vit2));
	    if (verbose>1){cout<<vit1->print(voc1)<<"-"<<vit2->print(voc2)<<" | ";}
	    if (c1_2 == sentLinks.end()){
	      sentLinks.insert(make_pair(make_pair(*vit1,*vit2),1));
	    }else{
	      ++c1_2->second;
	    }
	  }
	}
      }
    } // for linkClusterDiv
    if (verbose>1){cout<<endl;}
    //(void) times (time_buffer); double linkCountEnd = time_buffer->tms_utime / clocks_per_sec; linkCountTime+=linkCountEnd-linkCountBeg;

    // Load link counts in (red)count1_2 hash, 
    // ensuring that nlinks <= ncooc and that for source and target part of link, nlinks <= nocc (so that relative probabilities are never greater than 1)
    if (probsAsReduced){
      for (hash_map<pair<alUnit,alUnit>, unsigned int, hashfpairalunit, hasheqpairalunit>::const_iterator rsl=redSentLinks.begin(); rsl!=redSentLinks.end();++rsl){
	unsigned int oc1(0),oc2(0);
	hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator rp1=redUnits1.find(rsl->first.first);
	if (rp1 != redUnits1.end()){oc1=rp1->second;}
	hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator rp2=redUnits2.find(rsl->first.second);
	if (rp2 != redUnits2.end()){oc2=rp2->second;}	      
	hash_map<pair<alUnit,alUnit>,assoCount, hashfpairalunit, hasheqpairalunit>::iterator c1_2=redCount1_2.find(rsl->first);
        if (c1_2 == redCount1_2.end()){
	  // This can happen because we pruned the hashes
	}else{
	  unsigned int lnks=rsl->second;
	  if (oc1<lnks) c1_2->second.lnks1+=oc1;
	  else c1_2->second.lnks1+=lnks; 
	  if (oc2<lnks) c1_2->second.lnks2+=oc2;
	  else c1_2->second.lnks2+=lnks;
	  unsigned int cooc;
	  if (oc1<=oc2) cooc=oc1;
	  else cooc = oc2;
	  if (cooc<lnks) c1_2->second.lnks12+=cooc;
	  else c1_2->second.lnks12+=lnks;
	}
      } //for
    }else{
      for (hash_map<pair<alUnit,alUnit>, unsigned int, hashfpairalunit, hasheqpairalunit>::const_iterator sl=sentLinks.begin(); sl!=sentLinks.end();++sl){
	unsigned int oc1(0),oc2(0);
	hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator p1=units1.find(sl->first.first);
	if (p1 != units1.end()){oc1=p1->second;}
	hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit>::const_iterator p2=units2.find(sl->first.second);
	if (p2 != units2.end()){oc2=p2->second;}	      
	hash_map<pair<alUnit,alUnit>,assoCount, hashfpairalunit, hasheqpairalunit>::iterator c1_2=count1_2.find(sl->first);
        if (c1_2 == count1_2.end()){
	  // This can happen because we pruned the hashes
	}else{
	  unsigned int lnks=sl->second;
	  if (oc1<lnks) c1_2->second.lnks1+=oc1;
	  else c1_2->second.lnks1+=lnks; 
	  if (oc2<lnks) c1_2->second.lnks2+=oc2;
	  else c1_2->second.lnks2+=lnks;
	  unsigned int cooc;
	  if (oc1<=oc2) cooc=oc1;
	  else cooc = oc2;
	  if (cooc<lnks) c1_2->second.lnks12+=cooc;
	  else c1_2->second.lnks12+=lnks;
	}
      }
    }
    
    // FERTILITY COUNTS
    //    (void) times (time_buffer);double fertCountBeg = time_buffer->tms_utime / clocks_per_sec;

    // load fertility counts in (red)count1,2 hashes
    if (fert){
      for (vector<jword>::const_iterator w1=sent1.begin();w1!=sent1.end();++w1){
	unsigned int fert=w1->fert;
	if (fert > maxfert) fert=maxfert;
	if (probsAsReduced){
	  wordIndex redInd = vocToRedVoc1.reduce(w1->form());
	  hash_map<formType, wcount>::iterator rc1=redCount1.find(redInd);
	  if (rc1 == redCount1.end()){
	    cerr<<"ERROR: word not present in redCount1\n";exit(EXIT_FAILURE);
	  }else{
	    ++rc1->second.vfert[fert];
	  }
	}else{
	  hash_map<formType, wcount>::iterator c1=count1.find(w1->form());
	  if (c1 == count1.end()){
	    cerr<<"ERROR: word not present in count1\n";exit(EXIT_FAILURE);
	  }else{
	    ++c1->second.vfert[fert];
	  }
	}
      }
      for (vector<jword>::const_iterator w2=sent2.begin();w2!=sent2.end();++w2){
	unsigned int fert=w2->fert;
	if (fert > maxfert) fert=maxfert;
	if (probsAsReduced){
	  wordIndex redInd = vocToRedVoc2.reduce(w2->form());
	  hash_map<formType, wcount>::iterator rc2=redCount2.find(redInd);
	  if (rc2 == redCount2.end()){
	    cerr<<"ERROR: word not present in redCount2\n";exit(EXIT_FAILURE);
	  }else{
	    ++rc2->second.vfert[fert];
	  }
	}else{
	  hash_map<formType, wcount>::iterator c2=count2.find(w2->form());
	  if (c2 == count2.end()){
	    cerr<<"ERROR: word not present in count2\n";exit(EXIT_FAILURE);
	  }else{
	    ++c2->second.vfert[fert];
	  }
	}
      }
      if (verbose>2){
	cout<<"FERT sentPair1:";
	for(unsigned int i=0;i<sent1.size();++i){cout<<sent1[i].form()<<" ("<<sent1[i].fert<<") ";}
	for (hash_map<formType, wcount>::const_iterator c=count1.begin();c!=count1.end();++c){
	  cout<<c->first<<" "<<c->second.vfert[0]<<" "<<c->second.vfert[1]<<" "<<c->second.vfert[2]<<" "<<c->second.vfert[3]<<" "<<c->second.vfert[4]<<endl;
	}
	cout<<"\nFERT sentPair2:";
	for(unsigned int i=0;i<sent2.size();++i){cout<<sent2[i].form()<<" ("<<sent2[i].fert<<") ";}
	for (hash_map<formType, wcount>::const_iterator c=count2.begin();c!=count2.end();++c){
	  cout<<c->first<<" "<<c->second.vfert[0]<<" "<<c->second.vfert[1]<<" "<<c->second.vfert[2]<<" "<<c->second.vfert[3]<<" "<<c->second.vfert[4]<<endl;
	}
	cout<<"\n";
      }
    }
    if (classFert){
      // READ CLASSES
      string scline;
      getline(scFile,scline);
      istringstream isssc(scline);
      string clbuf;
      wordIndex clbufInd;
      ind=0;
      while (isssc >> clbuf){
	clbufInd=clVoc1.insert(clbuf);
	sent1[ind].setCl(clbufInd);
	++ind;
      }
	    
      string tcline;
      getline(tcFile,tcline);
      istringstream isstc(tcline);
      ind=0;
      while (isstc >> clbuf){
	clbufInd=clVoc2.insert(clbuf);
	sent2[ind].setCl(clbufInd);
	++ind;
      }
      // LOAD HASHES
      for (unsigned int i=0;i<sent1.size();++i){
	unsigned int fert=sent1[i].fert;
	if (fert > maxfert) fert=maxfert;
	clType cl;
	if (i==0){cl=1;}
	else {cl=sent1[i-1].cl();}
	if (probsAsReduced){
	  wordIndex redInd=vocToRedVoc1.reduce(sent1.at(i).form());
	  hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::iterator rclc=redClCount1.find(make_pair(cl,redInd));
	  if (rclc==redClCount1.end()){
	    vector<unsigned long int> vfert(maxfert+1,0);
	    ++vfert[fert];
	    redClCount1.insert(make_pair(make_pair(cl,redInd),vfert));
	  }else{
	    ++(rclc->second.at(fert));
	  }
	}
	hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::iterator clc=clCount1.find(make_pair(cl,sent1[i].form()));
	if (clc==clCount1.end()){
	  vector<unsigned long int> vfert(maxfert+1,0);
	  ++vfert[fert];
	  clCount1.insert(make_pair(make_pair(cl,sent1[i].form()),vfert));
	}else{
	  ++(clc->second.at(fert));
	}
      }
      if (verbose>2){
	cout<<"sent1 class fert:";
	for (vector<jword>::const_iterator s=sent1.begin();s!=sent1.end();++s){
	  cout<<s->form()<<"|"<<s->cl()<<" ("<<s->fert<<") ";
	}
	cout<<endl;
	for (hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::const_iterator clc=clCount1.begin();clc!=clCount1.end();++clc){
	  cout<<clc->first.first<<","<<clc->first.second<<":";
	  for (vector<unsigned long int>::const_iterator fit=clc->second.begin();fit!=clc->second.end();++fit){cout<<*fit<<" ";}
	  cout<<endl;
	}
      }
      for (unsigned int i=0;i<sent2.size();++i){
	unsigned int fert=sent2[i].fert;
	if (fert > maxfert) fert=maxfert;
	clType cl;
	if (i==0){cl=1;}
	else {cl=sent2[i-1].cl();}
	if (probsAsReduced){
	  wordIndex redInd=vocToRedVoc2.reduce(sent2.at(i).form());
	  hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::iterator rclc=redClCount2.find(make_pair(cl,redInd));
	  if (rclc==redClCount2.end()){
	    vector<unsigned long int> vfert(maxfert+1,0);
	    ++vfert[fert];
	    redClCount2.insert(make_pair(make_pair(cl,redInd),vfert));
	  }else{
	    ++(rclc->second.at(fert));
	  }
	}
	hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::iterator clc=clCount2.find(make_pair(cl,sent2[i].form()));
	if (clc==clCount2.end()){
	  vector<unsigned long int> vfert(maxfert+1,0);
	  ++vfert[fert];
	  clCount2.insert(make_pair(make_pair(cl,sent2[i].form()),vfert));
	}else{
	  ++clc->second.at(fert);
	}
      }
      if (verbose>2){
	cout<<"sent2 class fert:";
	for (vector<jword>::const_iterator s=sent2.begin();s!=sent2.end();++s){
	  cout<<s->form()<<"|"<<s->cl()<<" ("<<s->fert<<") ";
	}
	cout<<endl;
	for (hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::const_iterator clc=clCount2.begin();clc!=clCount2.end();++clc){
	  cout<<clc->first.first<<","<<clc->first.second<<":";
	  for (vector<unsigned long int>::const_iterator fit=clc->second.begin();fit!=clc->second.end();++fit){cout<<*fit<<" ";}
	  cout<<endl;
	}
      }
    }// if classFert
	
    if (verbose>2){
      for (hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::const_iterator c1=phCount1.begin();c1!=phCount1.end();++c1){      
	for (hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::const_iterator c2=phCount2.begin();c2!=phCount2.end();++c2){
	  hash_map<pair<alUnit,alUnit>,assoCount, hashfpairalunit, hasheqpairalunit>::const_iterator c1_2=count1_2.find(make_pair(c1->first,c2->first));
	  if (c1_2 != count1_2.end()){
	    if ((c1_2->second.lnks1 == c1_2->second.lnks2) && (c1_2->second.lnks1 == c1_2->second.lnks12)){}
	    else{
	      cout<<c1->first.print(voc1)<<" ("<<c1->second<<"), ";
	      cout<<c2->first.print(voc2)<<" ("<<c2->second<<") -> ";
	      cout<<"cooc:"<<c1_2->second.cooc<<" links1 links2 links12:"<<c1_2->second.lnks1<<" "<<c1_2->second.lnks2<<" "<<c1_2->second.lnks12<<endl;
	    }
	  }
	}
      }
    }// if verbose>0
    if (verbose>0 && lnum % 100000==0){cerr<<lnum;}
    else if (verbose>0 && lnum % 10000==0){cerr<< "*";}
    else if (verbose>0 && lnum % 1000==0){cerr<< ".";}
    //    (void) times (time_buffer); double fertCountEnd = time_buffer->tms_utime / clocks_per_sec; fertCountTime+=fertCountEnd-fertCountBeg;
  }
  
  if (probsAsReduced){
    // load phCount1, phCount2 hashes with reduced counts
    // note that it is less costly to do it here than for every pair in count1_2 (in the loop to calculate probs)
    for (hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::iterator c1=phCount1.begin();c1!=phCount1.end();++c1){
      alUnit reducedUnit;
      c1->first.reduce(reducedUnit,vocToRedVoc1);
      hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::iterator rc1=redPhCount1.find(reducedUnit);
      if (rc1==redPhCount1.end()){
	cerr<<"Reduced unit not found in hash\n";
	exit(EXIT_FAILURE);
      }else{
	c1->second=rc1->second;
      }
    }
    for (hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::iterator c2=phCount2.begin();c2!=phCount2.end();++c2){
      alUnit reducedUnit;
      c2->first.reduce(reducedUnit,vocToRedVoc2);
      hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::iterator rc2=redPhCount2.find(reducedUnit);
      if (rc2==redPhCount2.end()){
	cerr<<"Reduced unit not found in hash\n";
	exit(EXIT_FAILURE);
      }else{
	c2->second=rc2->second;
      }
    }
    if (verbose>1){
      cout<<"Counts of unique phrases, for co-occurrence (phCount1):"<<endl;
      for (hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::const_iterator c1=phCount1.begin();c1!=phCount1.end();++c1){
	cout<<c1->first.print(voc1)<<"("<<c1->second<<") ";
      }
      cout<<endl;
      cout<<"Counts of unique phrases, for co-occurrence (phCount2):"<<endl;
      for (hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::const_iterator c2=phCount2.begin();c2!=phCount2.end();++c2){
	cout<<c2->first.print(voc2)<<"("<<c2->second<<") ";
      }
      cout<<endl;
    }
    // load count1, count2 hashes with reduced counts
    // this step could be avoided by adding several "if (probsAsReduced)" conditions in the fertility and back-off class fertility cost calculations
    // since going through count1 and count2 is not expensive compared to going through count1_2, we prefer to update count1 and count2 here once for all
    if (fert){
      for (hash_map<formType,wcount>::iterator c=count1.begin();c!=count1.end();++c){
	  wordIndex redInd=vocToRedVoc1.reduce(c->first);
	  hash_map<formType,wcount>::const_iterator rc=redCount1.find(redInd);
	  if (rc==redCount1.end()){
	    cerr<<"Reduced index not found in hash\n";
	    exit(EXIT_FAILURE);
	  }else{
	    c->second=rc->second;
	  }
     }
     for (hash_map<formType,wcount>::iterator c=count2.begin();c!=count2.end();++c){
	 wordIndex redInd=vocToRedVoc2.reduce(c->first);
	 hash_map<formType,wcount>::const_iterator rc=redCount2.find(redInd);
	 if ( rc == redCount2.end() ){
	   cerr<<"Reduced index not found in hash\n";
	   exit(EXIT_FAILURE);
	 }else{
	   c->second=rc->second;
	 }
       }
    }
    if (classFert){
      for (hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::iterator c=clCount1.begin();c!=clCount1.end();++c){
	  wordIndex redInd=vocToRedVoc1.reduce(c->first.second);
	  hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::const_iterator rc=redClCount1.find(make_pair(c->first.first,redInd));
	  if (rc==redClCount1.end()){
	    cerr<<"(class,reduced index) pair not found in hash\n";
	    exit(EXIT_FAILURE);
	  }else{
	    c->second=rc->second;
	  }
     }
      for (hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::iterator c=clCount2.begin();c!=clCount2.end();++c){
	  wordIndex redInd=vocToRedVoc2.reduce(c->first.second);
	  hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::const_iterator rc=redClCount2.find(make_pair(c->first.first,redInd));
	  if (rc==redClCount2.end()){
	    cerr<<"(class,reduced index) pair not found in hash\n";
	    exit(EXIT_FAILURE);
	  }else{
	    c->second=rc->second;
	  }
      }
    }
  }
  //  cerr<<"\nInsert:"<<insertTime<<endl; cerr<<"Find:"<<findTime<<endl; cerr<<"Hashf:"<<hashfTime<<endl; cerr<<"Hasheq:"<<hasheqTime<<endl; cerr<<"n hashEq:"<<nEq<<endl; cerr<<"Parsing sentences (and getting phrases):"<<parseSentTime<<endl; cerr<<"Occurrence counts:"<<countTime<<endl; 
  //  cerr<<"Co-occurrence and link counts:"<<linkCountTime<<endl; 
  //cerr<<"Fertility counts:"<<fertCountTime<<endl;
  //  (void) times (time_buffer); double tableCalcBeg = time_buffer->tms_utime / clocks_per_sec;

  if (verbose>0){cerr<<"\nOutputting table...\n";}
  const double INFIN=9999999999.0;
  const double TINY=1 - INFIN / (INFIN + 1);
  const float logTen=log(10.0);
  const float MinusLOGTINY=-1.0*log(TINY)/logTen;
  for (hash_map<pair<alUnit,alUnit>,assoCount , hashfpairalunit, hasheqpairalunit>::const_iterator c1_2=count1_2.begin();c1_2!=count1_2.end();++c1_2){
    double cooc,lnks1,lnks2,lnks12;
    if (probsAsReduced){
      alUnit redUnit1,redUnit2;
      c1_2->first.first.reduce(redUnit1,vocToRedVoc1);
      c1_2->first.second.reduce(redUnit2,vocToRedVoc2);
      hash_map<pair<alUnit,alUnit>,assoCount , hashfpairalunit, hasheqpairalunit>::const_iterator rc1_2=redCount1_2.find(make_pair(redUnit1,redUnit2));
      if (rc1_2 == redCount1_2.end()){
	cerr << "Error: pair of reduced units not found in hash\n";
	exit(EXIT_FAILURE);
      }else{
	cooc = rc1_2->second.cooc;
	lnks1 = rc1_2->second.lnks1;
	lnks2 = rc1_2->second.lnks2;
	lnks12 = rc1_2->second.lnks12;
      }
    }else{
      cooc = c1_2->second.cooc;
      lnks1 = c1_2->second.lnks1;
      lnks2 = c1_2->second.lnks2;
      lnks12 = c1_2->second.lnks12;
    }
    hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::const_iterator c1=phCount1.find(c1_2->first.first);
    double occ1 = c1->second;
    hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit>::const_iterator c2=phCount2.find(c1_2->first.second);
    double occ2 = c2->second;
    float disc=discount;
    if (occ1 <= discount && occ2 <= discount){
      if (occ1<occ2){disc=occ1-0.1;}
      else{disc=occ2-0.1;}
    }
    double prob = (lnks12-disc)*1.0/cooc*1.0;
    double odds = (lnks12+1.0)/(cooc-lnks12+1.0);
    double stProb = (lnks1-disc)*1.0/occ1*1.0;
    double tsProb = (lnks2-disc)*1.0/occ2*1.0;
	
    double cost;
    if (prob<TINY){cost=MinusLOGTINY;}
    else {cost=-log(prob)/logTen;}
    double oddsCost;
    if (odds<TINY){oddsCost=MinusLOGTINY;}
    else {oddsCost=-log(odds)/logTen;}
    double stCost;
    if (stProb<TINY){stCost=MinusLOGTINY;}
    else {stCost=-log(stProb)/logTen;}
    double tsCost;
    if (tsProb<TINY){tsCost=MinusLOGTINY;}
    else {tsCost=-log(tsProb)/logTen;}

    cout<<(c1_2->first).first.print(voc1)<<(c1_2->first).first.printDelim()<<(c1_2->first).second.print(voc2)<<(c1_2->first).second.printDelim();
    if (lnks12>cooc){
      cout<<"ERROR: lnks12 ("<<lnks12<<") > cooc ("<<cooc<<") for "<<(c1_2->first).first.print(voc1)<<(c1_2->first).first.printDelim()<<(c1_2->first).second.print(voc2)<<endl;
      exit(EXIT_FAILURE);
    }
    if (lnks1>occ1){
      cout<<"ERROR: lnks1 ("<<lnks1<<") > occ1 ("<<occ1<<") for "<<(c1_2->first).first.print(voc1)<<(c1_2->first).first.printDelim()<<(c1_2->first).second.print(voc2)<<endl;
      exit(EXIT_FAILURE);
    }
    if (lnks2>occ2){
      cout<<"ERROR: lnks2 ("<<lnks2<<") > occ2 ("<<occ2<<") for "<<(c1_2->first).first.print(voc1)<<(c1_2->first).first.printDelim()<<(c1_2->first).second.print(voc2)<<endl;
      exit(EXIT_FAILURE);
    }
    if (verbose>1){cout<<"(lnks1 lnks2 lnks12 oc1 oc2 cooc (prob):"<<lnks1<<" "<<lnks2<<" "<<lnks12<<" "<<occ1<<" "<<occ2<<" "<<cooc<<" ("<<prob<<") |"<<cost<<" "<<stCost<<" "<<tsCost<<" "<<oddsCost<<endl;}
    cout<<cost<<" "<<stCost<<" "<<tsCost<<" "<<oddsCost<<endl;
  }
  
  if (fert){
    // SRC FERTILITY TABLE
    for (hash_map<formType,wcount>::const_iterator c=count1.begin();c!=count1.end();++c){
      double lnks=0;
      foutSrcFert<<voc1.form(c->first);
	for (vector<unsigned long int>::const_iterator f=c->second.vfert.begin();f!=c->second.vfert.end();++f){
	  lnks+=*f;
	}
	for (unsigned int ic=0; ic<maxfert+1;++ic){
	  float cost=0;
	  for (unsigned int ifert=0; ifert<maxfert+1; ++ifert){
	    cost+=c->second.vfert[ifert]*1.0*abs((int)(ic-ifert))/lnks;
	    //foutSrcFert<<ifert<<" prob:"<<(c->second.vfert[ifert]/lnks)<<" "<<"ic:"<<ic<<" +cost:"<<(c->second.vfert[ifert]*1.0*abs(ic-ifert)/lnks)<<" cost:"<<cost<<endl;
	  }
	  foutSrcFert<<" "<<cost;
	  //foutSrcFert<<"COST: "<<cost<<endl;
	}
      foutSrcFert<<endl;
    }
    // TRG FERTILITY TABLE
    for (hash_map<formType,wcount>::const_iterator c=count2.begin();c!=count2.end();++c){
      double lnks=0;
      foutTrgFert<<voc2.form(c->first);
	for (vector<unsigned long int>::const_iterator f=c->second.vfert.begin();f!=c->second.vfert.end();++f){
	  lnks+=*f;
	}
	for (unsigned int ic=0; ic<maxfert+1;++ic){
	  float cost=0;
	  for (unsigned int ifert=0; ifert<maxfert+1; ++ifert){
	    cost+=c->second.vfert[ifert]*1.0*abs((int)(ic-ifert))/lnks;
	  }
	  foutTrgFert<<" "<<cost;
	}
	foutTrgFert<<endl;
      }
   }
   if (classFert){
    // SRC CLASS FERTILITY TABLE
    for (hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::const_iterator c=clCount1.begin();c!=clCount1.end();++c){
      double lnks=0;
      vector<unsigned long int> vfert=c->second;
      //foutSrcClassFert<<"vfert:";
      for (vector<unsigned long int>::const_iterator f=vfert.begin();f!=vfert.end();++f){
	//foutSrcClassFert<<*f<<" ";
	lnks+=*f;
      }
      //foutSrcClassFert<<endl;
      if (lnks < backoffth*1.0){
	hash_map<formType,wcount>::const_iterator cbo=count1.find(c->first.second);
	if (cbo==count1.end()){
	  cerr<<"ERROR: backoff probabilities not found\n";exit(0);
	}else{
	  //foutSrcClassFert<<"backoff"<<endl;
	  vfert=cbo->second.vfert;
	  lnks=0;
	  for (vector<unsigned long int>::const_iterator f=vfert.begin();f!=vfert.end();++f){
	    //foutSrcClassFert<<*f<<" ";
	    lnks+=*f;
	  }
	}
      } //if (lnks>backoff)
	    
      //foutSrcClassFert<<"total:"<<lnks<<endl;
      foutSrcClassFert<<clVoc1.form(c->first.first)<<" "<<voc1.form(c->first.second);
      //foutSrcClassFert<<c->first.first<<" "<<c->first.second<<":\n";
      for (int ic=0; (unsigned int) ic<maxfert+1;++ic){
	float cost=0;
	for (int ifert=0; (unsigned int) ifert<maxfert+1; ++ifert){
	  cost+=vfert.at(ifert)*1.0*abs(ic-ifert)/lnks;
	  //foutSrcClassFert<<ifert<<" prob:"<<(vfert.at(ifert)/lnks)<<" "<<"ic:"<<ic<<" +cost:"<<(vfert.at(ifert)*1.0*abs(ic-ifert)/lnks)<<" cost:"<<cost<<endl;
	}
	foutSrcClassFert<<" "<<cost;
	//foutSrcClassFert<<"COST: "<<cost<<endl;
      }
      foutSrcClassFert<<endl;
    }
    // TRG CLASS FERTILITY TABLE
    for (hash_map<pair<clType,formType>,vector<unsigned long int>, hashfpairclform, hasheqpairclform>::const_iterator c=clCount2.begin();c!=clCount2.end();++c){
      // TODO
      double lnks=0;
      vector<unsigned long int> vfert=c->second;
      for (vector<unsigned long int>::const_iterator f=vfert.begin();f!=vfert.end();++f){
	lnks+=*f;
      }
      if (lnks < backoffth*1.0){
	hash_map<formType,wcount>::const_iterator cbo=count2.find(c->first.second);
	if (cbo==count2.end()){
	  cerr<<"ERROR: backoff probabilities not found\n";exit(0);
	}else{
	  vfert=cbo->second.vfert;
	  lnks=0;
	  for (vector<unsigned long int>::const_iterator f=vfert.begin();f!=vfert.end();++f){
	    //foutSrcClassFert<<*f<<" ";
	    lnks+=*f;
	  }
	}
      } //if (lnks>backoff)
      foutTrgClassFert<<clVoc2.form(c->first.first)<<" "<<voc2.form(c->first.second);
      for (int ic=0; (unsigned int) ic<maxfert+1;++ic){
	float cost=0;
	for (int ifert=0; (unsigned int) ifert<maxfert+1; ++ifert){
	  cost+=vfert.at(ifert)*1.0*abs(ic-ifert)/lnks;
	}
	foutTrgClassFert<<" "<<cost;
      }
      foutTrgClassFert<<endl;
    }
  }
    
  if (foutSrcFert.is_open()){
    foutSrcFert.close();
  }
  if (foutTrgFert.is_open()){
    foutTrgFert.close();
  }
  if (foutSrcClassFert.is_open()){
    foutSrcClassFert.close();
  }
  if (foutTrgClassFert.is_open()){
    foutTrgClassFert.close();
  }

  (void) times (time_buffer); double tend = time_buffer->tms_utime / clocks_per_sec;
  if (verbose>1){
    cerr<<"\nHash size:"<<count1_2.size()<<endl;
  }
  cerr<<"Total Processing time:"<<tend-tstart<<endl;
  //cerr<<"Table Calculation and Output:"<<tend-tableCalcBeg<<endl;  
  return 0;
}

