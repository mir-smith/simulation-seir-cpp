/*
 * infection.cxx
 * 
 * SEIR infection model, working but needs work.
 * - Clean of integer use of enum.
 * - Input population as percentage of cells on screen.
 * - Graph on completion, etc, ad infinitum.
 * 
 * Copyright 2020 robert smith
 */

#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <ncurses.h>
using namespace std;

#define EMPTY (-1)
#define POPULATION 100
#define NTICKS (100 * 24)

enum health_state {Susceptible, Exposed, Infected, Recovered};

/*
 * Simple wrapper around ncurses.
 */
class ncurseswrap
{
  public:
  ncurseswrap() { initscr(); raw(); noecho(); }
  ~ncurseswrap() { endwin(); }
  void refresh() { ::refresh(); }
};

/*
 * Humans are just numbers (even #6) in one of four health states.
 */
class Human {
    int id;
    health_state state;
    unsigned int pos;
    int time_to_change;
  public:
    Human(int);
    string info(void) { return ("<Human id=" + to_string(id) + " pos=" + to_string(pos) + " state=" + getHealthString() + ">"); }
    int getId(void) { return id; }
    void setPos(unsigned int p) { pos = p; }
    unsigned int getPos(void) { return pos; }
    int get_health_state(void) { return state; }
    string getHealthString(void);
    char getHealthChar(void);
    void touch(Human);
    void infect(void);
    void tick(void);
    void draw(unsigned int);
};

Human::Human(int a) {
  id = a;
  state = Susceptible;
}

// TODO explicit integer for state to match colour attributes
void Human::draw(unsigned int width) {
  int c = 1;
  switch (state) {
  case Susceptible: c = 1; break;
  case Exposed: c = 2; break;
  case Infected: c = 3; break;
  case Recovered: c = 4; break;
  }
  attron(COLOR_PAIR(c));
  mvprintw(pos / width, pos % width, "%c", getHealthChar());
  attroff(COLOR_PAIR(c));
}

string Human::getHealthString() {
  switch (state) {
  case Susceptible: return "Susceptible";
  case Exposed: return "Exposed";
  case Infected: return "Infected";
  case Recovered: return "Recovered";
  default: return "?UNKNOWN?";
  }
}

char Human::getHealthChar() {
   return getHealthString()[0];
}

void Human::touch(Human contact) {
  if (state == Infected && contact.state == Susceptible) {
    contact.infect();
  } else if (state == Susceptible && contact.state == Infected) {
    infect();
  }
}

void Human::infect() {
  state = Exposed;
  time_to_change = -5 * 24;    // I'll be infectious in 5 days
}

void Human::tick() {
  if (++time_to_change == 0) {
    if (state == Exposed) {
      state = Infected;
      time_to_change = -10 * 24;	// I'll recover in 10 days
    } else if (state == Infected) {
      state = Recovered;
    }
  }
}

/*
 * The simulated world is a 2D array inhabited by a population of humans.
 * Each grid cell is empty or holds one human.
 */
class World {
  private:
    ncurseswrap nc;
    unsigned int height, width, grid_size;
    vector<int> grid;
    vector<Human> population;
  public:
    void genesis(unsigned int);
    unsigned int tick(int);
    //void census(string);
    unsigned int rand_grid_pos(void) { return (static_cast<double>(std::rand()) / RAND_MAX * grid_size); }
    unsigned int rand_direction(void) { return (static_cast<double>(std::rand()) / RAND_MAX * 4); }
};

void World::genesis(unsigned int npop) {
  getmaxyx(stdscr, height, width);
  height -= 1; // save botton row for text
  grid_size = width * height;
  for (unsigned int n = 0; n < grid_size; n++) {
    grid.push_back(EMPTY);
  }
  for (unsigned int n = 0; n < npop; n++) {
    Human h = Human(n);
    unsigned int pos;
    while (true) {
      //pos = rand() % grid_size;
      pos = rand_grid_pos();
      if (grid.at(pos) == EMPTY) {
        break;
      } 
    }
    grid.at(pos) = h.getId();
    h.setPos(pos);
    //mvprintw(pos / width, pos % width, "%c", h.getHealthChar());
    h.draw(width);
    population.push_back(h);
  }
  population.at(0).infect();
  nc.refresh();
}

unsigned int World::tick(int t) {
  int count[4] = { 0, 0, 0, 0 };
  for (vector<Human>::iterator it = population.begin() ; it != population.end(); ++it) {
    int pos = it->getPos();
    int new_pos = pos;
    switch (rand_direction()) {
    case 0: new_pos = (pos + width) % grid_size;
            break;
    case 1: new_pos = (pos - width) % grid_size;
            break;
    case 2: new_pos = (pos + 1) % grid_size;
            break;
    case 3: new_pos = (pos - 1) % grid_size;
    }
    if (grid.at(new_pos) == EMPTY) {
      grid.at(pos) = EMPTY;
      grid.at(new_pos) = it->getId();
      it->setPos(new_pos);
      mvprintw(pos / width, pos % width, " ");
      //mvprintw(new_pos / width, new_pos % width, "%c", it->getHealthChar());
      it->draw(width);
    } else {
      it->touch(population.at(grid.at(new_pos)));
    }
    it->tick();
    switch (it->get_health_state()) {
    case Susceptible: count[0]++; break;
    case Exposed: count[1]++; break;
    case Infected: count[2]++; break;
    case Recovered: count[3]++; break;
    }
  }
  mvprintw(height, 0, "T=%6.2f S=%5d E=%5d I=%5d R=%5d", t/24.0, count[0], count[1], count[2], count[3]);
  ofstream myfile;
  myfile.open("output.csv", ofstream::app);
  myfile << t << "," << count[0] << "," << count[1] << "," << count[2] << "," << count[3] << endl; 
  myfile.close();
  nc.refresh();
  return count[1] + count[2];
}

/*
void World::census(string msg) {
  cout << "census: " << msg << endl;
  for (vector<Human>::iterator it = population.begin() ; it != population.end(); it++) {
    cout << "census: "<< it->info() << endl;
  }
}
*/

int main (int argc, char **argv) {  
  World world;
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);
  init_pair(4, COLOR_GREEN, COLOR_BLACK);
  srand((unsigned) time(0));  // random seed
  world.genesis(POPULATION);
  for (int t = 0; t < NTICKS; t++) {
    this_thread::sleep_for(chrono::milliseconds(20));
    if (world.tick(t) < 1) break;
  }
  return 0;
}
