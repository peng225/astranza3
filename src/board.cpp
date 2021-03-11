#include "board.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>


/************************************/
/* Public methods                   */
/************************************/

void Board::init()
{
  black = INIT_BLACK;
  white = INIT_WHITE;
  turn = State::BLACK;
  tesuu = 1;

  candList.clear();
  std::stack<std::pair<int, int>>().swap(candListDiffs);

  // Add first cand pos (2, 2)
  BitBoard pos = xyToPos(2, 2);
  candList.push_back(pos);

  Direction dirsToInitCandList[11] = {
                Direction::RIGHT, Direction::RIGHT, Direction::RIGHT,
                Direction::DOWN, Direction::DOWN, Direction::DOWN,
                Direction::LEFT, Direction::LEFT, Direction::LEFT,
                Direction::UP, Direction::UP};
  
  for(auto dir : dirsToInitCandList){
    pos = transfer(pos, dir);
    candList.push_back(pos);
  }
}


State Board::getState(BitBoard pos) const
{
  assert(isValidPos(pos));
  BitBoard isBlack, isWhite;
  isBlack = black & pos;
  isWhite = white & pos;
  assert(isBlack == 0 || isWhite == 0);
  
  if(isBlack > 0){
    return State::BLACK;
  }else if(isWhite > 0){
    return State::WHITE;
  }else{
    return State::SPACE;
  }
}


void Board::display() const
{
  std::cout << "   ";
  for(int i = 0; i < BOARD_SIZE; i++){
    std::cout << i + 1 << " ";
  }
  std::cout << std::endl;
  
  for(int i = 0; i < BOARD_SIZE; i++){
    std::cout << i + 1 << " |";
    for(int j = 0; j < BOARD_SIZE; j++){
      BitBoard pos = xyToPos(j, i);
      if(getState(pos) == State::BLACK){
	std::cout << "b";
      }else if(getState(pos) == State::WHITE){
	std::cout << "w";
      }else{
	std::cout << " ";
      }
      std::cout << "|";
    }
    std::cout << std::endl;
  }
  std::cout << "black: " << __builtin_popcountl(black) << ", "
       << "white: " << __builtin_popcountl(white) << std::endl;
  std::cout << std::endl;

  if(getTurn() == State::BLACK){
    std::cout << "Turn:BLACK" << std::endl;
  }else{
    std::cout << "Turn:WHITE" << std::endl;
  }
}


bool Board::canPut (BitBoard pos, bool opponent) const
{
  if(!isValidPos(pos)){
    return false;
  }   
  
  // 空白の位置にのみ石を置ける
  if(((black | white) & pos) != 0){
    return false;
  }

  BitBoard ME;
  BitBoard OPPONENT;

  if(!opponent){
    ME = (turn == State::BLACK ? black : white);
    OPPONENT = (turn == State::BLACK ? white : black);
  }else{
    OPPONENT = (turn == State::BLACK ? black : white);
    ME = (turn == State::BLACK ? white : black);
  }

  
  for(int i = 0; i < NUM_DIRECTION; i++){    
    BitBoard tmpRevPattern = 0;
    BitBoard mask = transfer(pos, DIRS[i]);
    int oppStoneCount = 0;
    // 相手の石が存在する間ループ
    while(mask != 0 && (mask & OPPONENT) != 0) {
        tmpRevPattern |= mask;
        mask = transfer(mask, DIRS[i]);
	    oppStoneCount++;
    }
    /*
      上のループでたどった先に自分の石があり、
      かつひっくり返す石が１つでもあればO.K.
    */
    if((mask & ME) != 0 && oppStoneCount != 0){
      return true;
    }
  }
  
  return false;
}


State Board::getWinner() const
{
  int blackCount = __builtin_popcountl(black);
  int whiteCount = __builtin_popcountl(white);

  if(blackCount > whiteCount) return State::BLACK;
  else if(blackCount == whiteCount) return State::SPACE;
  else return State::WHITE;
}


void Board::getMoveList(std::vector<BitBoard>& moveList) const
{
    moveList.reserve(candList.size());
    for(const auto& cand : candList){
        if(canPut(cand)){
            moveList.push_back(cand);
        }
    }
    assert(!moveList.empty());
}

