#include "board.h"
#include "history.h"
#include "menu.h"
#include <assert.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>

const int MAX_HISTORY_NUM = 100;

int main(int argc, char *argv[])
{
  Board board;
  std::list<History> hist;
  int depth = DEFAULT_ROLLOUT_DEPTH;

  board.display();
  menu::init();

  int history_no = 0;
  HIST_ENTRY *history = NULL;
  char* line;
  const char *prompt = "> ";
  while ((line = readline(prompt)) != NULL) {
    std::string command;
    std::list<std::string> cmdAndArgs;
    boost::split(cmdAndArgs, line, boost::is_space());
    cmdAndArgs.remove("");
    if(cmdAndArgs.empty()){
      command = "";
    }else{
      command = *begin(cmdAndArgs);
      cmdAndArgs.erase(begin(cmdAndArgs));
    }

    if(command == "self" || command == "se"){
      menu::selfPlay(board, hist, cmdAndArgs);
    }
    else if(command == "undo" || command == "u"){
      menu::undo(board, hist);
    }
    else if(command == "setDepth" || command == "sd"){
      if(cmdAndArgs.size() < 1){
        std::cerr << "The depth is required." << std::endl;
      }else{
	    depth = atoi(begin(cmdAndArgs)->c_str());
	    std::cout << "Depth was changed to " << depth << "." << std::endl;
      }
    }    
    else if(command == "search" || command == "s"){
      menu::search(board, hist, depth);
    }
//    else if(command == "setTime" || command == "st"){
//      int time;
//      // (*ist) >> time;
//      if(cmdAndArgs.size() < 1){
//        std::cerr << "The search time is required." << std::endl;
//      }else{
//        time = atoi(begin(cmdAndArgs)->c_str());
//        // ai.setTime(time);
//        std::cout << "Search time was changed to " << time << "." << std::endl;
//      }
//    }
    else if(command == "learn" || command == "l"){
      menu::learn(cmdAndArgs);
    }
    // else if(command == "generate" || command == "g") gen_kifu(*ist);
    else if(command == "init" || command == "i"){
      board.init();
      hist.clear();
      board.display();
      // ai.init();
    }
    else if(command == "put" || command == "p"){
      menu::put(board, hist, cmdAndArgs);
    }
    else if(command == "display" || command == "d"){
      board.display();
    }
    else if(command == "save" || command == "sv"){
      menu::saveWeight(cmdAndArgs);
    }
    else if(command == "load" || command == "ld"){
      menu::loadWeight(cmdAndArgs);
    }
    else if(command == "evolve" || command == "ev"){
      menu::evolve(cmdAndArgs);
    }
    else if(command == "quit" || command == "q"){
      break;    
    }
    else if(command == ""){
    }
    else{
      std::cerr << "No such command exists." << std::endl;
    }
    
    // delete ist;
    cmdAndArgs.clear();
    add_history(line);
    if (++history_no > MAX_HISTORY_NUM) {
      history = remove_history(0);
      free(history);
    }
    free(line);
  }
  clear_history();
  return 0;
}
