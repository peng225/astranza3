#include "player_test.h"

//TEST_F(PlayerTest, getScore)
//{
//    board.putStone(3, 5);
//    
//    auto val = getScore(board, 0);
//    EXPECT_DOUBLE_EQ(a, val);
//
//    auto val = getScore(board, 1);
//    auto val = getScore(board, 2);
//}


TEST_F(PlayerTest, search)
{
    // 定石を打たないケースも動かしたいので、適当にたくさんコールする
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
    pl.search(board);
}