void Board::toVector(std::vector<float>& vec) const
{
    assert(vec.size() == 2 * NUM_CELL);
    for(int i = 0; i < BOARD_SIZE; i++){
        BitBoard pos = MSB_ONLY_64;
        pos = pos >> (BOARD_SIZE * i);
        for(int j = 0; j < BOARD_SIZE; j++){
            if(getState(pos) == State::BLACK){
              vec[i * BOARD_SIZE + j] = 1;
            } else if(getState(pos) == State::WHITE){
              vec[NUM_CELL + i * BOARD_SIZE + j] = 1;
            }
            pos = Board::transfer(pos, Direction::RIGHT);
        }
    }
}


bool Board::operator==(const Board &obj) const
{
  if(black == obj.black && white == obj.white
     && turn == obj.getTurn()){
    assert(tesuu == obj.tesuu);
    return true;
  }else{
    return false;
  }
}

std::pair<int, int> Board::posToXY(BitBoard pos)
{
  assert(isValidPos(pos));
  std::pair<int, int> coord;
  int clz = __builtin_clzl(pos);
  coord.first = clz % BOARD_SIZE;
  coord.second = clz / BOARD_SIZE;
  return coord;
}

BitBoard Board::xyToPos(int x, int y)
{
  return (MSB_ONLY_64 >> (x + BOARD_SIZE * y));
}

// TODO: what is this algorithm?
bool Board::isValidPos(BitBoard pos)
{
  return (pos != 0) && ((pos & (pos - 1)) == 0);
}

BitBoard Board::transfer(BitBoard oneBit, Direction d)
{
  switch(d){
  case Direction::LEFT_UP:
    return (oneBit << (BOARD_SIZE + 1)) & RIGHT_EDGE;
  case Direction::UP:
    return (oneBit << BOARD_SIZE);
  case Direction::RIGHT_UP:
    return (oneBit << (BOARD_SIZE - 1)) & LEFT_EDGE;
  case Direction::RIGHT:
    return (oneBit >> 1) & LEFT_EDGE;
  case Direction::RIGHT_DOWN:
    return (oneBit >> (BOARD_SIZE + 1)) & LEFT_EDGE;
  case Direction::DOWN:
    return (oneBit >> BOARD_SIZE);
  case Direction::LEFT_DOWN:
    return (oneBit >> (BOARD_SIZE - 1)) & RIGHT_EDGE;
  case Direction::LEFT:
    return (oneBit << 1) & RIGHT_EDGE;
  default:
    return 0xFFFFFFFFFFFFFFFF;
  }
}



/************************************/
/* Private methods                  */
/************************************/

// 要ユニットテスト
// BitBoard Board::getDoughnut(BitBoard pos) const
// {
//   assert(isValidPos(pos));
//   // XY座標に変換しなくても、先頭にならぶ0のビット数を数えたりすれば
//   // シフト演算だけでいけるのでは？
//   // さらに、ループ回してshift演算しなくても、
//   // 一気にshiftしてしまうこともできるのでは？
//   // その場合反対側に周りこんだビットの処理が面倒だけど、できそう。
//   pair<int, int> coord = posToXY(pos);
//   coord.first--;
//   coord.second--;  

//   BitBoard doughnut = DOUGHNUT;
//   if(coord.first < 0){
//     doughnut = transfer(doughnut, LEFT);
//   }else{
//     for(int i = 0; i < coord.first; i++){
//       doughnut = transfer(doughnut, RIGHT);
//     }
//   }
  
//   if(coord.second < 0){
//     doughnut = transfer(doughnut, UP);
//   }else{
//     for(int i = 0; i < coord.first; i++){
//       doughnut = transfer(doughnut, DOWN);
//     }
//   }
// }

// void Board::displayBitBoard(BitBoard bb)
// {
//   BitBoard one = 1;
//   for(int i = 0; i < BOARD_SIZE; i++){
//     for(int j = 0; j < BOARD_SIZE; j++){
//       if(((one << ((BOARD_SIZE - i - 1) * BOARD_SIZE + (BOARD_SIZE - j - 1))) & bb) != 0){
// 	std::cout << "1";
//       }else{
// 	std::cout << "0";
//       }
//     }
//     std::cout << std::endl;
//   }
// }


