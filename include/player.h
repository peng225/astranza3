#pragma once

#include "board.h"
#include "jouseki.h"
#include "cnn.h"
#include <unordered_map>
#include <random>
#include <cmath>
#include <memory>

constexpr int NUM_DEFAULT_ROLLOUT = 1600;
constexpr double PUCT = 0.5;
constexpr int DEFAULT_ROLLOUT_DEPTH = 4;

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
    Player(std::shared_ptr<DeepNetwork> dnet, int rdepth) : engine(seed_gen()), dist(0, 100000000), dn(dnet), rolloutDepth(rdepth)
    {
        jsk.readJousekiFile();
    }
    //Player() : engine(seed_gen()), dist(0, 100000000)
    //{
    //    jsk.readJousekiFile();
    //}
    BitBoard search(Board& board,
                    int numRollout = NUM_DEFAULT_ROLLOUT,
                    bool verbose = true);

private:
    std::unordered_map<Board, ExpandedNode> expandedTree;
    std::random_device seed_gen;
    std::mt19937 engine;
    std::uniform_int_distribution<> dist;
    Jouseki jsk;
    std::shared_ptr<DeepNetwork> dn;
    int rolloutDepth;
    int minScore;
    int expandThresh;

    double getScore(const Board& board, int numTotalSelect,
                    State parentTurn) const;
    void expand(Board& board, const std::vector<BitBoard>& moveList);
    double expandedTreeSearch(Board& board, State parentTurn);
    double rollout(Board& board);
    BitBoard selectMove(Board& board, bool verbose);
};

