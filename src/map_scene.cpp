#include "map_scene.hpp"
#include "scene_stack.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/Layer.hpp>
#include <tmxlite/Object.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>

MapScene::MapScene(SceneStack& stack, TextureManager& textures, const std::string& tmxPath)
    : sceneStack_(stack), textures_(textures), map_(textures_) {
    map_.load(tmxPath);

    sf::Vector2f startPos{0.f, 0.f};
    tmx::Map tmxMap;
    if (tmxMap.load(tmxPath)) {
        bool found = false;
        for (const auto& layer : tmxMap.getLayers()) {
            if (layer->getType() != tmx::Layer::Type::Object)
                continue;
            const auto& group = layer->getLayerAs<tmx::ObjectGroup>();
            for (const auto& obj : group.getObjects()) {
                const auto& name = obj.getName();
                if (name == "player" || name == "spawn") {
                    const auto pos = obj.getPosition();
                    startPos.x = pos.x;
                    startPos.y = pos.y;
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
        if (!found) {
            for (const auto& prop : tmxMap.getProperties()) {
                if (prop.getName() == "spawn_x") {
                    startPos.x = static_cast<float>(prop.getIntValue());
                } else if (prop.getName() == "spawn_y") {
                    startPos.y = static_cast<float>(prop.getIntValue());
                }
            }
        }
    }

    hero_.setSize(sf::Vector2f{64.f, 64.f});
    hero_.setFillColor(sf::Color::White);
    hero_.setOrigin(sf::Vector2f{32.f, 32.f});
    hero_.setPosition(startPos);
    
    // Inicializar sistemas
    eventSystem_ = std::make_unique<EventSystem>(&sceneStack_, &textures_);
    saveSystem_ = std::make_unique<SaveSystem>();
    
    if (!eventSystem_->initialize()) {
        std::cerr << "[MapScene] Falha ao inicializar EventSystem\n";
    }
    
    if (!saveSystem_->initialize("game")) {
        std::cerr << "[MapScene] Falha ao inicializar SaveSystem\n";
    }
    
    // Tentar carregar fonte para UI
    if (uiFont_.openFromFile("game/font.ttf")) {
        uiText_.emplace(uiFont_);
        uiText_->setCharacterSize(14);
        uiText_->setFillColor(sf::Color::Yellow);
        uiText_->setPosition({10, 10});
    }
    
    setupExampleEvents();
}

void MapScene::handleEvent(const sf::Event& event) {
    // Se há um evento sendo executado, delegar para o sistema de eventos
    if (eventSystem_ && eventSystem_->isEventRunning()) {
        eventSystem_->handleInput(event);
        return;
    }
    
    // Input normal do jogo
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            hero_.setPosition({static_cast<float>(mouse->position.x),
                               static_cast<float>(mouse->position.y)});
        }
    }
    
    // Teclas especiais
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
            checkEventTriggers();
        } else if (key->code == sf::Keyboard::Key::F5) {
            // Quick save
            if (saveSystem_) {
                sf::Vector2f pos = hero_.getPosition();
                saveSystem_->setPlayerPosition(1, pos.x, pos.y, 2);
                saveSystem_->saveGame(1);
                std::cout << "[MapScene] Quick save realizado\n";
            }
        } else if (key->code == sf::Keyboard::Key::F9) {
            // Quick load
            if (saveSystem_ && saveSystem_->saveExists(1)) {
                int mapId, direction;
                float x, y;
                saveSystem_->loadGame(1);
                saveSystem_->getPlayerPosition(mapId, x, y, direction);
                hero_.setPosition({x, y});
                std::cout << "[MapScene] Quick load realizado\n";
            }
        }
        
        // Atalhos AltGr + números (1-9) para saves
        if (key->alt && !key->control && !key->shift) {
            if (key->code >= sf::Keyboard::Key::Num1 && key->code <= sf::Keyboard::Key::Num9) {
                int slotId = static_cast<int>(key->code) - static_cast<int>(sf::Keyboard::Key::Num1) + 1;
                
                if (saveSystem_) {
                    sf::Vector2f pos = hero_.getPosition();
                    saveSystem_->setPlayerPosition(1, pos.x, pos.y, 2);
                    if (saveSystem_->saveGame(slotId)) {
                        std::cout << "[MapScene] Save realizado no slot " << slotId << std::endl;
                    }
                }
            }
        }
        
        // Ctrl + AltGr + números (1-9) para loads
        if (key->alt && key->control && !key->shift) {
            if (key->code >= sf::Keyboard::Key::Num1 && key->code <= sf::Keyboard::Key::Num9) {
                int slotId = static_cast<int>(key->code) - static_cast<int>(sf::Keyboard::Key::Num1) + 1;
                
                if (saveSystem_ && saveSystem_->saveExists(slotId)) {
                    int mapId, direction;
                    float x, y;
                    if (saveSystem_->loadGame(slotId)) {
                        saveSystem_->getPlayerPosition(mapId, x, y, direction);
                        hero_.setPosition({x, y});
                        std::cout << "[MapScene] Load realizado do slot " << slotId << std::endl;
                    }
                } else {
                    std::cout << "[MapScene] Save não encontrado no slot " << slotId << std::endl;
                }
            }
        }
        
        // Shift + AltGr + números (1-9) para deletar saves
        if (key->alt && !key->control && key->shift) {
            if (key->code >= sf::Keyboard::Key::Num1 && key->code <= sf::Keyboard::Key::Num9) {
                int slotId = static_cast<int>(key->code) - static_cast<int>(sf::Keyboard::Key::Num1) + 1;
                
                if (saveSystem_ && saveSystem_->deleteSave(slotId)) {
                    std::cout << "[MapScene] Save deletado do slot " << slotId << std::endl;
                } else {
                    std::cout << "[MapScene] Falha ao deletar save do slot " << slotId << std::endl;
                }
            }
        }
    }
}

