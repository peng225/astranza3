#include "jouseki.h"
#include "random.h"
#include <fstream>
#include <sstream>

//public member of class Jouseki
void Jouseki::readJousekiFile(std::string filename)
{
  std::ifstream ifs(filename.c_str());
  std::string line;
  std::list<BitBoard> moveList[NUM_SYMMETRY]; //対称性を処理するため
  while(getline(ifs, line)){
    if(line[0] == '#'){
      continue;
    }
    if(line.empty()){
      for(int i = 0; i < NUM_SYMMETRY; i++){
	if(!moveList[i].empty()){
	  jousekiList.push_back(moveList[i]);
	}
      }
      for(int i = 0; i < NUM_SYMMETRY; i++){
        moveList[i].clear();
      }
      continue;
    }
    
    int tx, ty;    
    std::stringstream buf(line);
    buf >> tx >> ty;
    assert(tx != 0);
    if(tx == 0) break;

    // 人間視点の座標をコンピュータ視点に変換
    tx--;
    ty--;
    
    // std::pair<int, int> coord(tx, ty);
    BitBoard pos = Board::xyToPos(tx, ty);
    int ox, oy;
    //std::cout << p.first << "," << p.second << std::endl;
    moveList[0].push_back(pos);
    ox = tx;
    oy = ty;
    tx = oy;
    ty = ox;
    pos = Board::xyToPos(tx, ty);
    // tx = coord.first;
    // ty = coord.second;
    // coord.first = ty;
    // coord.second = tx;
    //std::cout << p.first << "," << p.second << std::endl;
    moveList[1].push_back(pos);
    ox = tx;
    oy = ty;
    tx = BOARD_SIZE - 1 - ox;
    ty = BOARD_SIZE - 1 - oy;
    pos = Board::xyToPos(tx, ty);
    // tx = coord.first;
    // ty = coord.second;
    // coord.first = BOARD_SIZE - 1 - tx;
    // coord.second = BOARD_SIZE - 1 - ty;
    //std::cout << p.first << "," << p.second << std::endl;
    moveList[2].push_back(pos);
    ox = tx;
    oy = ty;
    tx = oy;
    ty = ox;
    pos = Board::xyToPos(tx, ty);
    // tx = coord.first;
    // ty = coord.second;
    // coord.first = ty;
    // coord.second = tx;
    //std::cout << p.first << "," << p.second << std::endl;
    moveList[3].push_back(pos);
  }
  ifs.close();

  // for(vector<list<pair<int, int> > >::iterator i = begin(jousekiList);
  //     i != end(jousekiList); i++){
  //   for(list<pair<int, int> >::iterator j = begin(*i);
  // 	j != end(*i); j++){
  //     cout << j->first+1 << ", " << j->second+1 << endl;
  //   }
  //   cout << endl;
  // }

  //randomize
  randJousekiList();
}

BitBoard Jouseki::useJouseki(Board &board)
{
  for(std::vector<std::list<BitBoard> >::iterator i = std::begin(jousekiList);
      i != std::end(jousekiList); i++){
    if((int)i->size() < board.getTesuu()) continue;
    int count = 0;
    Board tmpBoard;
    // 現在の手数の手前まで移動
    for(std::list<BitBoard>::iterator j = std::begin(*i);
	count < board.getTesuu() - 1; j++){
      tmpBoard.putStone(*j, true);
      count++;
    }
    // 定石ファイルに同じ盤面が存在すれば使う
    if(board == tmpBoard){
      std::cout << "jouseki x, y = ";
      std::list<BitBoard>::iterator itr = std::begin(*i);
      for(int j = 0; j < count; j++){
	itr++;
      }
      board.putStone(*itr, true);
      std::pair<int, int> coord = Board::posToXY(*itr);
      std::cout << coord.first + 1 << ", " << coord.second + 1 << std::endl;
      return *itr;
    }
  }
  return 0;
}

void Jouseki::randJousekiList()
{
  Random rnd;
  random_shuffle(jousekiList.begin(), jousekiList.end(), rnd);
}
