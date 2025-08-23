// src/main.cpp
#include <SFML/Graphics.hpp>
#include <sol/sol.hpp>
#include <tmxlite/Map.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include "boot_scene.hpp"
#include "scene_stack.hpp"

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

    // Pilha de cenas: inicia em BootScene
    SceneStack stack;
    auto bootScene = std::make_unique<BootScene>(stack);
    stack.pushScene(std::move(bootScene));

    while (window->isOpen()) {
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

        while (auto event = window->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window->close();
            }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    window->close();
                }
            }
            if (auto* current = stack.current()) {
                current->handleEvent(*event);
            }
        }

        stack.applyPending();

        if (!window->isOpen()) {
            break;
        }

        if (auto* current = stack.current()) {
            current->update(deltaTime);
        }

        // 1. Limpar a tela
        window->clear(sf::Color::Black);
        // 2. Desenhar objetos
        if (auto* current = stack.current()) {
            current->draw(*window);
        }
        // 3. Exibir o frame
        window->display();

        // Completar o tempo de frame restante
        sf::Time workTime = frameClock.getElapsedTime();
        if (workTime < targetFrameTime) {
            sf::sleep(targetFrameTime - workTime);
        }
    }

    window.reset();

    std::cout << "Lumy: bye!\n";
    return 0;
}