BitBoard Board::putStone(BitBoard pos, bool clearClds)
{ 
  if(!isValidPos(pos)){
    return 0;
  }
  
  BitBoard revPattern = 0;
  
  // 空白の位置にのみ石を置ける
  if(((black | white) & pos) != 0){
    return 0;
  }

  const BitBoard ME = (turn == State::BLACK ? black : white);
  const BitBoard OPPONENT = (turn == State::BLACK ? white : black);

  
  for(auto dir : DIRS){
    BitBoard tmpRevPattern = 0;
    BitBoard mask = transfer(pos, dir);
    // 相手の石が存在する間ループ
    while(mask != 0 && (mask & OPPONENT) != 0) {
        tmpRevPattern |= mask;
        mask = transfer(mask, dir);
    }
    // 上のループでたどった先に自分の石がなければひっくり返せない
    if((mask & ME) != 0){
      revPattern |= tmpRevPattern;
    }
  }

  // 石の反転を適用
  if(revPattern == 0){
    return 0;
  }else if(ME == black){
    black ^= pos | revPattern;
    white ^= revPattern;
  }else{
    white ^= pos | revPattern;
    black ^= revPattern;
  }

  forwardUpdateCandList(pos);
  changeTurn();  
  if(isPass()){
    // std::cout << (getTurn() == State::BLACK ? "BLACK" : "WHITE") << " PASS" << std::endl;
    changeTurn();  
  }
  tesuu++;

  if(clearClds){
    candListDiffs.pop();
    assert(candListDiffs.empty());
  }

  return revPattern;
}

void Board::undo(BitBoard pos, BitBoard revPattern)
{
  assert(0 < tesuu);

  const State WHO_PUT = (getState(pos) == State::BLACK 
                            ? State::BLACK : State::WHITE);

  if(WHO_PUT == State::BLACK){
    black ^= pos | revPattern;
    white ^= revPattern;
  }else{
    white ^= pos | revPattern;
    black ^= revPattern;
  }
  
  backUpdateCandList(pos);
  // パスが発生していた場合は手番の入れ替えは不要
  if(turn != WHO_PUT){
    changeTurn();
  }
  tesuu--;
}

bool Board::isEnd() const
{
  //for(list<BitBoard>::const_iterator itr = begin(candList);
  //    itr != end(candList); itr++){
  for(const auto& candidate : candList){
    if(canPut(candidate) || canPut(candidate, true)){
      return false;
    }
  }

  return true;
}

bool Board::isPass() const
{ 
  //for(list<BitBoard>::const_iterator itr = begin(candList);
  //    itr != end(candList); itr++){
  for(const auto& candidate : candList){
    if(canPut(candidate)){
      return false;
    }
  }

  return true;
}


void Board::forwardUpdateCandList(BitBoard pos)
{
  assert(isValidPos(pos));
  std::list<BitBoard>::iterator posItr = find(begin(candList), end(candList), pos);
  assert(posItr != end(candList));
  
  posItr = candList.erase(posItr);

  std::pair<int, int> clDiff;
  // candListから消された場所が元々candListの何番目にいたかという情報を取得
  clDiff.first = distance(begin(candList), posItr);

  int addCount = 0;
  
  for(int i = 0; i < NUM_DIRECTION; i++){
    BitBoard aroundPos = transfer(pos, DIRS[i]);
    // 盤面からはみ出していたらダメ
    if(!isValidPos(aroundPos)){
      continue;
    }
    // スペースかつリスト未登録であれば新規登録
    if(getState(aroundPos) == State::SPACE &&
       find(begin(candList), end(candList), aroundPos) == end(candList)){
      posItr = candList.insert(posItr, aroundPos);
      posItr++;
      addCount++;
      // candList.push_back(aroundPos);
    }
  }
  // 新たにいくつの要素をcandListに追加したかという情報
  clDiff.second = addCount;
  candListDiffs.push(clDiff);
}

void Board::backUpdateCandList(BitBoard pos)
{
  assert(isValidPos(pos));  

  assert(!candListDiffs.empty());
  std::pair<int, int> clDiff = candListDiffs.top();
  candListDiffs.pop();
  std::list<BitBoard>::iterator posItr = begin(candList);

  for(int i = 0; i < clDiff.first; i++){
    posItr++;
  }

  for(int i = 0; i < clDiff.second; i++){
    posItr = candList.erase(posItr);
  }

  candList.insert(posItr, pos);  
}

//void Board::displayCandListPos()
//{
//  BitBoard bb = (BitBoard)0;
//
//  for(list<BitBoard>::iterator itr = begin(candList);
//      itr != end(candList); itr++){
//    bb |= *itr;
//  }
//  displayBitBoard(bb);
//}
