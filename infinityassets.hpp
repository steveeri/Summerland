//============================================================================
// Name        : infinityimgassets.h
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
    SPRITE = 0, AUDIO = 1, LEVEL = 2
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
  const std::string pack_name = "TBA";

public:
  InfinityAssets() {
    //SMLND_DBG_LOG("Inside InfinityAssets construct");
    loadAssets();
  }
  ;
  ~InfinityAssets();
  InfinityAssets(const InfinityAssets&) = delete;
  InfinityAssets& operator=(const InfinityAssets&) = delete;

  AssetDtls* getSprite(const std::string name) {
    std::string ts = name + ".sprite";
    return (m_assets[ts]);
  }

  AssetDtls* getAudio(const std::string name) {
    std::string ts = name + ".sound";
    return (m_assets[ts]);
  }

  AssetDtls* getLevel(const int id) {
    std::string ts = id + ".level";
    return (m_assets[ts]);
  }

private:

  void loadAssets() {

    SMLND_DBG_LOG("Inside loadAssets");
    auto loadSpr = [&](AssetDtls* ad) {
      ad->sprite = new olc::Sprite(ad->filePath);
    };

    auto loadAudio = [&](AssetDtls* ad) {
      //ad->audio = new olc::AudioFile(ad->filePath);
    };

    auto loadLevel = [&](AssetDtls* ad, std::ifstream& data) {
      std::string buffer;
      getline(data, buffer);
      //ad->rawlevelData = buffer.c_str();
      ad->rawlevelData = new std::string(buffer);
      SMLND_DBG_LOG_M("LoadLevel() level data string = ", *ad->rawlevelData);
    };

    auto cleanNameStr = [&](std::string& src) {
      std::string find = "_", replace = " ";
      for(std::string::size_type i = 0; (i = src.find(find, i)) != std::string::npos;) {
        src.replace(i, find.length(), replace);
        i += replace.length();
      }
    };


    SMLND_DBG_LOG("Before m_res_file file read ");
    // Temporary count of the number of levels.

    // first reads asset details list from file.
    std::ifstream data(m_res_file, std::ios::in | std::ios::binary);

    if (data.is_open()) {

      while (!data.eof()) {
        std::string tType;
        data >> tType;

        // Skip lines leading with '#', treated as comments.
        if (tType.find_first_of("#") == 0) {
          getline(data, tType);
          SMLND_DBG_LOG_M("Comment found in resource.dat file = ", tType);
          continue;
        }

        // Check for pack name.
        if (tType == "PACK") {
          std::string pkName;
          data >> pkName;
          cleanNameStr(pkName);
          this->pack_name = pkName;
          SMLND_DBG_LOG_M("Pack name found in resource.dat file = ", pkName);
          continue;
        }

        AssetDtls* ad = new AssetDtls();

        if (tType == "SPRITE")
          ad->type = AssetDtls::Type::SPRITE;
        if (tType == "AUDIO")
          ad->type = AssetDtls::Type::AUDIO;
        if (tType == "LEVEL")
          ad->type = AssetDtls::Type::LEVEL;

        // Next element should be name string reference. Clean it.
        std::string name;
        data >> name;
        cleanNameStr(name);
        ad->name = name;

        switch (ad->type) {
        case ad->Type::SPRITE:
          this->numSprites++;
          data >> ad->filePath;
          SMLND_DBG_LOG_M("Inside data loop reading sprite data, ad->filePath = ", ad->filePath);
          data >> ad->asset_w;
          data >> ad->asset_h;
          data >> ad->asset_cell_w;
          data >> ad->asset_cell_h;
          data >> ad->asset_cell_x_cnt;
          data >> ad->asset_cell_y_cnt;
          loadSpr(ad);
          m_assets[ad->name + ".sprite"] = ad;
          break;

        case ad->Type::AUDIO:
          this->numAudio++;
          data >> ad->filePath;
          SMLND_DBG_LOG_M("Inside data loop adding AUDIO, ad->filePath = ", ad->filePath);
          loadAudio(ad);
          m_assets.emplace(ad->name + ".audio", ad);
          break;

        case ad->Type::LEVEL:
          this->numLevels++;
          ad->id = this->numLevels;
          SMLND_DBG_LOG_M("Inside data loop adding LEVEL, ad->name = ", ad->name);
          loadLevel(ad, data);
          m_assets.emplace(ad->id + ".level", ad);
          break;
        }
      }
    } else {
      SMLND_DBG_LOG("File not opened");
    }

  }

};
// end class InfinityImgAssets

}// end namespace.
