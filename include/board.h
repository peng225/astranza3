#pragma once

#include <stdint.h>
#include <list>
#include <stack>
#include <algorithm>


enum class State{BLACK = -1, WHITE = 1, SPACE = 0};
enum class Direction{LEFT_UP, UP, RIGHT_UP, RIGHT,
		       RIGHT_DOWN, DOWN,
		       LEFT_DOWN, LEFT};
typedef uint64_t BitBoard;


const BitBoard MSB_ONLY_64 = 0x8000000000000000ULL;
const int BOARD_SIZE = 8;
const int NUM_CELL = 64;
const int NUM_DIRECTION = 8;
const BitBoard INIT_BLACK = 0x0000000810000000ULL;
const BitBoard INIT_WHITE = 0x0000001008000000ULL;
const BitBoard LEFT_EDGE = 0x7f7f7f7f7f7f7f7fULL;
const BitBoard RIGHT_EDGE = 0xfefefefefefefefeULL;

const Direction DIRS[NUM_DIRECTION] = {
                     Direction::LEFT_UP,
                     Direction::UP,
                     Direction::RIGHT_UP,
                     Direction::RIGHT,
					 Direction::RIGHT_DOWN,
                     Direction::DOWN,
					 Direction::LEFT_DOWN,
                     Direction::LEFT};

class Board
{
 public:
  Board() : black(INIT_BLACK), white(INIT_WHITE), turn(State::BLACK), tesuu(1)
  {
    init();
  };  

  void init();
  State getState(BitBoard pos) const;    
  void display() const;

  /** Check if you can put a stone on (x, y). */
  bool canPut (BitBoard pos, bool opponent = false) const;  

  /** If the player whose turn is "turn" wins, return 1.
   * else if draw, return 0.
   * else return -1.
   */
  State getWinner() const;  

  State getTurn() const
  {
    return turn;
  };

  int getTesuu() const
  {
    return tesuu;
  };

  void changeTurn()
  {
    turn = (turn == State::BLACK ? State::WHITE : State::BLACK);
  }

  // static void displayBitBoard(BitBoard bb);


  BitBoard putStone(BitBoard pos, bool clearClds = false);
  void undo(BitBoard pos, BitBoard revPattern);

  /** Check if the game is over or not. */
  bool isEnd() const;

  /** Check if the player whose turn is "turn" must pass or not. */
  bool isPass() const;

  std::vector<BitBoard> getMoveList() const;

  bool operator==(const Board &obj) const;
  std::size_t getHash() const
  {
    return black ^ white;
  }
  static std::pair<int, int> posToXY(BitBoard);
  static BitBoard xyToPos(int x, int y);
  static bool isValidPos(BitBoard pos);

 private:
  BitBoard black;
  BitBoard white;
  State turn;
  int tesuu;  

  std::list<BitBoard> candList;
  std::stack<std::pair<int, int>> candListDiffs;
  
  void forwardUpdateCandList(BitBoard pos);
  void backUpdateCandList(BitBoard pos);  
  /* BitBoard getDoughnut(BitBoard pos) const; */
  // void displayCandListPos();

  static BitBoard transfer(BitBoard oneBit, Direction d);
};

namespace std
{
    template <>
    struct hash<Board>
    {
        std::size_t operator () (const Board& key) const
        {
            return key.getHash();
        }
    };
}

