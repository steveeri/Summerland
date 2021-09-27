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

  bool showSplash = true;
  float timeSlice = 0.0f;

  std::pair<int, int> leftButton[3] = { };
  std::pair<int, int> rightButton[3] = { };

  int cells_x = 0;
  int cells_y = 0;
  int cell_w = 0;
  int cell_h = 0;
  //  int cell_ss_w = 0;
  //  int cell_ss_h = 0;
  //  int cell_ss_x = 0;
  //  int cell_ss_y = 0;
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

    return (report);
  }

  ~InfinityGame() {
    free(gameAssets);
  }

  // Draws a rotated area of given sprite based on (x,y) coordinates to be the centre painted area.
  // Rotation is integer based degrees from original.
  // Selected original sprite area is based on area (ox,oy) to (ox+w,oy+h).
  // Image area should/needs to be divisible by 2 nicely.  e.g.  32px -> 16px, 31 !-> 15.5???
  void DrawRotatedPartialSprite(int32_t x, int32_t y, Sprite *sprite, int32_t ox, int32_t oy, int32_t w, int32_t h,
      int32_t ang_deg, bool nil_cnrs) {
    if (sprite == nullptr) return;

    int32_t half_w = w / 2, half_h = h / 2;

    // convert angle deg to radians = angle * (PI/180).
    float angRad = ang_deg * (3.14159265359 / 180);

    for (int i = -half_w; i < half_w; i++) {
      for (int j = -half_h; j < half_h; j++) {
        Pixel pix = sprite->GetPixel((i + half_w) + ox, (j + half_h) + oy);
        float rotdx = (i * cosf(angRad)) - (j * sinf(angRad));
        float rotdy = (i * sinf(angRad)) + (j * cosf(angRad));

        // Don't print the corners of image if asked not to.
        if (nil_cnrs) {
          if (sqrt((i * i) + (j * j)) <= half_w) Draw(x + rotdx, y + rotdy, pix);
        } else {
          Draw(x + rotdx, y + rotdy, pix);
        }
      }
    }
  }


  // Called once at the start, so create things here
  bool OnUserCreate() override {

    // Check to see if previous saved levels.
    if (gameAssets->getSaved(gameAssets->pack_name) != nullptr) {
      gameLogic->setLevelComplete(gameAssets->getSaved(gameAssets->pack_name)->id);
    }

    leftButton[0] = std::pair<int, int>(35, this->ScreenHeight() / 2 - 20);
    leftButton[1] = std::pair<int, int>(15, this->ScreenHeight() / 2);
    leftButton[2] = std::pair<int, int>(35, this->ScreenHeight() / 2 + 20);

    rightButton[0] = std::pair<int, int>(this->ScreenWidth() - 35, this->ScreenHeight() / 2 - 20);
    rightButton[1] = std::pair<int, int>(this->ScreenWidth() - 15, this->ScreenHeight() / 2);
    rightButton[2] = std::pair<int, int>(this->ScreenWidth() - 35, this->ScreenHeight() / 2 + 20);

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

    if (this->showSplash) return (true);

    this->gameLogic->update(fElapsedTime);
    this->gameLogic->isLevelComplete();

    bool goPrev = false, goNext = false;

    // Use integer division to nicely get cursor position in node space
    if (GetMouse(0).bPressed) {
      int mouseX = this->GetMouseX();
      int mouseY = this->GetMouseY();
      int selectedNodeX = (mouseX - game_offset_w) / cell_w;
      int selectedNodeY = (mouseY - game_offset_h) / cell_h;

      // Clear previous errors and warnings.
      this->statusRpt.type = InfinityRpt::Type::OK;
      this->statusRpt.id = "OK";
      this->statusRpt.msg = "";

      // Update / rotate selected cell.. if a valid tile that is.
      this->curLevel->rotateTile(selectedNodeX, selectedNodeY);

      // Check if previous or next buttons were pressed.
      // Just use a circle range from the centre point.
      int tx, ty, diffX, diffY;

      if (this->curLevel->id > 1) {
        tx = leftButton[1].first;
        ty = leftButton[1].second;
        diffX = tx - mouseX;
        diffY = ty - mouseY;
        goPrev = sqrt(diffX * diffX + diffY * diffY) < 38;
      }

      if (this->gameLogic->levelCleared() >= curLevel->id) {
        tx = rightButton[1].first;
        ty = rightButton[1].second;
        diffX = tx - mouseX;
        diffY = ty - mouseY;
        goNext = sqrt(diffX * diffX + diffY * diffY) < 38;
      }
    }

    // Check if user requests to go to next level.
    //if (this->gameLogic->levelCleared() >= curLevel->id && (GetKey(Key::N).bPressed || goNext)) {
    if (GetKey(Key::N).bPressed || goNext) {   /// allow to proceed for now using N key.
      this->statusRpt = this->loadGameLevel(this->curLevel->id + 1);
    }

    // Check if user requests to go to previous level.
    if (GetKey(Key::P).bPressed || goPrev) {
      if (this->curLevel->id <= 1) {
        this->statusRpt.type = InfinityRpt::Type::MSG;
        this->statusRpt.id = "MSG";
        this->statusRpt.msg = "Message: no previous level to load.";
      } else {
        this->statusRpt = this->loadGameLevel(this->curLevel->id - 1);
      }
    }

    // Check if user requests to reload the current level.
    if (GetKey(Key::R).bPressed) {   /// allow to proceed for now using N key.
      this->statusRpt = this->loadGameLevel(this->curLevel->id);
      this->statusRpt.type = InfinityRpt::Type::MSG;
      this->statusRpt.id = "MSG";
      this->statusRpt.msg = "Message: current level reloaded.";
    }

    // Check if user requests to clear errors and other messages.
    if (GetKey(Key::C).bPressed) {
      this->statusRpt = InfinityRpt();
    }

    // Check if user wants to jump ahead to finished level.
    if (GetKey(Key::J).bPressed) {
      if (gameLogic->getLevelComplete() >= curLevel->id) {
        this->statusRpt = this->loadGameLevel(gameLogic->getLevelComplete() + 1);
        this->statusRpt.type = InfinityRpt::Type::MSG;
        this->statusRpt.id = "MSG";
        this->statusRpt.msg = "Message: jumped to level (" + std::to_string(curLevel->id) + ")";
      }
    }

    // Check if user wants to jump ahead to finished level.
    if (GetKey(Key::S).bPressed) {

      SMLND_DBG_LOG_M("User request to save highest current level = ", gameLogic->getLevelComplete());

      // Check to see if previous saved levels.
      if (gameAssets->getSaved(gameAssets->pack_name) != nullptr) {
        if (gameLogic->getLevelComplete() > gameAssets->getSaved(gameAssets->pack_name)->id) {
          this->statusRpt.type = InfinityRpt::Type::MSG;
          this->statusRpt.id = "MSG";
          this->statusRpt.msg = "Message: Game level saved(level=" + std::to_string(gameLogic->getLevelComplete())
              + ")";
          if (!this->gameAssets->saveLevel(gameAssets->getSaved(gameAssets->pack_name),
              gameLogic->getLevelComplete())) {
            this->statusRpt.type = InfinityRpt::Type::ERROR;
            this->statusRpt.id = "Error";
            this->statusRpt.msg = "Error: Unable to save level (" + std::to_string(gameLogic->getLevelComplete()) + ")";
          }
        }
      }
    }

    // Check if user wants to quit game..
    if (GetKey(Key::Q).bPressed) {
      return (false);
    }

    return (true);
  }

  // called by OnUserUpdate - once per frame
  bool userDraw(float fElapsedTime) {

    if (this->showSplash) {

      this->timeSlice += fElapsedTime;

      if (this->timeSlice < 2.5f) {
        FillRect((ScreenWidth() / 2) - 130, (ScreenHeight() / 2) - 90, 300, 140, MAGENTA);
        DrawString((ScreenWidth() / 2) - 120, (ScreenHeight() / 2) - 70, "LooP-e", BLACK, 4);
        DrawString((ScreenWidth() / 2) - 110, (ScreenHeight() / 2) - 30, "\"Tie da\"", BLUE, 3);
        DrawString((ScreenWidth() / 2) - 20, (ScreenHeight() / 2), "Knots\"", BLUE, 3);
        return (true);
      } else {
        this->showSplash = false;
      }
    }


    // Clear screen and render title of the level.
    this->Clear(BLACK);
    this->DrawString(10, 10, "Level(" + std::to_string(curLevel->id) + "): " + gameLogic->level->name, YELLOW, 2);
    this->DrawString(10, 33, "Game pack (" + gameAssets->pack_name + ")", CYAN, 1);
    this->DrawString(10, this->ScreenHeight() - 17,
        "[Keys: 'N'ext | 'P'revious | 'R'eload | 'C'lear | 'J'ump | 'S'ave | 'Q'uit ]", GREEN, 1);

    if (gameLogic->levelCleared() > curLevel->id) {
      this->DrawString(ScreenWidth() - 350, 10,
        "Completed Levels(" + std::to_string(gameLogic->levelCleared()) + "). 'J' to jump forward", GREEN, 1);
    } else {
      this->DrawString(ScreenWidth() - 350, 10, "Completed Levels(" + std::to_string(gameLogic->levelCleared()) + ")",
          GREEN, 1);
    }

    if (this->statusRpt.type != InfinityRpt::Type::OK) {
      this->DrawString(10, this->ScreenHeight() - 35, this->statusRpt.msg, RED, 1);
    }

    // Check to see if the current game is completed.
    if (this->gameLogic->isLevelComplete()) {
      this->DrawString(50, 50, "Nicely done! Press 'N' or Right Button for next level", RED, 2);
    }

    if (curLevel->id > 1) {
      this->FillTriangle(leftButton[0].first, leftButton[0].second, leftButton[1].first, leftButton[1].second,
          leftButton[2].first, leftButton[2].second, WHITE);
    }

    if (this->gameLogic->levelCleared() >= curLevel->id) {
      this->FillTriangle(rightButton[0].first, rightButton[0].second, rightButton[1].first, rightButton[1].second,
          rightButton[2].first, rightButton[2].second, WHITE);
    }

    std::map<std::pair<int, int>, GameCell*> cells = curLevel->getGameCells();
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
        glyphRotnIndex = (int) (cellRotIndex) / INF_ANGLEOFFSET;
        this->DrawPartialSprite(xPos, yPos, curSprite->sprite, glyphRotnIndex * cell_w, glyphTypeIndex * cell_h, cell_w,
            cell_h);

      } else {

        // Paint rotated cell relative to the centre of the image.
        float origAngleIndex = 0;
        if (cellRotIndex > 270) {
          origAngleIndex = 270;
        } else if (cellRotIndex > 180) {
          origAngleIndex = 180;
        } else if (cellRotIndex > 90) {
          origAngleIndex = 90;
        }

        // the closest partial image offset to rotate.
        glyphRotnIndex = (int) (origAngleIndex) / INF_ANGLEOFFSET;
        // the rotation angle in degrees relative to the nearest/leaving partial image.
        float deltaAngle = (int) (cellRotation) % 360 + (cellRotation - (int) (cellRotation)) - origAngleIndex;

        this->DrawRotatedPartialSprite(xPos + cell_w / 2, yPos + cell_h / 2, curSprite->sprite, glyphRotnIndex * cell_w,
            glyphTypeIndex * cell_h, cell_w, cell_h, deltaAngle, false);
      }
    } // end for loop.
    
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

  if (gameEngine.Construct(1280, 890, 1, 1)) {  // request 25 FPS
    gameEngine.Start();
  }

  return (0);
}

