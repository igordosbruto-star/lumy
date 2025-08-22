// src/main.cpp
#include <SFML/Graphics.hpp>
#include <sol/sol.hpp>
#include <tmxlite/Map.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include "scene.hpp"

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
    auto window = std::make_unique<sf::RenderWindow>(
        sf::VideoMode(sf::Vector2u{W, H}), "Lumy — hello-town");

    // Limitador manual de FPS
    const sf::Time targetFrameTime = sf::seconds(1.f / 60.f);
    sf::Clock frameClock;
    std::size_t fpsFrameCount = 0;
    sf::Time fpsAccumulated = sf::Time::Zero;
    const float maxDeltaTime = 1.f / 30.f;

    // Um quadradinho para animar (placeholder do “herói”)
    auto scene = std::make_unique<Scene>(sf::Vector2f{W * 0.5f, H * 0.5f});

    while (true) {
        if (!window->isOpen()) {
            break;
        }

        sf::Time elapsed = frameClock.restart();
        float deltaTime = elapsed.asSeconds();
        if (deltaTime > maxDeltaTime) {
            std::cout << "[Frame drop] deltaTime: " << deltaTime << "s\n";
            deltaTime = maxDeltaTime;
        }

        // Média de FPS a cada ~1 segundo
        fpsFrameCount++;
        fpsAccumulated += elapsed;
        if (fpsAccumulated >= sf::seconds(1.f)) {
            float avgFps = static_cast<float>(fpsFrameCount) / fpsAccumulated.asSeconds();
            std::cout << "FPS médio: " << avgFps << "\n";
            fpsFrameCount = 0;
            fpsAccumulated = sf::Time::Zero;
        }

        sf::Event event;
        while (window->pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window->close();
                    break;

                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Key::Escape) {
                        window->close();
                    }
                    break;

                default:
                    break;
            }
            scene->handleEvent(event);
        }

        scene->update(deltaTime);

        // 1. Limpar a tela
        window->clear(sf::Color::Black);
        // 2. Desenhar objetos
        scene->draw(*window);
        // 3. Exibir o frame
        window->display();

        // Completar o tempo de frame restante
        sf::Time workTime = frameClock.getElapsedTime();
        if (workTime < targetFrameTime) {
            sf::sleep(targetFrameTime - workTime);
        }
    }

    scene.reset();
    window.reset();

    std::cout << "Lumy: bye!\n";
    return 0;
}
