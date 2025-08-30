// src/event_system.cpp
#include "event_system.hpp"
#include "scene_stack.hpp"
#include "texture_manager.hpp"
#include <iostream>
#include <filesystem>

EventSystem::EventSystem(SceneStack* stack, TextureManager* textures)
    : sceneStack(stack), textureManager(textures) {
    // Inicializar Lua
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string);
    
    // Configurar display de texto
    textBackground.setFillColor(sf::Color(0, 0, 0, 180));
    // textDisplay será inicializado quando a fonte for carregada
}

bool EventSystem::initialize() {
    // Tentar carregar fonte
    if (!font.openFromFile("game/font.ttf")) {
        std::cerr << "[EventSystem] Erro: não foi possível carregar game/font.ttf\n";
        return false;
    }
    
    // Inicializar textDisplay com fonte carregada
    textDisplay.emplace(font);
    textDisplay->setFillColor(sf::Color::White);
    textDisplay->setCharacterSize(18);
    return true;
}

void EventSystem::setSwitch(int id, bool value) {
    switches[id] = value;
    std::cout << "[EventSystem] Switch " << id << " = " << (value ? "ON" : "OFF") << "\n";
}

bool EventSystem::getSwitch(int id) const {
    auto it = switches.find(id);
    return it != switches.end() ? it->second : false;
}

void EventSystem::setVariable(int id, int value) {
    variables[id] = value;
    std::cout << "[EventSystem] Variable " << id << " = " << value << "\n";
}

int EventSystem::getVariable(int id) const {
    auto it = variables.find(id);
    return it != variables.end() ? it->second : 0;
}

void EventSystem::addEvent(const GameEvent& event) {
    events.push_back(event);
    std::cout << "[EventSystem] Evento adicionado: " << event.name << " (ID: " << event.id << ")\n";
}

void EventSystem::triggerEvent(int eventId) {
    for (auto& event : events) {
        if (event.id == eventId) {
            std::cout << "[EventSystem] Executando evento: " << event.name << "\n";
            
            // Encontrar página ativa
            for (auto& page : event.pages) {
                if (page.enabled && !page.commands.empty()) {
                    if (evaluateCondition(page.switchId, page.switchValue, page.variableId, page.variableValue)) {
                        isExecuting = true;
                        currentCommands = &page.commands;
                        currentCommandIndex = 0;
                        continueExecution();
                        return;
                    }
                }
            }
            break;
        }
    }
}

void EventSystem::executeCommand(const EventCommand& command) {
    switch (command.type) {
        case EventCommandType::ShowText:
            executeShowText(command.params);
            break;
        case EventCommandType::SetSwitch:
            executeSetSwitch(command.params);
            break;
        case EventCommandType::SetVariable:
            executeSetVariable(command.params);
            break;
        case EventCommandType::ConditionalBranch:
            executeConditionalBranch(command.params);
            break;
        case EventCommandType::Wait:
            executeWait(command.params);
            break;
        case EventCommandType::TransferPlayer:
            executeTransferPlayer(command.params);
            break;
        case EventCommandType::PlayBGM:
            executePlayBGM(command.params);
            break;
        case EventCommandType::PlaySE:
            executePlaySE(command.params);
            break;
        case EventCommandType::ShowPicture:
            executeShowPicture(command.params);
            break;
        case EventCommandType::ErasePicture:
            executeErasePicture(command.params);
            break;
        default:
            std::cout << "[EventSystem] Comando não implementado: " << static_cast<int>(command.type) << "\n";
            continueExecution();
            break;
    }
}

void EventSystem::update(float deltaTime) {
    if (isWaiting) {
        waitTimer -= deltaTime;
        if (waitTimer <= 0.0f) {
            isWaiting = false;
            continueExecution();
        }
    }
    
    if (showingText) {
        textTimer += deltaTime;
    }
}

void EventSystem::draw(sf::RenderWindow& window) {
    if (showingText && textDisplay.has_value()) {
        window.draw(textBackground);
        window.draw(*textDisplay);
    }
    
    // Desenhar imagens
    for (const auto& [id, sprite] : pictures) {
        if (sprite) {
            window.draw(*sprite);
        }
    }
}

void EventSystem::handleInput(const sf::Event& event) {
    if (showingText) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Space || key->code == sf::Keyboard::Key::Enter) {
                showingText = false;
                continueExecution();
            }
        }
    }
}

// Implementação dos comandos específicos

void EventSystem::executeShowText(const EventCommandParams& params) {
    if (!params.stringParams.empty()) {
        currentText = params.stringParams[0];
        showingText = true;
        textTimer = 0.0f;
        
        // Configurar posição e tamanho do texto
        float windowWidth = 640.0f; // Assumindo tamanho padrão
        float windowHeight = 360.0f;
        
        textBackground.setSize({windowWidth - 40, 100});
        textBackground.setPosition({20, windowHeight - 120});
        
        if (textDisplay.has_value()) {
            textDisplay->setString(currentText);
            textDisplay->setPosition({30, windowHeight - 110});
        }
        
        std::cout << "[EventSystem] Exibindo texto: " << currentText << "\n";
    } else {
        continueExecution();
    }
}

