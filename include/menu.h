#pragma once

#include "board.h"
#include "history.h"
#include "learner.h"
#include <list>
#include <iostream>
#include <string>
#include <memory>


namespace menu
{
extern void init();
extern void put(Board &board, std::list<History> &hist, const std::list<std::string> &args);
extern void undo(Board &board, std::list<History> &hist);
extern void search(Board &board, std::list<History> &hist);
extern void selfPlay(Board &board, std::list<History> &hist, const std::list<std::string> &args);
extern void learn(const std::list<std::string> &args);
extern void saveWeight(const std::list<std::string> &args);
extern void loadWeight(const std::list<std::string> &args);
}

