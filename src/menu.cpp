#include "menu.h"
#include "player.h"
#include "learner.h"
#include <fstream>
#include <cassert>
#include <unistd.h>
#include <cmath>
#include <omp.h>

namespace menu
{
constexpr int NUM_DN_OBJ = 2;
std::shared_ptr<DeepNetwork> dn[NUM_DN_OBJ];

void init()
{
    for(int i = 0; i < NUM_DN_OBJ; i++){
        menu::dn[i] = std::make_shared<DeepNetwork>();
        // Construct network
        dn[i]->setInputInfo(DataSize(BOARD_SIZE, BOARD_SIZE), 2);

        auto l1 = std::make_shared<ConvolutionLayer>(1, 4, 8);
        dn[i]->addLayer(l1);

        auto l2 = std::make_shared<ReLULayer>();
        dn[i]->addLayer(l2);

        auto l3 = std::make_shared<PoolingLayer>(1, 4);
        dn[i]->addLayer(l3);

        auto l4 = std::make_shared<ConvolutionLayer>(1, 3, 16);
        dn[i]->addLayer(l4);

        auto l5 = std::make_shared<ReLULayer>();
        dn[i]->addLayer(l5);

        auto l6 = std::make_shared<PoolingLayer>(1, 3);
        dn[i]->addLayer(l6);

        auto l7 = std::make_shared<FullConnectLayer>(DataSize(40, 1));
        dn[i]->addLayer(l7);

        auto l8 = std::make_shared<SigmoidLayer>();
        dn[i]->addLayer(l8);

        auto l9 = std::make_shared<FullConnectLayer>(DataSize(2, 1));
        dn[i]->addLayer(l9);

        auto l10 = std::make_shared<SoftmaxLayer>();
        dn[i]->addLayer(l10);
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
    hist.emplace_back(History(board, pos));

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

void search(Board &board, std::list<History> &hist, int rolloutDepth)
{
  Player pl(dn[0], rolloutDepth);
  auto pos = pl.search(board);
  // history
  hist.emplace_back(History(board, pos));

  board.display();

  // 終了処理
  if(board.isEnd()){
    printWinner(board);
  }
}


float selfPlay(int numSelfPlay,
               int rolloutDepth,
               int numRollout,
               bool shouldSaveKifu,
               int dnId1,
               int dnId2,
               bool verbose)
{
  int numPl1Win = 0;
  int numDraw = 0;
  #pragma omp parallel for
  for(int i = 0; i < numSelfPlay; i++){
    #pragma omp critical(print)
    {
      std::cout << "Self play #" << i << " start." << std::endl;
    }
    Player pl1(dn[dnId1], rolloutDepth);
    Player pl2(dn[dnId2], rolloutDepth);
    Board board;
    board.init();
    std::list<History> hist;
    BitBoard pos;
    while(!board.isEnd()){
      if(board.getTurn() == State::BLACK){
        pos = pl1.search(board, numRollout, verbose);
      }else{
        pos = pl2.search(board, numRollout, verbose);
      }
      // history
      hist.emplace_back(History(board, pos));
      if(verbose){
        #pragma omp critical(print)
        {
          board.display();
        }
      }
    }

    // 終了処理
    #pragma omp critical(print)
    {
      printWinner(board);
    }
    auto winner = board.getWinner();
    #pragma omp critical(numPl1WinAndNumDrawUpdate)
    {
      if(winner == State::BLACK){
        numPl1Win++;
      }else if(winner == State::SPACE){
        numDraw++;
      }
    }
    if(shouldSaveKifu){
      #pragma omp critical(saveFile)
      {
        std::ofstream ofs("kifu/evolve/kifu" + std::to_string(i));
        ofs << static_cast<int>(board.getWinner()) << std::endl;
        for(auto h : hist){
          auto xy = Board::posToXY(h.getPos());
          ofs << xy.first << " " << xy.second << std::endl;
        }
        ofs.close();
      }
      #pragma omp critical(print)
      {
        std::cout << "Saved a kifu to " << "\"kifu/evolve/kifu" + std::to_string(i) << "\"." << std::endl;
      }
    }
    board.init();
    hist.clear();
  }

  // Print the result
  float p1WinRate;
  if(numSelfPlay == numDraw){
    p1WinRate = 0.5;
    std::cout << "All games were draw" << std::endl;
  }else{
    p1WinRate = static_cast<double>(numPl1Win) / (numSelfPlay - numDraw);
    std::cout << "Player 1's win rate: "
              << 100 * p1WinRate << "%" << std::endl;
  }

  return p1WinRate;
}

void selfPlay(const std::list<std::string> &args)
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

  selfPlay(numSelfPlay, rolloutDepth, NUM_DEFAULT_ROLLOUT,
           shouldSaveKifu, dnId1, dnId2, true);
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


void saveWeight(const std::list<std::string> &args)
{
  if(args.size() < 2){
    std::cerr << "usage: save [dnID 0 or 1] [filename]" << std::endl;
    return;
  }

  std::list<std::string>::const_iterator itr = std::begin(args);
  auto dnId = atoi(itr->c_str());
  itr++;
  auto filename = itr->c_str();

  dn[dnId]->saveWeight(filename);
}

void loadWeight(const std::list<std::string> &args)
{
  if(args.size() < 2){
    std::cerr << "usage: load [dnID 0 or 1] [filename]" << std::endl;
    return;
  }

  std::list<std::string>::const_iterator itr = std::begin(args);
  auto dnId = atoi(itr->c_str());
  itr++;
  auto filename = itr->c_str();

  dn[dnId]->loadWeight(filename);
}

void evolve(const std::list<std::string> &args)
{
  if(args.size() < 7){
    std::cerr << "usage: evolve [rollout depth] [num rollout] [num max itr] [num one round] [num generation] [dnID 0 or 1] [filename]" << std::endl;
    return;
  }

  std::list<std::string>::const_iterator itr = std::begin(args);
  auto rolloutDepth = atoi(itr->c_str());
  itr++;
  auto numRollout = atoi(itr->c_str());
  itr++;
  auto numMaxItr = atoi(itr->c_str());
  itr++;
  auto numOneRound = atoi(itr->c_str());
  itr++;
  auto numGeneration = atoi(itr->c_str());
  itr++;
  auto dnId = atoi(itr->c_str());
  itr++;
  auto filename = *itr;

  std::cout << "rolloutDepth: " << rolloutDepth << std::endl
            << "numRollout: " << numRollout << std::endl
            << "numMaxItr: " << numMaxItr << std::endl
            << "numOneRound: " << numOneRound << std::endl
            << "numGeneration: " << numGeneration << std::endl
            << "dnId: " << dnId << std::endl
            << "filename: " << filename << std::endl;

  int othDnId = (dnId + 1) % 2;

  int numTestPlay = std::max(20, numOneRound/10);
  numTestPlay = (numTestPlay + 1) / 2 * 2;
  std::cout << "numTestPlay: " << numTestPlay << std::endl;

  // np + Z_{0.025} * √(np(1-p))
  double requiredWinRate = std::min(1.0, (numTestPlay*0.5 + 1.96*(sqrt(numTestPlay*0.25))) / numTestPlay);
  std::cout << "requiredWinRate: " << requiredWinRate << std::endl;
  std::cout << std::endl;

  float pl1WinRate;
  int numIteration;

  for(int i = 0; i < numGeneration; i++){
    std::cout << "Generation#: " << i << std::endl;
    numIteration = 0;
    pl1WinRate = 0;
    while(pl1WinRate < requiredWinRate && numIteration < numMaxItr) {
      std::cout << "Iteration#: " << numIteration << std::endl;

      // Produce phase
      std::cout << "Produce phase start!" << std::endl;
      sleep(1);
      selfPlay(numOneRound, rolloutDepth, numRollout,
               true, dnId, dnId, false);
      std::cout << std::endl;

      // Learning phase
      std::cout << "Learning phase start!" << std::endl;
      sleep(1);
      Learner ln(dn[dnId]);
      ln.loadKifu(numOneRound);
      ln.learn();
      std::cout << std::endl;

      // Test phase
      std::cout << "Test phase start!" << std::endl;
      std::cout << "BLACK phase start!" << std::endl;
      pl1WinRate = selfPlay(numTestPlay/2,
                            DEFAULT_ROLLOUT_DEPTH, NUM_DEFAULT_ROLLOUT,
                            false, dnId, othDnId, false);
      std::cout << "WHITE phase start!" << std::endl;
      pl1WinRate += 1.0 - selfPlay(numTestPlay/2,
                            DEFAULT_ROLLOUT_DEPTH, NUM_DEFAULT_ROLLOUT,
                            false, othDnId, dnId, false);
      pl1WinRate /= 2;
      std::cout << "Total win rate: " << 100 * pl1WinRate << "%" << std::endl;

      // save
      dn[dnId]->saveWeight(filename + "_" + std::to_string(i));
      std::cout << "saved the weight." << std::endl;
      std::cout << std::endl;

      numIteration++;
    }
    if(requiredWinRate <= pl1WinRate ) {
      std::cout << "Evolved enough at the generation#" << i << ". " << std::endl;
    } else {
      std::cout << "WARNING: Could not evolve enough at the generation#" << i << ". " << std::endl;
    }
    dn[othDnId]->loadWeight(filename);
  }
}

} // namespace menu end

