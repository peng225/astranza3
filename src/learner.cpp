#include "learner.h"
#include "random.h"
#include "utility.h"
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <omp.h>


//棋譜ファイルを読んで局面を変数kyokumenに突っ込んでいき、最後にrandomize
void Learner::loadKifu(int numLoadKifu)
{
  assert(0 < numLoadKifu);
  std::ifstream ifs;
  std::string line;
  char filename[MAX_FILENAME];

  for(int i = 0; i < numLoadKifu; i++){
    sprintf(filename, "kifu/evolve/kifu%d", i);
    ifs.open(filename);
    if(!ifs){
      std::cerr << "Too few training data!" << std::endl;
      break;
    }
    
    Board board;
    int count = 0;
    int tx, ty;
    std::cout << "filename: " << filename << std::endl;
    getline(ifs, line);
    State winner = static_cast<State>(std::stoi(line));
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
      // 学習データを増やすために反転したデータも生成する
      kyokumen.emplace_back(CorrectMove(board, pos, winner));

      auto orgPos = pos;

      board.flipWithLTtoRDDiagonal();
      pos = Board::flipBBWithLTtoRDDiagonal(pos);
      kyokumen.emplace_back(CorrectMove(board, pos, winner));

      board.flipWithRTtoLDDiagonal();
      pos = Board::flipBBWithRTtoLDDiagonal(pos);
      kyokumen.emplace_back(CorrectMove(board, pos, winner));

      board.flipWithLTtoRDDiagonal();
      pos = Board::flipBBWithLTtoRDDiagonal(pos);
      kyokumen.emplace_back(CorrectMove(board, pos, winner));

      // 指し手を進められるように元に戻す
      board.flipWithRTtoLDDiagonal();
      pos = Board::flipBBWithRTtoLDDiagonal(pos);

      assert(pos == orgPos);

      // boardを一手ずつ進めていく
      if(!board.putStone(pos)){
        auto xy = Board::posToXY(pos);
        std::cerr << "Failed to put stone!" << std::endl;
        std::cerr << "(x, y) = (" << xy.first
                  << ", " << xy.second << ")" << std::endl;
        exit(1);
      }
    }
    assert(board.isEnd());
    assert(board.getWinner() == winner);

    // 最後の局面は手が決まってしまっており、学習に利用できないので破棄
    // -> 勝率の計算精度を上げるのに使えるので破棄しないことにした
    // kyokumen.pop_back();
    // kyokumen.pop_back();
    // kyokumen.pop_back();
    // kyokumen.pop_back();
    ifs.close();
  }
  
  // randomize
  Random rnd;
  random_shuffle(kyokumen.begin(), kyokumen.end(), rnd);

  std::cout << "Training data randomized!" << std::endl;
  std::cout << "Loaded kyokumen num: " << kyokumen.size() << std::endl;
}


void Learner::learn()
{
    if(kyokumen.empty()){
        std::cerr << "No training data found" << std::endl;
        return;
    }

    for(int i = 0; i < REPEAT_NUM; i++){
        std::cout << "learning iteration#: " << i << std::endl;
        #pragma omp parallel for
        for(int i = 0; static_cast<size_t>(i) < kyokumen.size(); i++)
        {
            std::vector<float> input;
            std::vector<float> correctOutput(2 + NUM_CELL);
            kyokumen.at(i).board.toVector(input);
            if(kyokumen.at(i).winner == State::BLACK){
                correctOutput.at(0) = 1;
                correctOutput.at(1) = 0;
            }else if(kyokumen.at(i).winner == State::WHITE){
                correctOutput.at(0) = 0;
                correctOutput.at(1) = 1;
            }else{
                correctOutput.at(0) = 0.5;
                correctOutput.at(1) = 0.5;
            }
            auto xy = Board::posToXY(kyokumen.at(i).correctPos);
            correctOutput.at(2 + xy.first + xy.second * BOARD_SIZE) = 1.0;
            dn->backPropagate(input, correctOutput,
                kyokumen.at(i).board.getTesuu() / static_cast<double>(NUM_CELL));
        }
        dn->flush();
        // randomize
        Random rnd;
        random_shuffle(kyokumen.begin(), kyokumen.end(), rnd);
    }

    std::cout << "check" << std::endl;
    int count = 0;
    std::vector<float> input;
    for(const auto& km : kyokumen)
    {
        std::vector<float> correctOutput(2 + NUM_CELL);
        km.board.toVector(input);
        //km.board.display();
        //printVector(input);
        if(km.winner == State::BLACK){
            correctOutput.at(0) = 1;
            correctOutput.at(1) = 0;
        }else if(km.winner == State::WHITE){
            correctOutput.at(0) = 0;
            correctOutput.at(1) = 1;
        }else{
            correctOutput.at(0) = 0.5;
            correctOutput.at(1) = 0.5;
        }
        auto xy = Board::posToXY(km.correctPos);
        correctOutput.at(2 + xy.first + xy.second * BOARD_SIZE) = 1.0;

        auto out = dn->feedInput(input);
        assert(out.back().size() == 2 + NUM_CELL);
        std::cout << "i, out, cout: " << std::endl;
        for(int i = 0; static_cast<size_t>(i) < out.back().size(); i++) {
            std::cout << "         : " << i << ", " << out.back().at(i) << ", " << correctOutput.at(i) << std::endl;
        }
        std::cout << std::endl;

        count++;
        if(count == 10) break;
    }
    std::cout << "all black input:" << std::endl;
    input.clear();
    input.resize(2 * NUM_CELL);
    for(int i = 0; static_cast<size_t>(i) < NUM_CELL; i++) {
        input.at(i) = 1;
    }
    auto out = dn->feedInput(input);
    assert(out.back().size() == 2 + NUM_CELL);
    std::cout << "out: " << out.back().at(0) << std::endl;
    std::cout << "   : " << out.back().at(1) << std::endl;

    std::cout << "all white input:" << std::endl;
    input.clear();
    input.resize(2 * NUM_CELL);
    for(int i = 0; static_cast<size_t>(i) < NUM_CELL; i++) {
        input.at(i + NUM_CELL) = 1;
    }
    out = dn->feedInput(input);
    assert(out.back().size() == 2 + NUM_CELL);
    std::cout << "out: " << out.back().at(0) << std::endl;
    std::cout << "   : " << out.back().at(1) << std::endl;
    std::cout << std::endl;
}

