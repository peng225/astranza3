#include "history.h"

History::History(const Board &board, BitBoard pos)
{
  this->board = board;
  this->pos = pos;
}

void History::recover(Board& board) const
{
  board = this->board;
}
