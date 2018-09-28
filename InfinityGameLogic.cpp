/*
 * InfinityGameLogic.cpp
 *
 *  Created on: 25 Sep 2018
 *      Author: steve
 */

#include "InfinityGameLogic.hpp"

#include <sstream>

namespace smlnd {


GameCell::GameCell(const int x, const int y, const int tileAngle, const Glyph glyph, const unsigned short flags) {

  this->x = x;
  this->y = y;
  this->curAngle = tileAngle;
  this->targetAngle = tileAngle;
  this->numEdgesJoined = 0;
  lastUpdated = 0.0f;
  this->glyph = glyph;
  this->flags = flags;
}

GameCell::~GameCell(){}

void GameCell::rotate() {

  // Guard: Don't act on user input while animating tile.
  if (this->animating) return;

  // Rotate tile to new target position.
  targetAngle = curAngle + INF_ANGLEOFFSET;
  animating = true;
}

/**
 * Update the tiles based on any outstanding move actions.
 * Only animate if animating and time-slice since last update
 * is less than Desired animation speed.
 */
void GameCell::update(float fElaspedTime) {

  if (this->glyph == BLNK) return;

  // Update last updated value.
  if (animating) this->lastUpdated += fElaspedTime;

  // Conditional time-slice update to tile rotation.
  if (animating && (this->lastUpdated >= INF_ANIMATIONSPEED)) {
    this->lastUpdated = 0.0f;
    if (curAngle != targetAngle) {
      curAngle += INF_ANGLEDELTA;
    } else {
      this->lastUpdated = 0.0f;
      animating = false;
    }
  }
}

void GameCell::checkEdges() {
  // todo - some logic needed here.
}

/**
 * This method will check with adjacent edges and confirm if all edges are aligned
 * with partner edges.
 */
bool GameCell::isComplete() {
  // todo - some logic needed here.
return (false);
}


Level::Level(const int id, const std::string name, const char* layout) {
  this->id = id;
  this->name = name;

  auto getGlyphType = [&](const std::string& glyphName) {
    if (glyphName == "SBAR") return (Glyph::SBAR);
    if (glyphName == "SARC") return (Glyph::SARC);
    if (glyphName == "DARC") return (Glyph::DARC);
    if (glyphName == "TRIO") return (Glyph::TRIO);
    if (glyphName == "LEND") return (Glyph::LEND);
    if (glyphName == "QUAD") return (Glyph::QUAD);
    return (Glyph::BLNK);
  };

  auto getGlyphAngle = [&](const unsigned short& flags) {
    if (flags & 0x10) return (90.0f);
    if (flags & 0x20) return (180.0f);
    if (flags & 0x40) return (270.0f);
    return (0.0f);
  };


  // Process layout to build cells.
  std::istringstream data(layout);

  data >> this->spriteName;
  data >> this->gridCols;
  data >> this->gridRows;

  int size = this->gridCols * this->gridRows;

  for (int i = 0; i < size && !data.eof(); i++) {
    std::string s_glyph;
    unsigned short flags;

    data >> s_glyph;
    data >> flags;

    int x = i%this->gridCols;
    int y = i/this->gridCols;

    GameCell* cell = new GameCell(x, y, getGlyphAngle(flags), getGlyphType(s_glyph), flags);
    this->cells.emplace(std::to_string(x) + "," + std::to_string(y), cell);
  }
}

Level::~Level() {
  this->cells.clear();
}

std::map<std::string, GameCell*>& Level::getGameCells() {
  return (cells);
}

void Level::rotateTile(int x, int y) {
  if (x >= 0 && x < this->gridCols && y >= 0 && y < this->gridRows) {
    GameCell* cell = this->cells[std::to_string(x) + "," + std::to_string(y)];
    if (cell != nullptr) cell->rotate();
  }
}

bool Level::update(const float fElaspedTime) {
  for (auto& x: this->cells) x.second->update(fElaspedTime);
  return (true);
}

//todo work needed here.
bool Level::isComplete() {
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

bool InfinityGameLogic::levelComplete() {
  return ((this->level != nullptr) ? this->level->isComplete() : false);
}

} /* namespace smlnd */
