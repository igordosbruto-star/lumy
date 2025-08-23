#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <unordered_map>
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

private:
    struct Layer {
        const sf::Texture* texture{};
        sf::VertexArray vertices;
    };

    TextureManager& textures_;
    std::unordered_map<int, const sf::Texture*> tilesetTextures_;
    std::vector<Layer> layers_;
};