void MapScene::update(float deltaTime) {
    // Atualizar sistema de eventos
    if (eventSystem_) {
        eventSystem_->update(deltaTime);
    }
    
    // Se um evento está executando, não processar movimento
    if (eventSystem_ && eventSystem_->isEventRunning()) {
        return;
    }
    
    sf::Vector2f pos = hero_.getPosition();
    sf::Vector2f newPos = pos;
    bool moved = false;
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        newPos.y -= moveSpeed_ * deltaTime;
        moved = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        newPos.y += moveSpeed_ * deltaTime;
        moved = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        newPos.x -= moveSpeed_ * deltaTime;
        moved = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        newPos.x += moveSpeed_ * deltaTime;
        moved = true;
    }
    
    // Aplicar movimento (desabilitar colisão temporariamente para debug)
    if (moved) {
        // Verificar se a nova posição está dentro dos limites do mapa
        const auto& ts = map_.getTileSize();
        const float mapWidth = static_cast<float>(map_.getWidth() * ts.x);
        const float mapHeight = static_cast<float>(map_.getHeight() * ts.y);
        
        // Limitar dentro das bordas do mapa
        newPos.x = std::max(32.0f, std::min(newPos.x, mapWidth - 32.0f));
        newPos.y = std::max(32.0f, std::min(newPos.y, mapHeight - 32.0f));
        
        // Testar colisão apenas se estivermos dentro dos limites
        unsigned tileX = static_cast<unsigned>((newPos.x + 16.0f) / static_cast<float>(ts.x));
        unsigned tileY = static_cast<unsigned>((newPos.y + 16.0f) / static_cast<float>(ts.y));
        
        // Por enquanto, permitir todo movimento para debug
        hero_.setPosition(newPos);
        
        // Log para debug
        if (moved) {
            std::cout << "[Debug] Movimento: (" << newPos.x << ", " << newPos.y << ") Tile: (" << tileX << ", " << tileY << ")" << std::endl;
        }
    }
    
    // Atualizar UI
    if (uiText_.has_value()) {
        sf::Vector2f heroPos = hero_.getPosition();
        std::string uiInfo = "Posição: (" + std::to_string(static_cast<int>(heroPos.x)) + ", " + std::to_string(static_cast<int>(heroPos.y)) + ")";
        uiInfo += "\nControles: WASD=Mover, Enter/Space=Interagir";
        uiInfo += "\nF5=QuickSave, F9=QuickLoad, AltGr+N=Save, Ctrl+AltGr+N=Load, Shift+AltGr+N=Delete";
        
        if (eventSystem_) {
            uiInfo += "\nSwitches: 1=" + std::string(eventSystem_->getSwitch(1) ? "ON" : "OFF");
            uiInfo += ", Variável 1=" + std::to_string(eventSystem_->getVariable(1));
        }
        
        uiText_->setString(uiInfo);
    }
}

