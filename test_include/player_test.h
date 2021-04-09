#include "gtest/gtest.h"
#include "player.h"
#include <cnn.h>
#include <memory>

class PlayerTest : public ::testing::Test
{
public:
    PlayerTest() : pl(std::make_shared<DeepNetwork>(), 4){}
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

