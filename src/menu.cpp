#include "menu.h"
#include <assert.h>

void printWinner(const Board &board)
{
  State winner = board.getWinner();
  switch(winner){
  case State::BLACK:
    std::cout << "BLACK won." << std::endl;
    break;
  case State::WHITE:
    std::cout << "WHITE won." << std::endl;
    break;
  case State::SPACE:
    std::cout << "Draw." << std::endl;
    break;
  default:
    std::cout << "Bad winner data received." << std::endl;
  }
}

void put(Board &board, std::list<History> &hist, const std::list<std::string> &args){
  if(args.size() < 2){
    std::cerr << "x and y value is required." << std::endl;
    return;
  }
  
  int x = 0, y = 0;
  bool putSuccess = true;

  std::list<std::string>::const_iterator itr = std::begin(args);
  x = atoi(itr->c_str());
  itr++;
  y = atoi(itr->c_str());

  // 人間視点の座標をコンピュータ視点へ変換
  x--;
  y--;
  BitBoard pos = Board::xyToPos(x, y);
  assert(Board::isValidPos(pos));


  // BitBoard revPattern;  
  // if((revPattern = board.putStone(x, y)) == 0){
  if(!board.putStone(pos, true)){
    std::cout << "Illegal move!" << std::endl;
    putSuccess = false;
  }
  if(putSuccess){
    // history
    hist.push_back(History(board, pos));

    board.display();

    // 終了処理
    if(board.isEnd()){
      printWinner(board);
      return;
    }

    // パスの処理
    if(board.isPass()){
      std::cout << (board.getTurn() == State::BLACK ? "BLACK" : "WHITE") << " PASS" << std::endl;
      board.changeTurn();
    }    
  }

  
  // board.undo(x, y, revPattern);
  // board.display();  
}

void undo(Board &board, std::list<History> &hist)
{
  if(hist.size() != 0){
    hist.back().recover(board);
    hist.pop_back();
    board.display();
  }else{
    std::cout << "No history exists." << std::endl;
  }
}

