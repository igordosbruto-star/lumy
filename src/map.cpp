#include "map.hpp"

#include <tmxlite/Layer.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/Tileset.hpp>

#include <SFML/Graphics/Vertex.hpp>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <string>

Map::Map(TextureManager &textures) : textures_(textures) {}

bool Map::load(const std::string &path) {
  tmx::Map tmxMap;
  if (!tmxMap.load(path)) {
    std::cerr << "Failed to load TMX: " << path << '\n';
    return false;
  }

  const auto tileCount = tmxMap.getTileCount();
  std::cout << "TMX loaded: " << tileCount.x << "x" << tileCount.y
            << " tiles, layers: " << tmxMap.getLayers().size() << '\n';

  tilesetTextures_.clear();
  layers_.clear();
  tilesets_.clear();
  mapWidth_ = tileCount.x;
  mapHeight_ = tileCount.y;
  tileSize_ = {tmxMap.getTileSize().x, tmxMap.getTileSize().y};

  std::filesystem::path base = std::filesystem::path(path).parent_path();
  std::vector<TilesetInfo> tilesets;

  for (const auto &ts : tmxMap.getTilesets()) {
    auto texPath = base / ts.getImagePath();
    const sf::Texture &tex = textures_.acquire(texPath);
    int first = static_cast<int>(ts.getFirstGID());
    tilesetTextures_.emplace(first, &tex);
    TilesetInfo info;
    info.firstGid = first;
    info.tileSize = {ts.getTileSize().x, ts.getTileSize().y};
    info.columns = ts.getColumnCount();
    info.texture = &tex;
    tilesets.push_back(info);

    // store properties for future use
    for (const auto &prop : ts.getProperties()) {
      (void)prop;
    }
  }

  std::sort(tilesets.begin(), tilesets.end(),
            [](const TilesetInfo &a, const TilesetInfo &b) {
              return a.firstGid < b.firstGid;
            });

  tilesets_ = tilesets;

  for (const auto &layer : tmxMap.getLayers()) {
    if (layer->getType() != tmx::Layer::Type::Tile)
      continue;

    const auto &tmxLayer = layer->getLayerAs<tmx::TileLayer>();
    std::string layerName = layer->getName();

    for (const auto &prop : layer->getProperties()) {
      (void)prop;
    }

    std::unordered_map<const sf::Texture *, sf::VertexArray> batches;
    const auto &tiles = tmxLayer.getTiles();
    TileLayer baseLayer;
    baseLayer.ids.resize(tiles.size());
    baseLayer.name = layerName;
    const auto tmxTileSize = tmxMap.getTileSize();
    sf::Vector2u tileSizePx{tmxTileSize.x, tmxTileSize.y};

    for (std::size_t i = 0; i < tiles.size(); ++i) {
      const auto &tile = tiles[i];
      baseLayer.ids[i] = tile.ID;
    }

    for (std::size_t i = 0; i < baseLayer.ids.size(); ++i) {
      std::uint32_t id = baseLayer.ids[i];
      if (id == 0)
        continue;

      const TilesetInfo *tsInfo = nullptr;
      for (const auto &info : tilesets) {
        if (id >= static_cast<std::uint32_t>(info.firstGid))
          tsInfo = &info;
        else
          break;
      }
      if (!tsInfo)
        continue;

      sf::VertexArray &va = batches[tsInfo->texture];
      va.setPrimitiveType(sf::PrimitiveType::Triangles);

      std::uint32_t localID = id - tsInfo->firstGid;
      unsigned tu = localID % tsInfo->columns;
      unsigned tv = localID / tsInfo->columns;
      float tx = static_cast<float>(tu * tsInfo->tileSize.x);
      float ty = static_cast<float>(tv * tsInfo->tileSize.y);

      unsigned x = static_cast<unsigned>(i % tileCount.x);
      unsigned y = static_cast<unsigned>(i / tileCount.x);
      sf::Vertex quad[4];
      quad[0].position = {static_cast<float>(x * tileSizePx.x),
                          static_cast<float>(y * tileSizePx.y)};
      quad[1].position = {static_cast<float>((x + 1) * tileSizePx.x),
                          static_cast<float>(y * tileSizePx.y)};
      quad[2].position = {static_cast<float>((x + 1) * tileSizePx.x),
                          static_cast<float>((y + 1) * tileSizePx.y)};
      quad[3].position = {static_cast<float>(x * tileSizePx.x),
                          static_cast<float>((y + 1) * tileSizePx.y)};

      sf::Vector2f uv[4] = {{tx, ty},
                            {tx + tsInfo->tileSize.x, ty},
                            {tx + tsInfo->tileSize.x, ty + tsInfo->tileSize.y},
                            {tx, ty + tsInfo->tileSize.y}};

      std::uint8_t flip = tiles[i].flipFlags;
      if (flip & tmx::TileLayer::FlipFlag::Horizontal) {
        std::swap(uv[0], uv[1]);
        std::swap(uv[3], uv[2]);
      }
      if (flip & tmx::TileLayer::FlipFlag::Vertical) {
        std::swap(uv[0], uv[3]);
        std::swap(uv[1], uv[2]);
      }
      if (flip & tmx::TileLayer::FlipFlag::Diagonal) {
        std::swap(uv[1], uv[3]);
      }

      for (int v = 0; v < 4; ++v) {
        quad[v].texCoords = uv[v];
      }

      va.append(quad[0]);
      va.append(quad[1]);
      va.append(quad[2]);
      va.append(quad[0]);
      va.append(quad[2]);
      va.append(quad[3]);
    }

    if (batches.empty()) {
      layers_.push_back(std::move(baseLayer));
    } else {
      for (auto &[texPtr, vertices] : batches) {
        TileLayer tl;
        tl.texture = texPtr;
        tl.ids = baseLayer.ids;
        tl.vertices = std::move(vertices);
        tl.name = layerName;
        layers_.push_back(std::move(tl));
      }
    }
  }

  return true;
}

