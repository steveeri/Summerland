/*
 * InfinityGameLogic.hpp
 *
 *  Created on: 25 Sep 2018
 *      Author: steve
 */

#pragma once

#include "smlnd_log.hpp"

#include <string>
#include <map>


namespace smlnd {

// Enumeration values
enum Glyph {
  SBAR = 0, SARC = 1, DARC = 2, TRIO = 3, LEND = 4, QUAD = 5, BLNK = -1
};
enum Edges {
  N = 0, S = 180, E = 90, W = 270   // north, south, east, and west.
};

const int INF_EDGES = 4;                  // Standard number of tile edges.
const int INF_ANGLEOFFSET = 90;           // Standard user press rotation selected offset.
const float INF_ANIMATIONSPEED = 0.001f;   // tile animation speed. Needs to be moderated relative to ANGLEDELTA.
const float INF_ANGLEDELTA = 3.75f;       // tile rotations speeds. Must be integer integral of INF_ANGLEOFFSET.

/**
 * A struct to contain error data if required.
 */
struct InfinityRpt {
  enum Type {
    OK, MSG, DEBUG, ERROR, FATAL
  } type = OK;
  std::string id = "OK";
  std::string msg = "no errors occurred";
};

/**
 * This class holds the details pertaining to the current state of a game cell
 * in particulars this class relates details of the edges and alignment mappings, identify
 * the sprite sheet graphic entity that displays the tile.
 * There are currently 6 tile variants.
 */
class GameCell {

private:
  bool animating = false;
  unsigned short flags = 0;

public:
  int x, y;
  int initAngle = 0, targetAngle = 0;
  float curAngle = 0;
  float lastUpdated = 0;
  int numEdgesJoined;
  int edges[INF_EDGES];
  Glyph glyph;

public:
  GameCell(const int x, const int y, const int tileAngle, const Glyph glyph, const unsigned short flags);
  virtual ~GameCell();
  void rotate();
  void update(const float fElaspedTime);
  float getCellRotation() { return (this->curAngle); };
  void checkEdges();
  bool isComplete();
};


/**
 * This class holds the details pertaining to the current level
 */
class Level {

private:
  std::map<std::string, GameCell*> cells;
  bool complete = false;

public:
  int id = 0;
  std::string name;
  std::string spriteName;
  int gridCols = 0, gridRows = 0;

public:
  Level(const int id, const std::string name, const char* layout);
  virtual ~Level();
  std::map<std::string, GameCell*>& getGameCells();
  void rotateTile(int x, int y);
  bool update(const float fElaspedTime);
  bool isComplete();
  bool getSize() { return (this->cells.size()); }
};


class InfinityGameLogic {

private:
  bool complete = false;

public:
  Level* level;  // An object holding all levels defined.

public:
  InfinityGameLogic(const std::string name, const char* layout);
  virtual ~InfinityGameLogic();
  bool loadNewLevel(const int id, const std::string name, const char* layout);
  void rotateTile(int x, int y);
  bool update(const float fElaspedTime);
  bool levelComplete();
};

} /* namespace smlnd */

