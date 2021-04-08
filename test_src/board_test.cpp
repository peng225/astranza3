#include "gtest/gtest.h"
#include "board.h"


TEST(board, posToXY)
{
    auto xy = Board::posToXY(MSB_ONLY_64);
    EXPECT_EQ(0, xy.first);
    EXPECT_EQ(0, xy.second);

    xy = Board::posToXY(MSB_ONLY_64 >> 20);
    EXPECT_EQ(4, xy.first);
    EXPECT_EQ(2, xy.second);
}

TEST(board, xyToPos)
{
    EXPECT_EQ(MSB_ONLY_64, Board::xyToPos(0, 0));
    EXPECT_EQ((MSB_ONLY_64 >> 1), Board::xyToPos(1, 0));
    EXPECT_EQ((MSB_ONLY_64 >> 9), Board::xyToPos(1, 1));
}

TEST(board, isValidPos)
{
    EXPECT_TRUE(Board::isValidPos(MSB_ONLY_64));
    EXPECT_FALSE(Board::isValidPos(3));
    EXPECT_FALSE(Board::isValidPos(9));
    EXPECT_FALSE(Board::isValidPos(MSB_ONLY_64 | 9));
}

TEST(board, basic)
{
    Board board;
    board.init();
    EXPECT_EQ(State::WHITE, board.getState(Board::xyToPos(3, 3)));
    EXPECT_EQ(State::WHITE, board.getState(Board::xyToPos(4, 4)));
    EXPECT_EQ(State::BLACK, board.getTurn());
    EXPECT_TRUE(board.canPut(Board::xyToPos(2, 3)));

    board.changeTurn();
    EXPECT_EQ(State::WHITE, board.getTurn());
}

TEST(board, undo)
{
    Board board;
    board.init();
    auto rp = board.putStone(Board::xyToPos(5, 4));
    EXPECT_NE(0ULL, rp);

    auto orgBoard = board;
    rp = board.putStone(Board::xyToPos(3, 5));
    EXPECT_NE(0ULL, rp);

    board.undo(Board::xyToPos(3, 5), rp);

    EXPECT_EQ(orgBoard, board);
}

TEST(board, toVector)
{
    Board board;
    board.init();
    std::vector<float> input(2 * NUM_CELL);

    board.toVector(input);
    EXPECT_EQ(0, input.at(0));
    EXPECT_EQ(1, input.at(28));
    EXPECT_EQ(1, input.at(35));
    EXPECT_EQ(0, input.at(NUM_CELL - 1));

    EXPECT_EQ(0, input.at(NUM_CELL));
    EXPECT_EQ(1, input.at(NUM_CELL + 27));
    EXPECT_EQ(1, input.at(NUM_CELL + 36));
    EXPECT_EQ(0, input.at(2 * NUM_CELL - 1));
}

TEST(board, transfer)
{
    BitBoard pos = MSB_ONLY_64;
    EXPECT_EQ((pos >> 1), Board::transfer(pos, Direction::RIGHT));

    pos >>= 7;
    EXPECT_EQ(0ULL, Board::transfer(pos, Direction::RIGHT));

    pos = MSB_ONLY_64;
    EXPECT_EQ(pos >> 9, Board::transfer(pos, Direction::RIGHT_DOWN));

    pos >>= 9;
    EXPECT_EQ(MSB_ONLY_64, Board::transfer(pos, Direction::LEFT_UP));
}

TEST(board, flipBBWithLTtoRDDiagonal)
{
    BitBoard pos = MSB_ONLY_64;
    EXPECT_EQ(pos, Board::flipBBWithLTtoRDDiagonal(pos));

    pos = 1ULL;
    EXPECT_EQ(pos, Board::flipBBWithLTtoRDDiagonal(pos));

    pos = MSB_ONLY_64;
    pos >>= 7;
    EXPECT_EQ(1ULL << 7, Board::flipBBWithLTtoRDDiagonal(pos));

    pos = 0x123456789accdef0ULL;
    EXPECT_EQ(pos, Board::flipBBWithLTtoRDDiagonal(Board::flipBBWithLTtoRDDiagonal(pos)));
}

TEST(board, flipBBWithRTtoLDDiagonal)
{
    BitBoard pos = MSB_ONLY_64;
    EXPECT_EQ(1ULL, Board::flipBBWithRTtoLDDiagonal(pos));

    pos = 1ULL;
    EXPECT_EQ(MSB_ONLY_64, Board::flipBBWithRTtoLDDiagonal(pos));

    pos = MSB_ONLY_64;
    pos >>= 7;
    EXPECT_EQ(pos, Board::flipBBWithRTtoLDDiagonal(pos));

    pos = 0x123456789accdef0ULL;
    EXPECT_EQ(pos, Board::flipBBWithRTtoLDDiagonal(Board::flipBBWithRTtoLDDiagonal(pos)));
}

TEST(board, flipBBMix)
{
    BitBoard orgPos = 0x123456789accdef0ULL;
    BitBoard pos = orgPos;
    pos = Board::flipBBWithLTtoRDDiagonal(pos);
    pos = Board::flipBBWithRTtoLDDiagonal(pos);
    pos = Board::flipBBWithLTtoRDDiagonal(pos);
    pos = Board::flipBBWithRTtoLDDiagonal(pos);
    EXPECT_EQ(orgPos, pos);

    pos = Board::flipBBWithRTtoLDDiagonal(pos);
    pos = Board::flipBBWithLTtoRDDiagonal(pos);
    pos = Board::flipBBWithRTtoLDDiagonal(pos);
    pos = Board::flipBBWithLTtoRDDiagonal(pos);
    EXPECT_EQ(orgPos, pos);
}
