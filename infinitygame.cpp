#include <cstdlib>
#include <string>

#include <unistd.h>
#include <stdio.h>
#include <limits.h>

#include "olcPixelGameEngine.h"
#include "smlnd_log.hpp"
#include "infinityassets.hpp"
#include "InfinityGameLogic.hpp"

using namespace smlnd;
using namespace olc;

class InfinityGame: public olc::PixelGameEngine {

private:
  InfinityAssets* gameAssets = nullptr;
  AssetDtls* curSprite = nullptr;
  Level* curLevel = nullptr;
  InfinityGameLogic* gameLogic = nullptr;
  InfinityRpt statusRpt;

  int cells_x = 0;
  int cells_y = 0;
  int cell_w = 0;
  int cell_h = 0;
  int cell_ss_w = 0;
  int cell_ss_h = 0;
  int cell_ss_x = 0;
  int cell_ss_y = 0;
  unsigned int game_offset_w = 0;
  unsigned int game_offset_h = 0;

public:
  InfinityGame(InfinityAssets* infAssets) {
    SMLND_DBG_LOG("Inside InfinityGame constructor");
    gameAssets = infAssets;
    this->statusRpt = loadGameLevel(1);
    this->SetPixelMode(Pixel::Mode::ALPHA);
    sAppName = "Infinity";
  }

  InfinityRpt loadGameLevel(const int id) {
    SMLND_DBG_LOG_M("loadGameLevel: requested game level to load (id) = ", id);

    InfinityRpt report;
    if (gameAssets->getLevel(id) == nullptr) {
      SMLND_DBG_LOG_M("loadGameLevel error: no level available for (id) = ", id);
      report.type = InfinityRpt::FATAL;
      report.id = std::to_string(id);
      report.msg = "loadGameLevel error: no level available for (id)";
      return (report);
    }

    if (gameLogic == nullptr) {
      gameLogic = new InfinityGameLogic(gameAssets->getLevel(id)->name,
          gameAssets->getLevel(id)->rawlevelData->c_str());
    } else {
      gameLogic->loadNewLevel(id, gameAssets->getLevel(id)->name, gameAssets->getLevel(id)->rawlevelData->c_str());
    }

    curLevel = gameLogic->level;
    curSprite = gameAssets->getSprite(curLevel->spriteName);

    // If selected sprite not available then load 'default'
    if (curSprite == nullptr) {
      curSprite = gameAssets->getSprite("default");
      SMLND_DBG_LOG_M("Load level error: no sprite for (id). Loading default = ", id);
      report.type = InfinityRpt::Type::ERROR;
      report.id = std::to_string(id);
      report.msg = "Load level error: no sprite for (id). Loading default";

      if (curSprite == nullptr) {
        report.type = InfinityRpt::Type::FATAL;
        return (report);
      }
    }

    cells_x = curLevel->gridCols;
    cells_y = curLevel->gridRows;
    cell_w = curSprite->asset_cell_w;
    cell_h = curSprite->asset_cell_h;
    cell_ss_w = curSprite->asset_w;
    cell_ss_h = curSprite->asset_h;
    cell_ss_x = curSprite->asset_cell_x_cnt;
    cell_ss_y = curSprite->asset_cell_y_cnt;

//    SMLND_DBG_LOG_M("cells_x = ", cells_x);
//    SMLND_DBG_LOG_M("cells_y = ", cells_y);
//    SMLND_DBG_LOG_M("cell_w = ", cell_w);
//    SMLND_DBG_LOG_M("cell_h = ", cell_h);
//    SMLND_DBG_LOG_M("cell_ss_w = ", cell_ss_w);
//    SMLND_DBG_LOG_M("cell_ss_h = ", cell_ss_h);
//    SMLND_DBG_LOG_M("cell_ss_x = ", cell_ss_x);
//    SMLND_DBG_LOG_M("cell_ss_y = ", cell_ss_y);

    return (report);
  }

  ~InfinityGame() {
    free(gameAssets);
  }

  // Called once at the start, so create things here
  bool OnUserCreate() override {
    return (true);
  }

  // called once per frame
  bool OnUserUpdate(float fElapsedTime) override {

    // Calculate the offset of the board to place game in the centre of the window.
    this->game_offset_w = (this->ScreenWidth() - (curLevel->gridCols * cell_w)) / 2;
    this->game_offset_h = (this->ScreenHeight() - (curLevel->gridRows * cell_h)) / 2;

    return (userUpdate(fElapsedTime) && userDraw(fElapsedTime));
  }

