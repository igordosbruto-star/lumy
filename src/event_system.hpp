// src/event_system.hpp
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <optional>
#include <SFML/Graphics.hpp>
#include <sol/sol.hpp>

// Forward declarations
class SceneStack;
class TextureManager;

// Enum para tipos de comandos de evento
enum class EventCommandType {
    ShowText = 101,
    SetSwitch = 121,
    SetVariable = 122,
    ConditionalBranch = 111,
    EndConditional = 412,
    Wait = 230,
    TransferPlayer = 201,
    PlayBGM = 241,
    PlaySE = 250,
    ShowPicture = 231,
    ErasePicture = 235
};

// Estrutura para parâmetros de comando
struct EventCommandParams {
    std::vector<std::string> stringParams;
    std::vector<int> intParams;
    std::vector<float> floatParams;
    std::unordered_map<std::string, std::string> extraData;
};

// Classe para um comando de evento
class EventCommand {
public:
    EventCommandType type;
    EventCommandParams params;
    int indent = 0; // Para controlar estruturas if/else

    EventCommand(EventCommandType t, const EventCommandParams& p = {}) 
        : type(t), params(p) {}
};

// Classe para uma página de evento
class EventPage {
public:
    std::vector<EventCommand> commands;
    bool enabled = true;
    
    // Condições para ativar a página
    int switchId = 0;
    bool switchValue = true;
    int variableId = 0;
    int variableValue = 0;
};

// Classe principal do evento
class GameEvent {
public:
    int id;
    std::string name;
    int x, y; // Posição no mapa
    std::vector<EventPage> pages;
    
    GameEvent(int eventId, const std::string& eventName, int posX, int posY)
        : id(eventId), name(eventName), x(posX), y(posY) {}
};

// Sistema de execução de eventos
class EventSystem {
private:
    SceneStack* sceneStack;
    TextureManager* textureManager;
    sol::state lua;
    
    // Estado do sistema
    std::unordered_map<int, bool> switches;
    std::unordered_map<int, int> variables;
    std::vector<GameEvent> events;
    
    // Controle de execução
    bool isExecuting = false;
    size_t currentCommandIndex = 0;
    std::vector<EventCommand>* currentCommands = nullptr;
    
    // UI para texto
    sf::Font font;
    std::optional<sf::Text> textDisplay;
    sf::RectangleShape textBackground;
    bool showingText = false;
    std::string currentText;
    float textTimer = 0.0f;
    
    // Controle de wait
    float waitTimer = 0.0f;
    bool isWaiting = false;
    
    // Sistema de imagens
    std::unordered_map<int, std::unique_ptr<sf::Sprite>> pictures;

public:
    EventSystem(SceneStack* stack, TextureManager* textures);
    
    // Inicialização
    bool initialize();
    
    // Controle de switches e variáveis
    void setSwitch(int id, bool value);
    bool getSwitch(int id) const;
    void setVariable(int id, int value);
    int getVariable(int id) const;
    
    // Controle de eventos
    void addEvent(const GameEvent& event);
    void triggerEvent(int eventId);
    void executeCommand(const EventCommand& command);
    
    // Sistema de execução
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    bool isEventRunning() const { return isExecuting; }
    
    // Manipulação de input durante eventos
    void handleInput(const sf::Event& event);
    
    // Comandos específicos
private:
    void executeShowText(const EventCommandParams& params);
    void executeSetSwitch(const EventCommandParams& params);
    void executeSetVariable(const EventCommandParams& params);
    void executeConditionalBranch(const EventCommandParams& params);
    void executeWait(const EventCommandParams& params);
    void executeTransferPlayer(const EventCommandParams& params);
    void executePlayBGM(const EventCommandParams& params);
    void executePlaySE(const EventCommandParams& params);
    void executeShowPicture(const EventCommandParams& params);
    void executeErasePicture(const EventCommandParams& params);
    
    // Utilitários
    bool evaluateCondition(int switchId, bool switchValue, int varId, int varValue);
    void continueExecution();
    void stopExecution();
};
