#include "player.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <climits>


/************************************/
/* Public methods                   */
/************************************/

BitBoard Player::search(Board& board, int numRollout, bool verbose)
{
    assert(!board.isEnd());
    auto jousekiPos = jsk.useJouseki(board, verbose);
    if(jousekiPos != 0){
        return jousekiPos;
    }

    expandedTree.clear();
    minScore = INT_MIN;
    expandThresh = static_cast<int>(0.4 * log(numRollout));

    Board rootBoard = board;
    expandedTree.emplace(board, ExpandedNode(1.0));
    std::vector<BitBoard> moveList;
    board.getMoveList(moveList);
    expand(board, moveList);
    int loopNum;
    if(moveList.size() == 1) {
        // 合法手が1つしか無い場合はroll outしてもしょうがない
        // ただし、評価値は求めたいので少なめにroll outする
        loopNum = std::max(numRollout/16, 30);
    } else {
        loopNum = numRollout;
    }
    for(int i = 0; i < loopNum; i++){
        // rootノードには親がいないので、適当にSPACEにしておく
        expandedTreeSearch(board, State::SPACE);
        board = rootBoard;
    }

    return selectMove(board, verbose);
}


/************************************/
/* Private methods                  */
/************************************/

double Player::getScore(const Board& board, int numTotalSelect,
                        State parentTurn) const
{
    auto node = &expandedTree.at(board);
    double bias = PUCT * node->prob * sqrt(numTotalSelect) / (node->numSelect + 1);
    return (board.getTurn() != parentTurn ? -node->value : node->value)
           + bias;
}

void Player::expand(Board& board, const std::vector<BitBoard>& moveList)
{
    assert(expandedTree.at(board).isLeaf);
    if(board.isEnd()){
        return;
    }

    expandedTree.at(board).isLeaf = false;
    std::vector<float> input;
    board.toVector(input);
    auto out = dn->feedInput(input);
    assert(out.back().size() == 2 + NUM_CELL);
    for(const auto& pos : moveList){
        auto xy = Board::posToXY(pos);
        auto prob = out.back().at(2 + xy.first + xy.second * BOARD_SIZE);
        auto revPattern = board.putStone(pos);
        expandedTree.emplace(board, ExpandedNode(prob));
        board.undo(pos, revPattern);
    }
}

double Player::expandedTreeSearch(Board& board, State parentTurn)
{
    assert(expandedTree.find(board) != std::end(expandedTree));
    double value;
    auto node = &expandedTree.at(board);
    node->numSelect++;

    // if(8 <= depth){
    //     std::cout << "depth: " << depth << std::endl;
    // }

    std::vector<BitBoard> moveList;
    if(!board.isEnd()){
        board.getMoveList(moveList);
    }

    if(node->isLeaf && expandThresh < node->numSelect){
        expand(board, moveList);
    }

    auto myTurn = board.getTurn();
    if(node->isLeaf){
        value = rollout(board);
    }
    else
    {
        BitBoard selectedPos = 0;
        double maxScore = minScore;
        auto parentNumSelect = node->numSelect;
        for(const auto& pos : moveList){
            auto revPattern = board.putStone(pos);
            assert(revPattern != 0);
            auto score = getScore(board, parentNumSelect, myTurn);
            // std::cout << "score: " << score << std::endl;
            if(maxScore < score){
                maxScore = score;
                selectedPos = pos;
            }
            board.undo(pos, revPattern);
        }
        auto revPattern = board.putStone(selectedPos);
        assert(revPattern != 0);
        value = expandedTreeSearch(board, myTurn);
        //auto xy = Board::posToXY(selectedPos);
        //std::cout << "selectedPos: (" << xy.first+1 << ", " 
        //          << xy.second+1 << ")" << std::endl;
    }

    //std::cout << "(value, myTurn, parentTurn) = ("
    //          << value << ", " << (int)myTurn << ", " << (int)parentTurn << ")"
    //          << std::endl;
    //std::cout << "before node value: " << node->value << std::endl;
    node->value = (node->value * (node->numSelect - 1) + value) / node->numSelect;
    //std::cout << "after node value: " << node->value << std::endl;
    return (myTurn != parentTurn ? -value : value);
}

double Player::rollout(Board& board)
{
    std::vector<BitBoard> moveList;
    auto orgTurn = board.getTurn();
    int count = 0;
    //std::cout << std::endl;
    //std::cout << "playout of " << (int)orgTurn << std::endl;
    while(!board.isEnd() && count < rolloutDepth){
        board.getMoveList(moveList);
        auto pos = moveList[dist(engine) % moveList.size()];
        board.putStone(pos);
        moveList.clear();
        count++;
    }

    if(board.isEnd()){
        auto winner = board.getWinner();
        if(winner == State::SPACE){
            return 0;
        }
        else if(winner == orgTurn){
            return 1;
        }
        else{
            return -1;
        }
    }
    else
    {
        std::vector<float> input;
        board.toVector(input);
        assert(dn != nullptr);
        auto out = dn->feedInput(input);
        assert(out.back().size() == 2 + NUM_CELL);
        if(orgTurn == State::BLACK){
            return out.back().at(0) - out.back().at(1);
        }else{
            return -out.back().at(0) + out.back().at(1);
        }
    }
}

BitBoard Player::selectMove(Board& board, bool verbose)
{
    BitBoard selectedPos = 0;
    int maxNumSelect = -1;
    std::vector<BitBoard> moveList;
    board.getMoveList(moveList);
    auto myTurn = board.getTurn();

    for(const auto& pos : moveList){
        auto revPattern = board.putStone(pos);
        assert(revPattern != 0);
        assert(expandedTree.find(board) != std::end(expandedTree));
        auto numSelect = expandedTree.at(board).numSelect;
        if(maxNumSelect < numSelect){
            maxNumSelect = numSelect;
            selectedPos = pos;
        }
        if(verbose) {
          auto xy = Board::posToXY(pos);
          std::cout << "pos: (" << xy.first + 1 << ", " << xy.second + 1
                    << "), (numSelect, value): (" << numSelect << ", "
                    << (myTurn != board.getTurn() ?
                        -expandedTree.at(board).value :
                         expandedTree.at(board).value) << ")"
                    << (myTurn != board.getTurn() ? "" : "(pass)")
                    << std::endl;
        }
        board.undo(pos, revPattern);
    }
    auto xy = Board::posToXY(selectedPos);
    if(verbose){
      std::cout << "pos: (" << xy.first + 1 << ", " << xy.second + 1
                << ") was selected" << std::endl;
    }
    board.putStone(selectedPos);
    return selectedPos;
}

