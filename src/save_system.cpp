// src/save_system.cpp
#include "save_system.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

SaveSystem::SaveSystem() {
    resetToDefaults();
}

bool SaveSystem::initialize(const std::string& gameDirectory) {
    saveDirectory = gameDirectory + "/saves";
    
    if (!ensureSaveDirectoryExists()) {
        std::cerr << "[SaveSystem] Erro: não foi possível criar diretório de saves: " << saveDirectory << "\n";
        return false;
    }
    
    std::cout << "[SaveSystem] Inicializado com diretório: " << saveDirectory << "\n";
    return true;
}

void SaveSystem::setSwitch(int id, bool value) {
    globalSwitches[id] = value;
    std::cout << "[SaveSystem] Switch global " << id << " = " << (value ? "ON" : "OFF") << "\n";
}

bool SaveSystem::getSwitch(int id) const {
    auto it = globalSwitches.find(id);
    return it != globalSwitches.end() ? it->second : false;
}

void SaveSystem::setVariable(int id, int value) {
    globalVariables[id] = value;
    std::cout << "[SaveSystem] Variável global " << id << " = " << value << "\n";
}

int SaveSystem::getVariable(int id) const {
    auto it = globalVariables.find(id);
    return it != globalVariables.end() ? it->second : 0;
}

void SaveSystem::setPlayerPosition(int mapId, float x, float y, int direction) {
    playerData.mapId = mapId;
    playerData.x = x;
    playerData.y = y;
    playerData.direction = direction;
}

void SaveSystem::getPlayerPosition(int& mapId, float& x, float& y, int& direction) const {
    mapId = playerData.mapId;
    x = playerData.x;
    y = playerData.y;
    direction = playerData.direction;
}

void SaveSystem::setPlayerStats(int level, int exp, int gold) {
    playerData.level = level;
    playerData.exp = exp;
    playerData.gold = gold;
}

void SaveSystem::getPlayerStats(int& level, int& exp, int& gold) const {
    level = playerData.level;
    exp = playerData.exp;
    gold = playerData.gold;
}

void SaveSystem::setParty(const std::vector<int>& party) {
    playerData.party = party;
}

std::vector<int> SaveSystem::getParty() const {
    return playerData.party;
}

void SaveSystem::setInventory(const std::vector<int>& inventory) {
    playerData.inventory = inventory;
}

std::vector<int> SaveSystem::getInventory() const {
    return playerData.inventory;
}

bool SaveSystem::saveGame(int slotId) const {
    std::string filePath = getSaveFilePath(slotId);
    
    try {
        std::string jsonData = serializeToJson();
        
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "[SaveSystem] Erro: não foi possível abrir arquivo para escrita: " << filePath << "\n";
            return false;
        }
        
        file << jsonData;
        file.close();
        
        std::cout << "[SaveSystem] Jogo salvo no slot " << slotId << ": " << filePath << "\n";
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[SaveSystem] Erro ao salvar: " << e.what() << "\n";
        return false;
    }
}

bool SaveSystem::loadGame(int slotId) {
    std::string filePath = getSaveFilePath(slotId);
    
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "[SaveSystem] Arquivo de save não encontrado: " << filePath << "\n";
        return false;
    }
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "[SaveSystem] Erro: não foi possível abrir arquivo para leitura: " << filePath << "\n";
            return false;
        }
        
        std::string jsonData;
        std::string line;
        while (std::getline(file, line)) {
            jsonData += line + "\n";
        }
        file.close();
        
        if (deserializeFromJson(jsonData)) {
            std::cout << "[SaveSystem] Jogo carregado do slot " << slotId << ": " << filePath << "\n";
            return true;
        } else {
            std::cerr << "[SaveSystem] Erro ao deserializar dados do save\n";
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[SaveSystem] Erro ao carregar: " << e.what() << "\n";
        return false;
    }
}

bool SaveSystem::saveExists(int slotId) const {
    std::string filePath = getSaveFilePath(slotId);
    return std::filesystem::exists(filePath);
}

bool SaveSystem::deleteSave(int slotId) {
    std::string filePath = getSaveFilePath(slotId);
    
    if (!std::filesystem::exists(filePath)) {
        return true; // Já não existe
    }
    
    try {
        std::filesystem::remove(filePath);
        std::cout << "[SaveSystem] Save removido: " << filePath << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SaveSystem] Erro ao remover save: " << e.what() << "\n";
        return false;
    }
}