  // called by OnUserUpdate - once per frame
  bool userUpdate(float fElapsedTime) {

    this->gameLogic->update(fElapsedTime);

    // Use integer division to nicely get cursor position in node space
    if (GetMouse(0).bPressed) {
      int selectedNodeX = (this->GetMouseX() - game_offset_w) / cell_w;
      int selectedNodeY = (this->GetMouseY() - game_offset_h) / cell_h;

      // Update / rotate selected cell.
      this->curLevel->rotateTile(selectedNodeX, selectedNodeY);
    }

    // Check if user requests to go to next level.
    if (GetKey(Key::N).bPressed) {
      this->statusRpt = this->loadGameLevel(this->curLevel->id + 1);
    }

    // Check if user requests to go to previous level.
    if (GetKey(Key::P).bPressed) {
      if (this->curLevel->id <= 1) {
        this->statusRpt.type = InfinityRpt::Type::MSG;
        this->statusRpt.id = "MSG";
        this->statusRpt.msg = "Message: no previous level to load.";
      } else {
        this->statusRpt = this->loadGameLevel(this->curLevel->id - 1);
      }
    }

    // Check if user requests to clear errors and other messages.
    if (GetKey(Key::C).bPressed) {
      this->statusRpt = InfinityRpt();
    }

    // Check if user wants to quit game..
    if (GetKey(Key::Q).bPressed) {
      return (false);
    }

    return (true);
  }

  // called by OnUserUpdate - once per frame
  bool userDraw(float fElapsedTime) {

    // Clear screen and render title of the level.
    this->Clear(BLACK);
    this->DrawString(10, 10, "Level: " + gameLogic->level->name, YELLOW, 2);
    this->DrawString(10, 33, "Game pack (" + gameAssets->pack_name + ")", CYAN, 1);
    this->DrawString(10, this->ScreenHeight() - 15, "[Keys: 'N'ext | 'P'revious | 'C'lear | 'Q'uit ]", GREEN, 1);

    if (this->statusRpt.type != InfinityRpt::Type::OK) {
      this->DrawString(10, this->ScreenHeight() - 35, this->statusRpt.msg, RED, 1);
    }

    std::map<std::string, GameCell*> cells = curLevel->getGameCells();
    for (auto& obj : curLevel->getGameCells()) {

      // Don't process if a blank cell.
      if (obj.second->glyph == smlnd::BLNK) continue;

      int glyphTypeIndex = static_cast<int>(obj.second->glyph);
      float cellRotation = obj.second->getCellRotation();
      int cellRotIndex = ((int) (cellRotation) % 360); // I'm ignoring any small decimals close to quadrants e.g. 360.5, 299.5.
      unsigned int xPos = (obj.second->x * cell_w) + game_offset_w;
      unsigned int yPos = (obj.second->y * cell_h) + game_offset_h;
      int glyphRotnIndex = 0;

      if (cellRotIndex == 0 || cellRotIndex == 90 || cellRotIndex == 180 || cellRotIndex == 270) {

        // Paint available set images variants from the sprite sheet without alteration.
        glyphRotnIndex = (int) (cellRotIndex) / 90;
        this->DrawPartialSprite(xPos, yPos, curSprite->sprite, glyphRotnIndex * cell_w, glyphTypeIndex * cell_h, cell_w,
            cell_h);

      } else {

        // todo Paint rotated cell relative to the centre of the image.
        float origAngleIndex = 0;
        if (cellRotIndex > 270) {
          origAngleIndex = 270;
        } else if (cellRotIndex > 180) {
          origAngleIndex = 180;
        } else if (cellRotIndex > 90) {
          origAngleIndex = 90;
        }

        glyphRotnIndex = (int) (origAngleIndex) / 90;         // the closest partial image offset to rotate.
        float deltaAngle = (int) (cellRotation) % 360 + (cellRotation - (int) (cellRotation)) - origAngleIndex;
        // the rotation angle relative to the given partial image.

//        SMLND_DBG_LOG_M("Originating Angle = ", origAngleIndex);
//        SMLND_DBG_LOG_M("Delta angle = ", deltaAngle);
//        SMLND_DBG_LOG_M("Rotation img Index = ", glyphRotnIndex);
//        SMLND_DBG_LOG_M("Glyph type Index = ", glyphTypeIndex);

        this->DrawRotatedPartialSprite(xPos + cell_w / 2, yPos + cell_h / 2, curSprite->sprite, glyphRotnIndex * cell_w,
            glyphTypeIndex * cell_h, cell_w, cell_h, deltaAngle, false);
      }
    }
    return (true);
  }

};


/**
 * Main function.
 */
int main(int argc, char **argv) {

  SMLND_DBG_LOG("Inside main before InfinityAssets created");

  printf("argc = %d\n", argc);
  for (int i = 0; i < argc; ++i) {
    printf("argv[ %d ] = %s\n", i, argv[i]);
  }

  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    SMLND_DBG_LOG_M("Current working dir:", cwd);
  }

  InfinityAssets* gameAssets = new InfinityAssets();
  InfinityGame gameEngine(gameAssets);

  SMLND_DBG_LOG("Inside main after InfinityAssets created");

  if (gameEngine.Construct(600, 480, 1, 1, 25)) {  // request 25 FPS
    gameEngine.Start();
  }

  return (0);
}

