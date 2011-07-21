#define PHMODUL 1000000 //typical number of phrases
#define WMODUL 500000 //typical number of words in the vocabulary
#define PAIRALUNITMODUL 2000000 //typical number of alignment unit pairs

#ifndef _DEFS_H
#define _DEFS_H

// size of vocabulary for words:
typedef unsigned long int wordIndex;

//number of possible links in a sentence pair:
typedef int linkIndex;

// sentence is defined in sentPair.h

//const int MAX_WD_ASSO_SCORES=10;
//typedef unsigned char qscore;
//const qscore MAX_QSCORE=255;
typedef unsigned int qscore;
const qscore MAX_QSCORE=2047;
typedef float cost;

//maximum number of levels allowed for recursive multi-level alignment
const int maxLevels=10;
#endif
