#pragma once

#include "board.h"

/** This class holds the information of the board history.
 */
class History
{
 public:
  History(const Board &board, BitBoard pos);
  /** This function gives the information which it has to the parameters.
   * In short, copies the value from "this->ban" to "ban", 
   * from "this->turn" to "turn", and from "this->tesuu" to "tesuu".
   */
  void recover(Board &board) const;
  BitBoard getPos()
  {
    return pos;
  }
 private:
  Board board;
  BitBoard pos;
};