void EventSystem::executeSetSwitch(const EventCommandParams& params) {
    if (params.intParams.size() >= 2) {
        int switchId = params.intParams[0];
        bool value = params.intParams[1] != 0;
        setSwitch(switchId, value);
    }
    continueExecution();
}

void EventSystem::executeSetVariable(const EventCommandParams& params) {
    if (params.intParams.size() >= 2) {
        int varId = params.intParams[0];
        int value = params.intParams[1];
        setVariable(varId, value);
    }
    continueExecution();
}

void EventSystem::executeConditionalBranch(const EventCommandParams& params) {
    // Implementação básica de if/else
    // params.intParams[0] = tipo de condição (0 = switch, 1 = variável)
    // params.intParams[1] = ID
    // params.intParams[2] = valor esperado
    
    bool condition = false;
    if (params.intParams.size() >= 3) {
        if (params.intParams[0] == 0) { // Switch
            condition = (getSwitch(params.intParams[1]) == (params.intParams[2] != 0));
        } else if (params.intParams[0] == 1) { // Variable
            condition = (getVariable(params.intParams[1]) == params.intParams[2]);
        }
    }
    
    std::cout << "[EventSystem] Condição avaliada: " << (condition ? "verdadeira" : "falsa") << "\n";
    
    if (!condition) {
        // Pular para o próximo else ou endif
        size_t indent = 0;
        while (currentCommandIndex < currentCommands->size()) {
            currentCommandIndex++;
            if (currentCommandIndex >= currentCommands->size()) break;
            
            const auto& cmd = (*currentCommands)[currentCommandIndex];
            if (cmd.type == EventCommandType::ConditionalBranch) {
                indent++;
            } else if (cmd.type == EventCommandType::EndConditional) {
                if (indent == 0) break;
                indent--;
            }
        }
    }
    
    continueExecution();
}

void EventSystem::executeWait(const EventCommandParams& params) {
    if (!params.intParams.empty()) {
        waitTimer = static_cast<float>(params.intParams[0]) / 60.0f; // Frames para segundos
        isWaiting = true;
        std::cout << "[EventSystem] Aguardando " << waitTimer << " segundos\n";
    } else {
        continueExecution();
    }
}

void EventSystem::executeTransferPlayer(const EventCommandParams& params) {
    // Implementação básica - apenas log por agora
    if (params.intParams.size() >= 3) {
        int mapId = params.intParams[0];
        int x = params.intParams[1];
        int y = params.intParams[2];
        std::cout << "[EventSystem] Transfer para mapa " << mapId << " posição (" << x << ", " << y << ")\n";
        
        // TODO: Implementar transferência real quando tivermos sistema de mapas completo
    }
    continueExecution();
}

void EventSystem::executePlayBGM(const EventCommandParams& params) {
    if (!params.stringParams.empty()) {
        std::string filename = params.stringParams[0];
        std::cout << "[EventSystem] Tocando BGM: " << filename << "\n";
        // TODO: Implementar reprodução de áudio
    }
    continueExecution();
}

void EventSystem::executePlaySE(const EventCommandParams& params) {
    if (!params.stringParams.empty()) {
        std::string filename = params.stringParams[0];
        std::cout << "[EventSystem] Tocando SE: " << filename << "\n";
        // TODO: Implementar reprodução de áudio
    }
    continueExecution();
}

void EventSystem::executeShowPicture(const EventCommandParams& params) {
    if (params.intParams.size() >= 3 && !params.stringParams.empty()) {
        int pictureId = params.intParams[0];
        int x = params.intParams[1];
        int y = params.intParams[2];
        std::string filename = params.stringParams[0];
        
        // Tentar carregar a imagem através do TextureManager
        try {
            const sf::Texture& texture = textureManager->acquire(filename);
            pictures[pictureId] = std::make_unique<sf::Sprite>(texture);
            pictures[pictureId]->setPosition({static_cast<float>(x), static_cast<float>(y)});
            std::cout << "[EventSystem] Exibindo imagem " << pictureId << ": " << filename << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[EventSystem] Erro ao carregar imagem: " << filename << " - " << e.what() << "\n";
        }
    }
    continueExecution();
}

void EventSystem::executeErasePicture(const EventCommandParams& params) {
    if (!params.intParams.empty()) {
        int pictureId = params.intParams[0];
        pictures.erase(pictureId);
        std::cout << "[EventSystem] Removendo imagem " << pictureId << "\n";
    }
    continueExecution();
}

bool EventSystem::evaluateCondition(int switchId, bool switchValue, int varId, int varValue) {
    if (switchId > 0) {
        return getSwitch(switchId) == switchValue;
    }
    if (varId > 0) {
        return getVariable(varId) == varValue;
    }
    return true; // Sem condições = sempre ativo
}

void EventSystem::continueExecution() {
    if (!isExecuting || !currentCommands) return;
    
    currentCommandIndex++;
    if (currentCommandIndex >= currentCommands->size()) {
        stopExecution();
        return;
    }
    
    executeCommand((*currentCommands)[currentCommandIndex]);
}

void EventSystem::stopExecution() {
    isExecuting = false;
    currentCommands = nullptr;
    currentCommandIndex = 0;
    showingText = false;
    isWaiting = false;
    std::cout << "[EventSystem] Execução de evento finalizada\n";
}
