/*
 * curses.cxx
 * 
 * Simple testbed for curses wrapped in a class.
 * 
 * Copyright 2020 robert smith
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <ncurses.h>
using namespace std; 

class ncurseswrap
{
  public:
  ncurseswrap() { initscr(); }
  ~ncurseswrap() { endwin(); }
  void refresh() { ::refresh(); }
};

int main(int argc, char **argv)
{
  int nrows, ncols;
  ncurseswrap nc;
  getmaxyx(stdscr, nrows, ncols);
  mvprintw(nrows - 1, 0, "This screen has %d rows and %d columns\n", nrows, ncols);   
  for (int n = 0; n < nrows; n++) {
    mvprintw(n, n, "X");
    nc.refresh();
    this_thread::sleep_for(chrono::seconds(1));
  }
  return 0;
}
