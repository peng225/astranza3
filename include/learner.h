#pragma once
#include "board.h"
#include <cnn.h>
#include <vector>
#include <list>
#include <string>

const int REPEAT_NUM = 4;
const int THRESH_NUM_CUTOFF_MOVE = 4;
const int MAX_FILENAME = 32;


//学習する重みは黒のためのもの。白の場合は符号を逆転して使う。
/** This class provides the learning algorithm.
 */

struct CorrectMove
{
    CorrectMove(Board& board, BitBoard pos, State winner){
        this->board = board;
        correctPos = pos;
        this->winner = winner;
    }
  
  Board board;
  BitBoard correctPos;
  State winner;
};

class Learner
{
 public:
    Learner(std::shared_ptr<DeepNetwork> dnet) : dn(dnet)
    {
    }
    void loadKifu(int numLoadKifu);
    void learn();

 private:
    std::vector<CorrectMove> kyokumen;
    std::shared_ptr<DeepNetwork> dn;
};

