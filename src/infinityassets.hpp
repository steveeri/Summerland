//============================================================================
// Name        : infinityimgassets.hpp
// Author      : Steve Richards
// Version     :
// Copyright   : TBA
// Description : class to get game sprite files from storage.
//============================================================================

#pragma once

#include "olcPixelGameEngine.h"
#include "smlnd_log.hpp"

#include <map>
#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include <fstream>

namespace smlnd {

class AssetDtls {
public:
  enum Type {
    SPRITE = 0, AUDIO = 1, LEVEL = 2, SAVED = 3
  } type;
  int id;
  std::string name, filePath;
  int asset_w;
  int asset_h;
  int asset_cell_w;
  int asset_cell_h;
  int asset_cell_x_cnt;
  int asset_cell_y_cnt;
  olc::Sprite* sprite;
  olc::AudioFile* audio;
  std::string* rawlevelData;
};

class InfinityAssets final {

private:
  //todo allow command line input of different dat file location.
  const std::string m_res_file = "res/infinity-resources.dat";
  std::map<std::string, AssetDtls*> m_assets;

public:
  int numLevels = 0, numSprites = 0, numAudio = 0;
  std::string pack_name = "TBA";

public:
  InfinityAssets();
  ~InfinityAssets();
  InfinityAssets(const InfinityAssets&) = delete;
  InfinityAssets& operator=(const InfinityAssets&) = delete;
  AssetDtls* getSprite(const std::string name);
  AssetDtls* getAudio(const std::string name);
  AssetDtls* getLevel(const int id);
  AssetDtls* getSaved(const std::string packName);
  bool saveLevel(const AssetDtls* ad, unsigned short levelId);

private:
  void loadAssets();
};

} // end namespace.
