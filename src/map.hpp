#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

#include "texture_manager.hpp"

class Map {
public:
    explicit Map(TextureManager& textures);

    // Loads a TMX map from the given path. Returns true on success.
    bool load(const std::string& path);

    // Draws all loaded layers to the given render target.
    void draw(sf::RenderTarget& target) const;

    // Draws a single layer at the given index.
    void drawLayer(std::size_t index, sf::RenderTarget& target) const;

    // Draws layers in the range [first, last). If last exceeds the layer count,
    // it is clamped to the end.
    void drawRange(std::size_t first, std::size_t last, sf::RenderTarget& target) const;

    std::size_t getLayerCount() const { return layers_.size(); }
    const std::string& getLayerName(std::size_t index) const { return layers_[index].name; }

    std::uint32_t getTileID(std::size_t layer, unsigned x, unsigned y) const;
    void setTileID(std::size_t layer, unsigned x, unsigned y, std::uint32_t id);

private:
    struct TileLayer {
        const sf::Texture* texture{};
        std::vector<std::uint32_t> ids;
        sf::VertexArray vertices;
        std::string name;
    };

    struct TilesetInfo {
        int firstGid{};
        sf::Vector2u tileSize;
        unsigned columns{};
        const sf::Texture* texture{};
    };

    TextureManager& textures_;
    std::unordered_map<int, const sf::Texture*> tilesetTextures_;
    std::vector<TileLayer> layers_;
    std::vector<TilesetInfo> tilesets_;
    unsigned mapWidth_{};
    unsigned mapHeight_{};
    sf::Vector2u tileSize_{};
};

