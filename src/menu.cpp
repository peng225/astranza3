#include "menu.h"
#include "player.h"
#include "learner.h"
#include <fstream>
#include <cassert>

namespace menu
{
const int NUM_DN_OBJ = 2;
std::shared_ptr<DeepNetwork> dn[NUM_DN_OBJ];

void init()
{
    for(int i = 0; i < NUM_DN_OBJ; i++){
        menu::dn[i] = std::make_shared<DeepNetwork>();
        // Construct network
        dn[i]->setInputInfo(DataSize(BOARD_SIZE, BOARD_SIZE), 1);

        auto l1 = std::make_shared<ConvolutionLayer>(1, 3, 2);
        dn[i]->addLayer(l1);

        auto l2 = std::make_shared<ReLULayer>();
        dn[i]->addLayer(l2);

        auto l3 = std::make_shared<PoolingLayer>(1, 3);
        dn[i]->addLayer(l3);

        auto l4 = std::make_shared<FullConnectLayer>(DataSize(2, 1));
        dn[i]->addLayer(l4);

        auto l5 = std::make_shared<ActivateLayer>();
        dn[i]->addLayer(l5);
    }
}

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
  // if(!board.putStone(pos, true)){
  if(!board.putStone(pos)){
    std::cout << "Illegal move!" << std::endl;
  }
  else{
    // history
    hist.push_back(History(board, pos));

    board.display();

    // 終了処理
    if(board.isEnd()){
      printWinner(board);
      return;
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

void search(Board &board, std::list<History> &hist)
{
  Player pl(dn[0], DEFAULT_ROLLOUT_DEPTH);
  auto pos = pl.search(board);
  // history
  hist.push_back(History(board, pos));

  board.display();

  // 終了処理
  if(board.isEnd()){
    printWinner(board);
  }
}

void selfPlay(Board &board, std::list<History> &hist, const std::list<std::string> &args)
{
  if(args.size() < 5){
    std::cerr << "usage: self [numSelfPlay] [rollout depth] [0: not save, 1: save] [dnID 0 or 1] [dnID 0 or 1]" << std::endl;
    return;
  }
  
  std::list<std::string>::const_iterator itr = std::begin(args);
  auto numSelfPlay = atoi(itr->c_str());
  itr++;
  auto rolloutDepth = atoi(itr->c_str());
  itr++;
  bool shouldSaveKifu = (atoi(itr->c_str()) == 1 ? true : false);
  itr++;
  auto dnId1 = atoi(itr->c_str());
  itr++;
  auto dnId2 = atoi(itr->c_str());

  Player pl1(dn[dnId1], rolloutDepth);
  Player pl2(dn[dnId2], rolloutDepth);
  int numPl1Win = 0;
  int numDraw = 0;
  for(int i = 0; i < numSelfPlay; i++){
    BitBoard pos;
    while(!board.isEnd()){
      if(board.getTurn() == State::BLACK){
        pos = pl1.search(board);
      }else{
        pos = pl2.search(board);
      }
      // history
      hist.push_back(History(board, pos));
      board.display();
    }

    // 終了処理
    printWinner(board);
    auto winner = board.getWinner();
    if(winner == State::BLACK){
      numPl1Win++;
    }else if(winner == State::SPACE){
      numDraw++;
    }
    if(shouldSaveKifu){
      std::ofstream ofs("kifu/self/kifu" + std::to_string(i));
      ofs << static_cast<int>(board.getWinner()) << std::endl;
      for(auto h : hist){
        auto xy = Board::posToXY(h.getPos());
        ofs << xy.first << " " << xy.second << std::endl;
      }
      ofs.close();
    }
    board.init();
    hist.clear();
  }

  // Print the result
  if(numSelfPlay == numDraw){
    std::cout << "All games were draw" << std::endl;
  }else{
    std::cout << "Player 1's win rate: "
              << 100* static_cast<double>(numPl1Win) / (numSelfPlay - numDraw)
              << "%" << std::endl;
  }
}


void learn(const std::list<std::string> &args)
{
    if(args.size() < 2){
        std::cerr << "usage: learn [dnObjId] [numLoadKifu]" << std::endl;
        return;
    }
    std::list<std::string>::const_iterator itr = std::begin(args);
    auto dnObjId = atoi(itr->c_str());
    itr++;
    auto numLoadKifu = atoi(itr->c_str());
    if(NUM_DN_OBJ <= dnObjId){
        std::cerr << "dnObjId must be less than " << NUM_DN_OBJ << std::endl;
        return;
    }

    Learner ln(dn[dnObjId]);
    ln.loadKifu(numLoadKifu);
    ln.learn();
}

}

