#pragma once

#include "board.h"
#include <iostream>
#include <vector>
#include <list>
#include <string>

const std::string DEFAULT_JOUSEKI_FILENAME = "jouseki";
const int NUM_SYMMETRY = 4;

/** This class provides the functions to use the opening database.
 */
class Jouseki
{ 
 public:
  void readJousekiFile(std::string filename = DEFAULT_JOUSEKI_FILENAME);
  BitBoard useJouseki(Board &board, bool verbose);
 private:
  /** This function randomize the list of opening database.
   */  
  std::vector<std::list<BitBoard> > jousekiList;  

  void randJousekiList();
};

