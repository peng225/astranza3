#include "player.h"
#include <iostream>
#include <vector>
#include <cassert>


/************************************/
/* Public methods                   */
/************************************/

BitBoard Player::search(Board& board)
{
    assert(!board.isEnd());
    auto jousekiPos = jsk.useJouseki(board);
    if(jousekiPos != 0){
        return jousekiPos;
    }

    expandedTree.clear();

    Board rootBoard = board;
    expandedTree[board] = ExpandedNode();
    std::vector<BitBoard> moveList;
    board.getMoveList(moveList);
    expand(board, moveList);
    me = board.getTurn();
    for(int i = 0; i < NUM_ROLLOUT; i++){
        expandedTreeSearch(board, 0);
        board = rootBoard;
    }

    return selectMove(board);
}


/************************************/
/* Private methods                  */
/************************************/

double Player::getScore(const Board& board, int numTotalSelect) const
{
    auto node = &expandedTree.at(board);
    double bias = PUCT * sqrt(numTotalSelect) / (node->numSelect + 1);
    return node->value + bias;
}

void Player::expand(Board& board, const std::vector<BitBoard>& moveList)
{
    assert(expandedTree[board].isLeaf);
    if(board.isEnd()){
        return;
    }

    expandedTree[board].isLeaf = false;
    for(const auto& pos : moveList){
        auto revPattern = board.putStone(pos);
        expandedTree[board] = ExpandedNode();
        board.undo(pos, revPattern);
    }
}

double Player::expandedTreeSearch(Board& board, int depth)
{
    assert(expandedTree.find(board) != std::end(expandedTree));
    double value;
    auto node = &expandedTree[board];
    node->numSelect++;

    //if(8 <= depth){
    //    std::cout << "depth: " << depth << std::endl;
    //}

    std::vector<BitBoard> moveList;
    if(!board.isEnd()){
        board.getMoveList(moveList);
    }

    if(node->isLeaf && EXPAND_THRESH < node->numSelect){
        expand(board, moveList);
    }

    if(node->isLeaf){
        value = rollout(board);
    }
    else
    {
        BitBoard selectedPos;
        double maxScore = MIN_SCORE;
        auto parentNumSelect = node->numSelect;
        for(const auto& pos : moveList){
            auto revPattern = board.putStone(pos);
            assert(revPattern != 0);
            auto score = getScore(board, parentNumSelect);
            // std::cout << "score: " << score << std::endl;
            if(maxScore < score){
                maxScore = score;
                selectedPos = pos;
            }
            board.undo(pos, revPattern);
        }
        auto revPattern = board.putStone(selectedPos);
        assert(revPattern != 0);
        value = expandedTreeSearch(board, depth + 1);
    }

    node->value = (node->value * (node->numSelect - 1) + value) / node->numSelect;
    return value;
}

double Player::rollout(Board& board)
{
    std::vector<BitBoard> moveList;
    while(!board.isEnd()){
        board.getMoveList(moveList);
        auto pos = moveList.at(dist(engine) % moveList.size());
        board.putStone(pos);
        moveList.clear();
    }
    auto winner = board.getWinner();
    if(winner == State::SPACE){
        return 0;
    }
    else if(winner == me){
        return 1;
    }
    else{
        return -1;
    }
}

BitBoard Player::selectMove(Board& board)
{
    BitBoard selectedPos;
    int maxNumSelect = -1;
    std::vector<BitBoard> moveList;
    board.getMoveList(moveList);

    for(const auto& pos : moveList){
        auto revPattern = board.putStone(pos);
        assert(revPattern != 0);
        assert(expandedTree.find(board) != std::end(expandedTree));
        auto numSelect = expandedTree[board].numSelect;
        if(maxNumSelect < numSelect){
            maxNumSelect = numSelect;
            selectedPos = pos;
        }
        auto xy = Board::posToXY(pos);
        std::cout << "pos: (" << xy.first + 1 << ", " << xy.second + 1
                  << "), value: " << numSelect << std::endl;
        board.undo(pos, revPattern);
    }
    auto xy = Board::posToXY(selectedPos);
    std::cout << "pos: (" << xy.first + 1 << ", " << xy.second + 1
              << ") was selected" << std::endl;
    board.putStone(selectedPos);
    return selectedPos;
}

