/**
 * Command - Sistema de Undo/Redo usando Command Pattern
 * Permite desfazer e refazer operações de edição
 */

#pragma once

#include <wx/wx.h>
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class Map;
class Layer;

// Interface base para comandos
class ICommand
{
public:
    virtual ~ICommand() = default;
    
    // Executar o comando
    virtual bool Execute() = 0;
    
    // Desfazer o comando
    virtual bool Undo() = 0;
    
    // Refazer o comando
    virtual bool Redo() { return Execute(); }
    
    // Nome do comando (para UI)
    virtual wxString GetName() const = 0;
    
    // Pode ser mesclado com outro comando?
    virtual bool CanMergeWith(const ICommand* other) const { return false; }
    
    // Mesclar com outro comando
    virtual void MergeWith(ICommand* other) {}
};

// Comando para pintar um tile
class PaintTileCommand : public ICommand
{
public:
    PaintTileCommand(Map* map, int layerIndex, int x, int y, int newTileId);
    
    bool Execute() override;
    bool Undo() override;
    wxString GetName() const override { return "Pintar Tile"; }
    
    bool CanMergeWith(const ICommand* other) const override;
    void MergeWith(ICommand* other) override;
    
private:
    Map* m_map;
    int m_layerIndex;
    int m_x, m_y;
    int m_oldTileId;
    int m_newTileId;
};

// Comando para preencher área com flood fill
class FillAreaCommand : public ICommand
{
public:
    FillAreaCommand(Map* map, int layerIndex, int startX, int startY, int newTileId);
    
    bool Execute() override;
    bool Undo() override;
    wxString GetName() const override { return "Preencher Área"; }
    
private:
    struct TileChange {
        int x, y;
        int oldTileId;
    };
    
    Map* m_map;
    int m_layerIndex;
    int m_startX, m_startY;
    int m_newTileId;
    std::vector<TileChange> m_changes;
    
    void GatherChanges(int x, int y, int targetTileId);
};

// Comando para criar layer
class CreateLayerCommand : public ICommand
{
public:
    CreateLayerCommand(Map* map, const wxString& layerName);
    
    bool Execute() override;
    bool Undo() override;
    wxString GetName() const override { return "Criar Layer"; }
    
private:
    Map* m_map;
    wxString m_layerName;
    int m_layerIndex; // Index where layer was created
};

// Comando para remover layer
class RemoveLayerCommand : public ICommand
{
public:
    RemoveLayerCommand(Map* map, int layerIndex);
    
    bool Execute() override;
    bool Undo() override;
    wxString GetName() const override { return "Remover Layer"; }
    
private:
    Map* m_map;
    int m_layerIndex;
    std::unique_ptr<Layer> m_savedLayer; // Backup of removed layer
};

// Comando para mover layer
class MoveLayerCommand : public ICommand
{
public:
    enum Direction { UP, DOWN };
    
    MoveLayerCommand(Map* map, int layerIndex, Direction direction);
    
    bool Execute() override;
    bool Undo() override;
    wxString GetName() const override;
    
private:
    Map* m_map;
    int m_layerIndex;
    Direction m_direction;
};

// Comando para duplicar layer
class DuplicateLayerCommand : public ICommand
{
public:
    DuplicateLayerCommand(Map* map, int layerIndex);
    
    bool Execute() override;
    bool Undo() override;
    wxString GetName() const override { return "Duplicar Layer"; }
    
private:
    Map* m_map;
    int m_sourceLayerIndex;
    int m_newLayerIndex;
};

// Gerenciador de histórico de comandos
class CommandHistory
{
public:
    CommandHistory(size_t maxHistorySize = 100);
    ~CommandHistory();
    
    // Executar e adicionar comando ao histórico
    bool ExecuteCommand(std::unique_ptr<ICommand> command);
    
    // Desfazer último comando
    bool Undo();
    
    // Refazer comando desfeito
    bool Redo();
    
    // Limpar histórico
    void Clear();
    
    // Informações
    bool CanUndo() const { return m_currentIndex > 0; }
    bool CanRedo() const { return m_currentIndex < static_cast<int>(m_commands.size()); }
    wxString GetUndoName() const;
    wxString GetRedoName() const;
    
    // Configuração
    void SetMaxHistorySize(size_t size) { m_maxHistorySize = size; }
    size_t GetMaxHistorySize() const { return m_maxHistorySize; }
    
    // Estatísticas
    size_t GetHistorySize() const { return m_commands.size(); }
    int GetCurrentIndex() const { return m_currentIndex; }
    
private:
    void TrimHistory();
    
    std::vector<std::unique_ptr<ICommand>> m_commands;
    int m_currentIndex; // Index of next command to redo
    size_t m_maxHistorySize;
};
