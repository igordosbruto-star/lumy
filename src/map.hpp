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

    std::uint32_t getTileID(std::size_t layer, unsigned x, unsigned y) const;
    void setTileID(std::size_t layer, unsigned x, unsigned y, std::uint32_t id);

private:
    struct TileLayer {
        const sf::Texture* texture{};
        std::vector<std::uint32_t> ids;
        sf::VertexArray vertices;
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