void SaveSystem::resetToDefaults() {
    globalSwitches.clear();
    globalVariables.clear();
    
    playerData = PlayerSaveData();
    playerData.party.push_back(1); // Hero por padrão
    
    std::cout << "[SaveSystem] Dados resetados para padrões\n";
}

std::vector<int> SaveSystem::getAvailableSaveSlots() const {
    std::vector<int> slots;
    
    for (int i = 1; i <= 10; ++i) { // Até 10 slots
        if (saveExists(i)) {
            slots.push_back(i);
        }
    }
    
    return slots;
}

std::string SaveSystem::getSaveFilePath(int slotId) const {
    return saveDirectory + "/save" + std::to_string(slotId) + ".json";
}

std::string SaveSystem::serializeToJson() const {
    json saveData;
    
    // Metadados
    saveData["version"] = "1.0";
    saveData["timestamp"] = std::time(nullptr);
    
    // Switches globais
    json switchesJson;
    for (const auto& [id, value] : globalSwitches) {
        switchesJson[std::to_string(id)] = value;
    }
    saveData["switches"] = switchesJson;
    
    // Variáveis globais
    json variablesJson;
    for (const auto& [id, value] : globalVariables) {
        variablesJson[std::to_string(id)] = value;
    }
    saveData["variables"] = variablesJson;
    
    // Dados do jogador
    json playerJson;
    playerJson["mapId"] = playerData.mapId;
    playerJson["x"] = playerData.x;
    playerJson["y"] = playerData.y;
    playerJson["direction"] = playerData.direction;
    playerJson["level"] = playerData.level;
    playerJson["exp"] = playerData.exp;
    playerJson["gold"] = playerData.gold;
    playerJson["party"] = playerData.party;
    playerJson["inventory"] = playerData.inventory;
    
    saveData["player"] = playerJson;
    
    return saveData.dump(4); // Pretty print com indentação de 4 espaços
}

bool SaveSystem::deserializeFromJson(const std::string& jsonData) {
    try {
        json saveData = json::parse(jsonData);
        
        // Verificar versão
        if (!saveData.contains("version")) {
            std::cerr << "[SaveSystem] Arquivo de save sem versão\n";
            return false;
        }
        
        std::string version = saveData["version"];
        if (version != "1.0") {
            std::cerr << "[SaveSystem] Versão do save não suportada: " << version << "\n";
            return false;
        }
        
        // Carregar switches
        if (saveData.contains("switches")) {
            globalSwitches.clear();
            for (const auto& [key, value] : saveData["switches"].items()) {
                int id = std::stoi(key);
                globalSwitches[id] = value.get<bool>();
            }
        }
        
        // Carregar variáveis
        if (saveData.contains("variables")) {
            globalVariables.clear();
            for (const auto& [key, value] : saveData["variables"].items()) {
                int id = std::stoi(key);
                globalVariables[id] = value.get<int>();
            }
        }
        
        // Carregar dados do jogador
        if (saveData.contains("player")) {
            const json& playerJson = saveData["player"];
            
            if (playerJson.contains("mapId")) playerData.mapId = playerJson["mapId"];
            if (playerJson.contains("x")) playerData.x = playerJson["x"];
            if (playerJson.contains("y")) playerData.y = playerJson["y"];
            if (playerJson.contains("direction")) playerData.direction = playerJson["direction"];
            if (playerJson.contains("level")) playerData.level = playerJson["level"];
            if (playerJson.contains("exp")) playerData.exp = playerJson["exp"];
            if (playerJson.contains("gold")) playerData.gold = playerJson["gold"];
            
            if (playerJson.contains("party")) {
                playerData.party = playerJson["party"].get<std::vector<int>>();
            }
            
            if (playerJson.contains("inventory")) {
                playerData.inventory = playerJson["inventory"].get<std::vector<int>>();
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[SaveSystem] Erro ao fazer parse do JSON: " << e.what() << "\n";
        return false;
    }
}

bool SaveSystem::ensureSaveDirectoryExists() const {
    try {
        if (!std::filesystem::exists(saveDirectory)) {
            std::filesystem::create_directories(saveDirectory);
        }
        return std::filesystem::is_directory(saveDirectory);
    } catch (const std::exception& e) {
        std::cerr << "[SaveSystem] Erro ao criar diretório: " << e.what() << "\n";
        return false;
    }
}
