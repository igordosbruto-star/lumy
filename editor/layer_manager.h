/**
 * LayerManager - Classe para gerenciar múltiplas camadas de um mapa
 */

#pragma once

#include "layer.h"
#include <wx/wx.h>
#include <vector>
#include <memory>
#include <algorithm>

class LayerManager {
public:
    LayerManager();
    ~LayerManager() = default;
    
    // Gerenciamento de layers
    Layer* CreateLayer(const LayerProperties& properties = LayerProperties());
    Layer* CreateLayer(int width, int height, const LayerProperties& properties = LayerProperties());
    bool AddLayer(std::unique_ptr<Layer> layer);
    bool RemoveLayer(int index);
    bool RemoveLayer(const wxString& name);
    void ClearLayers();
    
    // Acesso aos layers
    Layer* GetLayer(int index);
    const Layer* GetLayer(int index) const;
    Layer* GetLayer(const wxString& name);
    const Layer* GetLayer(const wxString& name) const;
    Layer* GetActiveLayer() { return GetLayer(m_activeLayerIndex); }
    const Layer* GetActiveLayer() const { return GetLayer(m_activeLayerIndex); }
    
    // Informações
    int GetLayerCount() const { return static_cast<int>(m_layers.size()); }
    int GetActiveLayerIndex() const { return m_activeLayerIndex; }
    bool SetActiveLayer(int index);
    bool SetActiveLayer(const wxString& name);
    
    // Operações de ordenação
    bool MoveLayerUp(int index);
    bool MoveLayerDown(int index);
    bool SetLayerOrder(int index, int newOrder);
    std::vector<Layer*> GetLayersSortedByZOrder() const;
    
    // Operações em lote
    void SetAllLayersVisible(bool visible);
    void SetLayerVisibility(int index, bool visible);
    void SetLayerOpacity(int index, float opacity);
    
    // Busca e filtros
    std::vector<int> FindLayersByType(LayerType type) const;
    std::vector<int> FindLayersByName(const wxString& pattern) const;
    std::vector<int> GetVisibleLayers() const;
    std::vector<int> GetUnlockedLayers() const;
    
    // Operações de tiles (delegadas ao layer ativo)
    int GetTile(int x, int y) const;
    void SetTile(int x, int y, int tileId);
    int GetTileFromLayer(int layerIndex, int x, int y) const;
    void SetTileInLayer(int layerIndex, int x, int y, int tileId);
    
    // Operações de área
    void FillRect(int x, int y, int width, int height, int tileId);
    void CopyRect(int srcX, int srcY, int width, int height, int srcLayer, int destLayer, int destX, int destY);
    
    // Operações especiais
    Layer* DuplicateLayer(int index);
    bool MergeLayerDown(int index);
    void FlattenLayers(); // Combinar todos os layers visíveis em um
    
    // Dimensões
    void ResizeAllLayers(int newWidth, int newHeight, int defaultTile = -1);
    int GetWidth() const;
    int GetHeight() const;
    
    // Serialização
    bool LoadFromJson(const wxString& jsonData);
    wxString SaveToJson() const;
    
    // Estado
    bool HasUnsavedChanges() const;
    void MarkAllLayersSaved();
    
    // Utilitários
    wxString GenerateUniqueLayerName(const wxString& baseName = "Nova Camada") const;
    bool IsValidLayerIndex(int index) const;
    
    // Debug
    void PrintDebugInfo() const;

private:
    void UpdateLayerZOrders();
    int FindLayerIndex(const wxString& name) const;
    
    std::vector<std::unique_ptr<Layer>> m_layers;
    int m_activeLayerIndex;
    int m_nextLayerId;
};