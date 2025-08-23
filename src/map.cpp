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

  std::filesystem::path base = std::filesystem::path(path).parent_path();

  struct TilesetInfo {
    int firstGid{};
    sf::Vector2u tileSize;
    unsigned columns{};
    const sf::Texture *texture{};
  };
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

  for (const auto &layer : tmxMap.getLayers()) {
    if (layer->getType() != tmx::Layer::Type::Tile)
      continue;

    const auto &tileLayer = layer->getLayerAs<tmx::TileLayer>();

    for (const auto &prop : layer->getProperties()) {
      (void)prop;
    }

    struct Batch {
      std::vector<std::uint32_t> ids;
      sf::VertexArray vertices;
    };
    std::unordered_map<const sf::Texture *, Batch> batches;
    const auto &tiles = tileLayer.getTiles();
    const auto tmxTileSize = tmxMap.getTileSize();
    sf::Vector2u tileSizePx{tmxTileSize.x, tmxTileSize.y};

    for (std::size_t i = 0; i < tiles.size(); ++i) {
      const auto &tile = tiles[i];
      if (tile.ID == 0)
        continue;

      const TilesetInfo *tsInfo = nullptr;
      for (const auto &info : tilesets) {
        if (tile.ID >= static_cast<std::uint32_t>(info.firstGid))
          tsInfo = &info;
        else
          break;
      }
      if (!tsInfo)
        continue;

      Batch &batch = batches[tsInfo->texture];
      sf::VertexArray &va = batch.vertices;
      va.setPrimitiveType(sf::PrimitiveType::Triangles);

      std::uint32_t localID = tile.ID - tsInfo->firstGid;
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

      std::uint8_t flip = tile.flipFlags;
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

      batch.ids.push_back(tile.ID);
    }

    for (auto &[texPtr, batch] : batches) {
      TileLayer tl;
      tl.texture = texPtr;
      tl.ids = std::move(batch.ids);
      tl.vertices = std::move(batch.vertices);
      layers_.push_back(std::move(tl));
    }
  }

  return true;
}

void Map::draw(sf::RenderTarget &target) const {
  for (const auto &layer : layers_) {
    sf::RenderStates states;
    states.texture = layer.texture;
    target.draw(layer.vertices, states);
  }
}
