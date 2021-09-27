//============================================================================
// Name        : infinityimgassets.cpp
// Author      : Steve Richards
// Version     :
// Copyright   : TBA
// Description : class to get game sprite files from storage.
//============================================================================

#include "infinityassets.hpp"
#include "olcPixelGameEngine.h"

namespace smlnd {

InfinityAssets::InfinityAssets() {
  loadAssets();
}

AssetDtls* InfinityAssets::getSprite(const std::string name) {
  return (m_assets[name + ".sprite"]);
}

AssetDtls* InfinityAssets::getAudio(const std::string name) {
  return (m_assets[name + ".sound"]);
}

AssetDtls* InfinityAssets::getLevel(const int id) {
  return (m_assets[id + ".level"]);
}

AssetDtls* InfinityAssets::getSaved(const std::string packName) {
  return (m_assets[packName + ".saved"]);
}

bool InfinityAssets::saveLevel(const AssetDtls* ad, unsigned short levelId) {

  SMLND_DBG_LOG_M("InfinityAssets::saveLevel called for filePath = ", ad->filePath);

  std::string ts = ad->name, find = " ", replace = "_";
  for (std::string::size_type i = 0; (i = ts.find(find, i)) != std::string::npos;) {
    ts.replace(i, find.length(), replace);
    i += replace.length();
  }

  std::ofstream saved(ad->filePath, std::ios::out | std::ios::binary);

  if (saved.is_open()) {
    saved << ts;
    saved << " ";
    saved << levelId;
    saved.close();
  } else {
    return (false);
  }

  return (true);
}


void InfinityAssets::loadAssets() {

  SMLND_DBG_LOG("Inside loadAssets");

  auto cleanNameStr = [&](std::string& src) {
    std::string find = "_", replace = " ";
    for(std::string::size_type i = 0; (i = src.find(find, i)) != std::string::npos;) {
      src.replace(i, find.length(), replace);
      i += replace.length();
    }
  };

  auto loadSpr = [&](AssetDtls* ad) {
    ad->sprite = new olc::Sprite(ad->filePath);
  };

  auto loadAudio = [&](AssetDtls* ad) {
    // todo ad->audio = new olc::AudioFile(ad->filePath);
    };

  auto loadLevel = [&](AssetDtls* ad, std::ifstream& data) {
    std::string buffer;
    getline(data, buffer);
    ad->rawlevelData = new std::string(buffer);
    SMLND_DBG_LOG_M("LoadLevel() raw data = ", *ad->rawlevelData);
  };

  auto loadSaved = [&](AssetDtls* ad, std::string& filePath) {

    std::ifstream saved(filePath, std::ios::in | std::ios::binary);

    ad->filePath = filePath;

    if (saved.is_open()) {
      std::string ts;
      saved >> ts;
      cleanNameStr(ts);
      ad->name = ts;
      saved >> ad->id;
    }
    SMLND_DBG_LOG_M("LoadSaved() level for game pack = ", ad->name);
    saved.close();
  };


  SMLND_DBG_LOG("Before m_res_file file read ");
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

      if (tType == "SPRITE") ad->type = AssetDtls::Type::SPRITE;
      if (tType == "AUDIO") ad->type = AssetDtls::Type::AUDIO;
      if (tType == "LEVEL") ad->type = AssetDtls::Type::LEVEL;
      if (tType == "SAVED") ad->type = AssetDtls::Type::SAVED;

      // Next element should be name string reference. Clean it.
      std::string name;
      data >> name;
      cleanNameStr(name);
      ad->name = name;

      switch (ad->type) {
      case ad->Type::SPRITE:
        this->numSprites++;
        data >> ad->filePath;
        SMLND_DBG_LOG_M("Loading sprite resource from filePath = ", ad->filePath);
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
        SMLND_DBG_LOG_M("Loading audio resource from filePath = ", ad->filePath);
        loadAudio(ad);
        m_assets.emplace(ad->name + ".audio", ad);
        break;

      case ad->Type::LEVEL:
        this->numLevels++;
        ad->id = this->numLevels;
        SMLND_DBG_LOG_M("Loading level data for name = ", ad->name);
        loadLevel(ad, data);
        m_assets.emplace(ad->id + ".level", ad);
        break;

      case ad->Type::SAVED:
        SMLND_DBG_LOG_M("Loading saved level data from = ", ad->name);
        std::string filePath;
        data >> filePath;
        loadSaved(ad, filePath);
        m_assets.emplace(ad->name + ".saved", ad);
        break;
      }
    }

    data.close();

  } else {
    SMLND_DBG_LOG("File not opened");
  }
}

} // end namespace.
