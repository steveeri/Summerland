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

const short E_SBAR[] = { 0, -1, 180, -1 };
const short E_SARC[] = { 0, 90, -1, -1 };
const short E_DARC[] = { 0, 90, 180, 270 };
const short E_TRIO[] = { 0, 90, 180, -1 };
const short E_LEND[] = { 0, -1, -1, -1 };
const short E_QUAD[] = { 0, 90, 180, 270 };
const short E_BLNK[] = { -1, -1, -1, -1 };

enum Edge {
  N = 0, S = 180, E = 90, W = 270   // north, south, east, and west.
};

const int INF_EDGES = 4;                  // Standard number of tile edges.
const int INF_ANGLEOFFSET = 90;           // Standard user press rotation selected offset.
const float INF_ANIMATIONSPEED = 0.0009f; // tile animation speed. Needs to be moderated relative to ANGLEDELTA.
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

public:
  int x, y;
  int initAngle = 0, targetAngle = 0;
  float curAngle = 0;
  float lastUpdated = 0.0f;
  short* edges;

  Glyph glyph;

private:
  GameCell(const GameCell&) = delete;
  GameCell& operator=(const GameCell&) = delete;

public:
  GameCell(const int x, const int y, const std::string s_glyph);
  virtual ~GameCell();
  void rotate();
  void update(const float fElaspedTime);
  float getCellRotation() {
    return (this->curAngle);
  }
  ;
  bool matchEdge(const short edge);
};

/**
 * This class holds the details pertaining to the current level
 */
class Level {

private:
  std::map<std::pair<int, int>, GameCell*> cells;
  bool complete = false;

public:
  int id = 0;
  std::string name;
  std::string spriteName;
  int gridCols = 0, gridRows = 0;

private:
  Level(const Level&) = delete;
  Level& operator=(const Level&) = delete;

public:
  Level(const int id, const std::string name, const char* layout);
  virtual ~Level();
  std::map<std::pair<int, int>, GameCell*>& getGameCells();
  void rotateTile(int x, int y);
  bool update(const float fElaspedTime);
  bool isComplete();

  bool getSize() {
    return (this->cells.size());
  }
};

class InfinityGameLogic {

private:
  bool complete = false;
  unsigned short lvlCleared = 0;

public:
  Level* level;  // An object holding all levels defined.

public:
  InfinityGameLogic(const std::string name, const char* layout);
  virtual ~InfinityGameLogic();
  bool loadNewLevel(const int id, const std::string name, const char* layout);
  void rotateTile(int x, int y);
  bool update(const float fElaspedTime);
  unsigned short levelCleared() {
    return (this->lvlCleared);
  }
  void setLevelComplete(const unsigned short levelId);
  unsigned short getLevelComplete();
  bool isLevelComplete();
};

} /* namespace smlnd */

