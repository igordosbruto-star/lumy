#pragma once

#include <SFML/Graphics/Texture.hpp>

#include <unordered_map>
#include <string>
#include <filesystem>

// Manages loading and caching of textures.
class TextureManager {
public:
    // Returns a reference to the texture located at the given path.
    // Loads the texture from disk if it isn't already cached.
    const sf::Texture& acquire(const std::filesystem::path& path);

    // Clears all cached textures.
    void clear();

private:
    std::unordered_map<std::string, sf::Texture> textures_;
};