std::uint32_t Map::getTileID(std::size_t layer, unsigned x, unsigned y) const {
  if (layer >= layers_.size())
    return 0;
  const auto &ids = layers_[layer].ids;
  std::size_t idx = static_cast<std::size_t>(y) * mapWidth_ + x;
  if (idx >= ids.size())
    return 0;
  return ids[idx];
}

void Map::setTileID(std::size_t layer, unsigned x, unsigned y, std::uint32_t id) {
  if (layer >= layers_.size())
    return;
  TileLayer &tl = layers_[layer];
  std::size_t idx = static_cast<std::size_t>(y) * mapWidth_ + x;
  if (idx >= tl.ids.size())
    return;
  tl.ids[idx] = id;

  const TilesetInfo *tsInfo = nullptr;
  for (const auto &info : tilesets_) {
    if (id >= static_cast<std::uint32_t>(info.firstGid))
      tsInfo = &info;
    else
      break;
  }
  if (!tsInfo)
    return;

  tl.texture = tsInfo->texture;

  std::uint32_t localID = id - tsInfo->firstGid;
  unsigned tu = localID % tsInfo->columns;
  unsigned tv = localID / tsInfo->columns;
  float tx = static_cast<float>(tu * tsInfo->tileSize.x);
  float ty = static_cast<float>(tv * tsInfo->tileSize.y);

  float px = static_cast<float>(x * tileSize_.x);
  float py = static_cast<float>(y * tileSize_.y);

  sf::Vertex *quad = &tl.vertices[idx * 6];
  quad[0].position = {px, py};
  quad[1].position = {px + tileSize_.x, py};
  quad[2].position = {px + tileSize_.x, py + tileSize_.y};
  quad[3].position = {px, py};
  quad[4].position = {px + tileSize_.x, py + tileSize_.y};
  quad[5].position = {px, py + tileSize_.y};

  quad[0].texCoords = {tx, ty};
  quad[1].texCoords = {tx + tsInfo->tileSize.x, ty};
  quad[2].texCoords = {tx + tsInfo->tileSize.x, ty + tsInfo->tileSize.y};
  quad[3].texCoords = {tx, ty};
  quad[4].texCoords = {tx + tsInfo->tileSize.x, ty + tsInfo->tileSize.y};
  quad[5].texCoords = {tx, ty + tsInfo->tileSize.y};
}

void Map::draw(sf::RenderTarget &target) const {
  drawRange(0, layers_.size(), target);
}

void Map::drawLayer(std::size_t index, sf::RenderTarget &target) const {
  drawRange(index, index + 1, target);
}

void Map::drawRange(std::size_t first, std::size_t last,
                     sf::RenderTarget &target) const {
  if (first >= layers_.size())
    return;
  if (last > layers_.size())
    last = layers_.size();

  sf::RenderStates states;
  const sf::Texture *currentTexture = nullptr;

  for (std::size_t i = first; i < last; ++i) {
    const auto &layer = layers_[i];
    if (layer.vertices.getVertexCount() == 0)
      continue;

    if (layer.texture != currentTexture) {
      currentTexture = layer.texture;
      states.texture = currentTexture;
    }

    // Future extension: perform view-based culling here to skip drawing tiles
    // outside the visible area, reducing unnecessary draw calls.
    target.draw(layer.vertices, states);
  }
}
