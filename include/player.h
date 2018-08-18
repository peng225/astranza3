#pragma once

#include "board.h"
#include "jouseki.h"
#include <unordered_map>
#include <random>
#include <cmath>

const int NUM_ROLLOUT = 3000;
const double MIN_SCORE = -NUM_ROLLOUT * 100;
const double PCUT = 0.5;
const int EXPAND_THRESH = static_cast<int>(sqrt(NUM_ROLLOUT));

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
    State me;
    std::random_device seed_gen;
    std::mt19937 engine;
    std::uniform_int_distribution<> dist;
    Jouseki jsk;

    double getScore(const Board& board, int numTotalSelect) const;
    void expand(Board& board);
    double expandedTreeSearch(Board& board);
    double rollout(Board& board);
    BitBoard selectMove(Board& board);
};

