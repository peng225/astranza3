#pragma once

#include "board.h"
#include "jouseki.h"
#include <unordered_map>
#include <random>
#include <cmath>

const int NUM_ROLLOUT = 10000;
const double MIN_SCORE = -NUM_ROLLOUT * 100;
const double PUCT = 0.5;
const int EXPAND_THRESH = static_cast<int>(0.4 * log(NUM_ROLLOUT));

struct ExpandedNode
{
    ExpandedNode() : isLeaf(true), numSelect(0), value(0){}
    bool isLeaf;
    int numSelect;
    double value;
};

class Player
{
friend class PlayerTest;
public:
    Player() : engine(seed_gen()), dist(0, 100000000)
    {
        jsk.readJousekiFile();
    }
    BitBoard search(Board& board);

private:
    std::unordered_map<Board, ExpandedNode> expandedTree;
    std::random_device seed_gen;
    std::mt19937 engine;
    std::uniform_int_distribution<> dist;
    Jouseki jsk;

    double getScore(const Board& board, int numTotalSelect,
                    State parentTurn) const;
    void expand(Board& board, const std::vector<BitBoard>& moveList);
    double expandedTreeSearch(Board& board, State parentTurn);
    double rollout(Board& board);
    BitBoard selectMove(Board& board);
};

