#pragma once
#include <vector>
#include <list>
#include <string>
#include "board.h"

const int LOAD_KIFU_NUM = 20;
const int REPEAT_NUM = 10;
const int THRESH_NUM_CUTOFF_MOVE = 4;
const int MAX_FILENAME = 32;


//学習する重みは黒のためのもの。白の場合は符号を逆転して使う。
/** This class provides the learning algorithm.
 */

struct CorrectMove
{
  CorrectMove(Board& board, BitBoard pos){
    this->board = board;
    correctPos = pos;
  }
  
  Board board;
  BitBoard correctPos;
};

class Learner
{
 public:
  void loadKifu();

 private:
  std::vector<CorrectMove> kyokumen;
};

