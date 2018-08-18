#include "gtest/gtest.h"
#include "player.h"

class PlayerTest : public ::testing::Test
{
protected:
    Board board;
    Player pl;

    virtual void SetUp()
    {
        board.init();
    }

    // double getScore(const Board& board, int numTotalSelect)
    // {
    //     pl.getScore(board, numTotalSelect);
    // }

    // void addToExpandedTree()
    // {
    //     board.expandedTree[board] = ExpandedNode();
    // }
};

