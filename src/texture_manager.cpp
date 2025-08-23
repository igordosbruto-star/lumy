#include "texture_manager.hpp"

#include <stdexcept>

const sf::Texture& TextureManager::acquire(const std::filesystem::path& path) {
    auto normalized = std::filesystem::weakly_canonical(path);
    std::string key = normalized.generic_string();

    auto it = textures_.find(key);
    if (it != textures_.end()) {
        return it->second;
    }

    sf::Texture texture;
    if (!texture.loadFromFile(key)) {
        throw std::runtime_error("Failed to load texture: " + key);
    }

    auto [insertIt, inserted] = textures_.emplace(std::move(key), std::move(texture));
    return insertIt->second;
}

void TextureManager::clear() {
    textures_.clear();
}

