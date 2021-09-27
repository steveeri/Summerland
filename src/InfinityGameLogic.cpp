/*
 * InfinityGameLogic.cpp
 *
 *  Created on: 25 Sep 2018
 *      Author: steve
 */

#include "InfinityGameLogic.hpp"

#include <sstream>

namespace smlnd {


GameCell::GameCell(const int x, const int y, const std::string s_glyph) {

  this->x = x;
  this->y = y;

  auto getGlyphEdges = [&](const Glyph glyph) {
    if (glyph == SBAR) return (&E_SBAR);
    if (glyph == SARC) return (&E_SARC);
    if (glyph == DARC) return (&E_DARC);
    if (glyph == TRIO) return (&E_TRIO);
    if (glyph == LEND) return (&E_LEND);
    if (glyph == QUAD) return (&E_QUAD);
    return (&E_BLNK);
  };

  auto getGlyphType = [&](const std::string& glyphName) {
    if (glyphName == "SBAR") return (Glyph::SBAR);
    if (glyphName == "SARC") return (Glyph::SARC);
    if (glyphName == "DARC") return (Glyph::DARC);
    if (glyphName == "TRIO") return (Glyph::TRIO);
    if (glyphName == "LEND") return (Glyph::LEND);
    if (glyphName == "QUAD") return (Glyph::QUAD);
    return (Glyph::BLNK);
  };

  this->glyph = getGlyphType(s_glyph);
  this->curAngle = (rand() % INF_EDGES) * INF_ANGLEOFFSET;
  this->targetAngle = this->curAngle;
  this->edges = (short*) *getGlyphEdges(this->glyph);
}

GameCell::~GameCell(){}


void GameCell::rotate() {

  // Guard: Don't act on user input while animating tile.
  if (this->animating || this->glyph == BLNK) return;

  // Rotate tile to new target position.
  targetAngle = curAngle + INF_ANGLEOFFSET;
  animating = true;
}


/**
 * Update the tiles based on any remaining move actions.
 * Only animate if animating and time-slice since last update
 * is less than Desired animation speed.
 */
void GameCell::update(float fElaspedTime) {

  if (!animating || this->glyph == BLNK) return;

  // Update last updated value.
  this->lastUpdated += fElaspedTime;

  // Conditional time-slice update to tile rotation.
  if (this->lastUpdated >= INF_ANIMATIONSPEED) {

    this->lastUpdated = 0.0f;

    if (curAngle != targetAngle) {
      curAngle += INF_ANGLEDELTA;
    } else {
      animating = false;
    }
  }
}


bool GameCell::matchEdge(const short edgeAngle) {

  // Some guard checks.
  if (this->animating || this->glyph == BLNK) return (false);

  for (short i = 0; i < INF_EDGES; i++) {
    if (this->edges[i] == -1) continue;  // skip blank edges...
    int ta = ((this->edges[i] + static_cast<int>(this->curAngle))) % 360;
    if (ta == edgeAngle) return (true);
  }

  // Default is no match
  return (false);
}


Level::Level(const int id, const std::string name, const char* layout) {
  this->id = id;
  this->name = name;

  // Process layout to build cells.
  std::istringstream data(layout);

  data >> this->spriteName;
  data >> this->gridCols;
  data >> this->gridRows;

  int size = this->gridCols * this->gridRows;

  for (int i = 0; i < size && !data.eof(); i++) {
    std::string s_glyph;

    data >> s_glyph;
    int x = i%this->gridCols;
    int y = i/this->gridCols;

    GameCell* cell = new GameCell(x, y, s_glyph);
    this->cells.emplace(std::pair<int, int>(x, y), cell);
  }
}


Level::~Level() {
  SMLND_DBG_LOG("Level deconstructor called");
  for (auto& x : this->cells)
    delete x.second;
  this->cells.clear();
}


std::map<std::pair<int, int>, GameCell*>& Level::getGameCells() {
  return (cells);
}


void Level::rotateTile(int x, int y) {

  if (this->complete) return;

  if (x >= 0 && x < this->gridCols && y >= 0 && y < this->gridRows) {
    GameCell* cell = this->cells[std::pair<int, int>(x, y)];
    if (cell != nullptr) cell->rotate();
  }
}


bool Level::update(const float fElaspedTime) {

  if (this->complete) return (true);
  for (auto& x: this->cells) x.second->update(fElaspedTime);
  return (true);
}


/**
 * This method will check with adjacent edges and confirm if all edges are aligned
 * with partner edges. This function will return false at the earliest opportunity.
 */
bool Level::isComplete() {

  if (this->complete) return (true);

  for (unsigned short x = 0; x < this->gridCols; x++) {
    for (unsigned short y = 0; y < this->gridRows; y++) {

      std::pair<int, int> indx(x, y);

      GameCell* cell = this->cells[std::pair<int, int>(x, y)];
      if (cell == nullptr) return (false);

      if (cell->matchEdge(Edge::N)) {
        if (y == 0) return (false);
        if (!this->cells[std::pair<int, int>(x, y - 1)]->matchEdge(Edge::S)) return (false);
      }

      if (cell->matchEdge(Edge::S)) {
        if (y >= this->gridRows - 1) return (false);
        if (!this->cells[std::pair<int, int>(x, y + 1)]->matchEdge(Edge::N)) return (false);
      }

      if (cell->matchEdge(Edge::E)) {
        if (x >= this->gridCols - 1) return (false);
        if (!this->cells[std::pair<int, int>(x + 1, y)]->matchEdge(Edge::W)) return (false);
      }

      if (cell->matchEdge(Edge::W)) {
        if (x == 0) return (false);
        if (!this->cells[std::pair<int, int>(x - 1, y)]->matchEdge(Edge::E)) return (false);
      }
    }
  }

  this->complete = true;
  return (this->complete);
}


InfinityGameLogic::InfinityGameLogic(const std::string name, const char* layout) {
  this->complete = false;
  this->level = new Level(1, name, layout);
}


bool InfinityGameLogic::loadNewLevel(const int id, const std::string name, const char* layout) {
  this->complete = false;
  delete this->level;  // delete memory allocations for previous level.
  this->level = new Level(id, name, layout);
  if (this->level == nullptr) return (false);
  return (true);
}


InfinityGameLogic::~InfinityGameLogic() {
  delete this->level;
}


void InfinityGameLogic::rotateTile(int x, int y) {
  if (this->level != nullptr) this->level->rotateTile(x, y);
}


bool InfinityGameLogic::update(const float fElaspedTime) {
  return ((this->level != nullptr) ? this->level->update(fElaspedTime) : false);
}


bool InfinityGameLogic::isLevelComplete() {
  bool completed = (this->level != nullptr) ? this->level->isComplete() : false;
  if (completed && (this->level->id > this->lvlCleared)) this->lvlCleared = this->level->id;
  return (completed);
}

void InfinityGameLogic::setLevelComplete(const unsigned short levelId) {
  this->lvlCleared = levelId;
}

unsigned short InfinityGameLogic::getLevelComplete() {
  return (this->lvlCleared);
}

} /* namespace smlnd */
