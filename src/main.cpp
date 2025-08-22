// src/main.cpp
#include <SFML/Graphics.hpp>
#include <sol/sol.hpp>
#include <tmxlite/Map.hpp>
#include <filesystem>
#include <iostream>
#include <cmath>

int main() {
    std::cout << "Lumy: hello-town iniciando...\n";

    // Teste rápido do Lua/sol2 (só para validar includes/links)
    try {
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::math);
        lua.script("print('Lumy (Lua/sol2) OK')");
    } catch (const std::exception& e) {
        std::cerr << "[Lua] erro: " << e.what() << "\n";
    }

    // Teste opcional de TMX (não é obrigatório ter o arquivo)
    try {
        const char* tmxPath = "game/first.tmx"; // será copiado se você criar examples/hello-town/first.tmx
        if (std::filesystem::exists(tmxPath)) {
            tmx::Map map;
            if (map.load(tmxPath)) {
                std::cout << "TMX carregado: " << tmxPath << "\n";
            } else {
                std::cout << "Falha ao carregar TMX: " << tmxPath << "\n";
            }
        } else {
            std::cout << "Nenhum TMX encontrado (opcional): " << tmxPath << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[TMX] erro: " << e.what() << "\n";
    }

    // Janela SFML
    const unsigned W = 640, H = 360;
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u{W, H}), "Lumy — hello-town");
    window.setFramerateLimit(60);
    

    // Um quadradinho para animar (placeholder do “herói”)
    sf::RectangleShape hero(sf::Vector2f{64.f, 64.f});
    hero.setFillColor(sf::Color::White);
    hero.setOrigin(sf::Vector2f{32.f, 32.f});                    // Vector2f
    hero.setPosition(sf::Vector2f{W * 0.5f, H * 0.5f});          // Vector2f

    sf::Clock clock;

    while (window.isOpen()) {
        // pollEvent() retorna std::optional<Event>
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                window.close();
            } else if (ev->is<sf::Event::KeyPressed>() &&
                       ev->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) {
                window.close();
            }
        }

        // Movimento bobo só pra ver algo na tela
        float t = clock.getElapsedTime().asSeconds();
        hero.setPosition(sf::Vector2f{W * 0.5f + std::sin(t) * 40.f, H * 0.5f});

        window.clear(sf::Color::Black);
        window.draw(hero);
        window.display();
    }

    std::cout << "Lumy: bye!\n";
    return 0;
}