void MapScene::draw(sf::RenderWindow& window) const {
    // Draw ground_* layers first
    for (std::size_t i = 0; i < map_.getLayerCount(); ++i) {
        const std::string& name = map_.getLayerName(i);
        if (name.rfind("ground_", 0) == 0) {
            map_.drawLayer(i, window);
        }
    }

    window.draw(hero_);

    // Draw object_* and remaining layers
    for (std::size_t i = 0; i < map_.getLayerCount(); ++i) {
        const std::string& name = map_.getLayerName(i);
        if (name.rfind("ground_", 0) != 0) {
            map_.drawLayer(i, window);
        }
    }
    
    // Desenhar sistema de eventos (textos, imagens)
    if (eventSystem_) {
        eventSystem_->draw(window);
    }
    
    // Desenhar UI
    if (uiText_.has_value()) {
        window.draw(*uiText_);
    }
}

void MapScene::setupExampleEvents() {
    if (!eventSystem_) return;
    
    // Evento 1: Boas-vindas
    GameEvent welcomeEvent(1, "Welcome NPC", 200, 200);
    EventPage welcomePage;
    
    // Comando 1: Mostrar texto
    EventCommandParams textParams;
    textParams.stringParams.push_back("Bem-vindo ao mundo de Lumy!");
    welcomePage.commands.emplace_back(EventCommandType::ShowText, textParams);
    
    // Comando 2: Definir switch
    EventCommandParams switchParams;
    switchParams.intParams = {1, 1}; // Switch 1 = ON
    welcomePage.commands.emplace_back(EventCommandType::SetSwitch, switchParams);
    
    // Comando 3: Incrementar variável
    EventCommandParams varParams;
    varParams.intParams = {1, 100}; // Variável 1 = 100
    welcomePage.commands.emplace_back(EventCommandType::SetVariable, varParams);
    
    // Comando 4: Aguardar
    EventCommandParams waitParams;
    waitParams.intParams = {60}; // 60 frames = 1 segundo
    welcomePage.commands.emplace_back(EventCommandType::Wait, waitParams);
    
    // Comando 5: Segundo texto
    EventCommandParams textParams2;
    textParams2.stringParams.push_back("Use WASD para se mover e Enter/Space para interagir.");
    welcomePage.commands.emplace_back(EventCommandType::ShowText, textParams2);
    
    welcomeEvent.pages.push_back(welcomePage);
    eventSystem_->addEvent(welcomeEvent);
    
    // Evento 2: Demonstração condicional
    GameEvent conditionalEvent(2, "Conditional NPC", 300, 150);
    EventPage conditionalPage;
    
    // Condição: Se switch 1 está ON
    EventCommandParams condParams;
    condParams.intParams = {0, 1, 1}; // Tipo 0 (switch), Switch 1, valor esperado 1 (ON)
    conditionalPage.commands.emplace_back(EventCommandType::ConditionalBranch, condParams);
    
    // Se true: mostrar mensagem especial
    EventCommandParams trueTextParams;
    trueTextParams.stringParams.push_back("Você já falou com o primeiro NPC!");
    conditionalPage.commands.emplace_back(EventCommandType::ShowText, trueTextParams);
    
    // Fim do if
    conditionalPage.commands.emplace_back(EventCommandType::EndConditional);
    
    conditionalEvent.pages.push_back(conditionalPage);
    eventSystem_->addEvent(conditionalEvent);
    
    std::cout << "[MapScene] Eventos de exemplo configurados\n";
}

void MapScene::checkEventTriggers() {
    if (!eventSystem_) return;
    
    sf::Vector2f heroPos = hero_.getPosition();
    
    // Verificar proximidade com eventos
    // Evento 1 - próximo a (200, 200)
    float dist1 = std::sqrt(std::pow(heroPos.x - 200, 2) + std::pow(heroPos.y - 200, 2));
    if (dist1 < 80.0f) { // Dentro de 80 pixels
        eventSystem_->triggerEvent(1);
        return;
    }
    
    // Evento 2 - próximo a (300, 150)
    float dist2 = std::sqrt(std::pow(heroPos.x - 300, 2) + std::pow(heroPos.y - 150, 2));
    if (dist2 < 80.0f) { // Dentro de 80 pixels
        eventSystem_->triggerEvent(2);
        return;
    }
    
    std::cout << "[MapScene] Nenhum evento próximo para disparar\n";
}
