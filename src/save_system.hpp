// src/save_system.hpp
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// Estrutura para salvar dados do jogador
struct PlayerSaveData {
    int mapId = 1;
    float x = 5.0f;
    float y = 5.0f;
    int direction = 2; // 0=up, 1=right, 2=down, 3=left
    int level = 1;
    int exp = 0;
    int gold = 0;
    std::vector<int> party; // IDs dos membros do grupo
    std::vector<int> inventory; // IDs dos itens no inventário
};

// Sistema principal de Save/Load
class SaveSystem {
private:
    std::unordered_map<int, bool> globalSwitches;
    std::unordered_map<int, int> globalVariables;
    PlayerSaveData playerData;
    std::string saveDirectory;
    
public:
    SaveSystem();
    
    // Inicialização
    bool initialize(const std::string& gameDirectory);
    
    // Controle de switches globais
    void setSwitch(int id, bool value);
    bool getSwitch(int id) const;
    
    // Controle de variáveis globais
    void setVariable(int id, int value);
    int getVariable(int id) const;
    
    // Dados do jogador
    void setPlayerPosition(int mapId, float x, float y, int direction = 2);
    void getPlayerPosition(int& mapId, float& x, float& y, int& direction) const;
    
    void setPlayerStats(int level, int exp, int gold);
    void getPlayerStats(int& level, int& exp, int& gold) const;
    
    void setParty(const std::vector<int>& party);
    std::vector<int> getParty() const;
    
    void setInventory(const std::vector<int>& inventory);
    std::vector<int> getInventory() const;
    
    // Operações de arquivo
    bool saveGame(int slotId = 1) const;
    bool loadGame(int slotId = 1);
    bool saveExists(int slotId = 1) const;
    bool deleteSave(int slotId = 1);
    
    // Utilitários
    void resetToDefaults();
    std::vector<int> getAvailableSaveSlots() const;
    std::string getSaveFilePath(int slotId) const;
    
private:
    // Serialização
    std::string serializeToJson() const;
    bool deserializeFromJson(const std::string& jsonData);
    
    // Utilitários de arquivo
    bool ensureSaveDirectoryExists() const;
};
