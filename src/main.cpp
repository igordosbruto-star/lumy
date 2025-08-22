// src/main.cpp
#include <SFML/Graphics.hpp>
#include <sol/sol.hpp>
#include <tmxlite/Map.hpp>
#include <filesystem>
#include <iostream>

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

                const auto tileCount = map.getTileCount();
                std::cout << "Dimensões do mapa: " << tileCount.x << " x " << tileCount.y << " tiles\n";

                const auto& layers = map.getLayers();
                std::cout << "Camadas (" << layers.size() << "):\n";
                for (const auto& layer : layers) {
                    std::cout << " - " << layer->getName() << "\n";
                }
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

    // Clock para medir tempo entre frames
    sf::Clock frameClock;

    // Um quadradinho para animar (placeholder do “herói”)
    sf::RectangleShape hero(sf::Vector2f{64.f, 64.f});
    hero.setFillColor(sf::Color::White);
    hero.setOrigin(sf::Vector2f{32.f, 32.f});                    // Vector2f
    hero.setPosition(sf::Vector2f{W * 0.5f, H * 0.5f});          // Vector2f

    const float moveSpeed = 200.f;

    while (window.isOpen()) {
        sf::Time dt = frameClock.restart();
        float moveStep = moveSpeed * dt.asSeconds();

        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;

                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Key::Escape) {
                        window.close();
                    }
                    break;

                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        hero.setPosition(sf::Vector2f{static_cast<float>(event.mouseButton.x),
                                                      static_cast<float>(event.mouseButton.y)});
                    }
                    break;

                default:
                    break;
            }
        }

        sf::Vector2f pos = hero.getPosition();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) pos.y -= moveStep;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) pos.y += moveStep;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) pos.x -= moveStep;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) pos.x += moveStep;
        hero.setPosition(pos);

        window.clear(sf::Color::Black);
        window.draw(hero);
        window.display();
    }

    std::cout << "Lumy: bye!\n";
    return 0;
}
