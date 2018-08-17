#pragma once

#include "board.h"
#include "history.h"
#include <list>
#include <iostream>
#include <string>


extern void put(Board &board, std::list<History> &hist, const std::list<std::string> &args);
extern void undo(Board &board, std::list<History> &hist);
