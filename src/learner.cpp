#include "learner.h"
#include "random.h"
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>


//棋譜ファイルを読んで局面を変数kyokumenに突っ込んでいき、最後にrandomize
void Learner::loadKifu()
{
  std::ifstream ifs;
  std::string line;
  char filename[MAX_FILENAME];

  for(int i = 0; i < LOAD_KIFU_NUM; i++){
    sprintf(filename, "kifu/self/kifu%d", i);
    ifs.open(filename);
    if(!ifs){
      std::cerr << "Too few training data!" << std::endl;
      break;
    }
    
    Board board;
    int count = 0;
    int tx, ty;
    getline(ifs, line);
    std::cout << "filename: " << filename << std::endl;
    while(getline(ifs, line)){
      if(line == "\n"){
        break;
      }
      std::stringstream buf(line);
      // ファイルに記載されている手はコンピュータ視点なので変換不要
      buf >> tx >> ty;
      BitBoard pos = Board::xyToPos(tx, ty);
      // 最初の数手はいらない
      if(count < THRESH_NUM_CUTOFF_MOVE){
        // boardを一手ずつ進めていく
        // board.putStone(pos, true);
        auto rp = board.putStone(pos);
        if(rp == 0){
          auto xy = Board::posToXY(pos);
          std::cerr << "Failed to put stone according to jouseki!" << std::endl;
          std::cerr << "(x, y) = (" << xy.first
                    << ", " << xy.second << ")" << std::endl;
          std::cerr << "(tx, ty) = (" << tx 
                    << ", " << ty << ")" << std::endl;
          std::cerr << "count: " << count << std::endl;
          exit(1);
        }
        count++;
        continue;
      }
      // tboard.display();
      // 正解の指し手を保存していく
      kyokumen.push_back(CorrectMove(board, pos));

      // boardを一手ずつ進めていく
      // パスなら手順を入れ替えてやり直し
      // if(!board.putStone(pos, true)){
      if(!board.putStone(pos)){
        auto xy = Board::posToXY(pos);
        std::cerr << "Failed to put stone!" << std::endl;
        std::cerr << "(x, y) = (" << xy.first
                  << ", " << xy.second << ")" << std::endl;
        exit(1);
      }
    }
    // 最後の局面は手が決まってしまっており、学習に利用できないので破棄
    kyokumen.pop_back();
    ifs.close();
  }
  
  // randomize
  Random rnd;
  random_shuffle(kyokumen.begin(), kyokumen.end(), rnd);

  std::cout << "Training data randomized!" << std::endl;
  std::cout << "Loaded kyokumen num: " << kyokumen.size() << std::endl;
}


