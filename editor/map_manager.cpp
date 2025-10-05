/**
 * Implementação do MapManager
 */

#pragma execution_character_set("utf-8")

#include "map_manager.h"
#include "utf8_strings.h"
#include <wx/filename.h>
#include <wx/ffile.h>
#include <wx/msgdlg.h>

MapManager::MapManager()
    : m_hasUnsavedChanges(false)
{
    // Não criar mapa padrão automaticamente
}

// Getters
std::shared_ptr<Map> MapManager::GetCurrentMap() const
{
    // Converter unique_ptr para shared_ptr - cuidado: isso pode ser problemático
    // se o unique_ptr for movido enquanto o shared_ptr ainda existir
    if (m_currentMap) {
        // Criar um shared_ptr que não gerencia a memória (alias)
        return std::shared_ptr<Map>(m_currentMap.get(), [](Map*){});
    }
    return nullptr;
}

wxString MapManager::GetCurrentMapName() const
{
    if (!m_currentMap) {
        return UTF8("Sem mapa");
    }
    return m_currentMap->GetMetadata().name;
}

// Setters
void MapManager::SetCurrentMap(std::shared_ptr<Map> map)
{
    if (map) {
        // Converter shared_ptr para unique_ptr copiando o conteúdo
        m_currentMap = std::make_unique<Map>(*map);
    } else {
        m_currentMap.reset();
    }
    m_currentMapPath.Clear();
    m_hasUnsavedChanges = true;
}

// Operações de mapa
bool MapManager::NewMap(int width, int height, const wxString& name)
{
    try {
        m_currentMap = std::make_unique<Map>(width, height);
        m_currentMap->SetName(name);
        m_currentMapPath.Clear();
        m_hasUnsavedChanges = true;
        
        wxLogMessage("Novo mapa criado: %s (%dx%d)", name, width, height);
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Erro ao criar novo mapa: %s", e.what());
        return false;
    } catch (...) {
        wxLogError("Erro inesperado ao criar novo mapa");
        return false;
    }
}

bool MapManager::LoadMap(const wxString& filePath)
{
    if (!wxFileExists(filePath)) {
        wxLogError("Arquivo não encontrado: %s", filePath);
        return false;
    }
    
    return LoadMapFromFile(filePath);
}

// Sobrecarga para std::string
bool MapManager::LoadMap(const std::string& filePath)
{
    return LoadMapFromFile(wxString(filePath));
}

bool MapManager::SaveMap()
{
    if (!m_currentMap) {
        wxLogError("Nenhum mapa para salvar");
        return false;
    }
    
    if (m_currentMapPath.IsEmpty()) {
        // Mapa nunca foi salvo, precisa de Save As
        wxLogWarning("Caminho do mapa não definido. Use SaveMapAs() primeiro.");
        return false;
    }
    
    return SaveMapToFile(m_currentMapPath);
}

// Sobrecarga para std::string
bool MapManager::SaveMap(const std::string& filePath) const
{
    if (!m_currentMap) {
        return false;
    }
    return const_cast<MapManager*>(this)->SaveMapToFile(wxString(filePath));
}

bool MapManager::SaveMapAs(const wxString& filePath)
{
    if (!m_currentMap) {
        wxLogError("Nenhum mapa para salvar");
        return false;
    }
    
    if (SaveMapToFile(filePath)) {
        m_currentMapPath = filePath;
        
        // Atualizar nome do mapa baseado no arquivo se necessário
        wxFileName fileName(filePath);
        if (m_currentMap->GetMetadata().name == "Novo Mapa") {
            m_currentMap->SetName(fileName.GetName());
        }
        
        return true;
    }
    
    return false;
}

bool MapManager::CloseMap()
{
    m_currentMap.reset();
    m_currentMapPath.Clear();
    m_hasUnsavedChanges = false;
    
    wxLogMessage("Mapa fechado");
    return true;
}

// Acesso aos dados do mapa
int MapManager::GetTile(int x, int y) const
{
    if (!m_currentMap) {
        return -1;
    }
    return m_currentMap->GetTile(x, y);
}

void MapManager::SetTile(int x, int y, int tileId)
{
    if (!m_currentMap) {
        return;
    }
    
    int oldTile = m_currentMap->GetTile(x, y);
    if (oldTile != tileId) {
        m_currentMap->SetTile(x, y, tileId);
        m_hasUnsavedChanges = true;
    }
}

int MapManager::GetMapWidth() const
{
    return m_currentMap ? m_currentMap->GetWidth() : 0;
}

int MapManager::GetMapHeight() const
{
    return m_currentMap ? m_currentMap->GetHeight() : 0;
}

// Validação
bool MapManager::IsValidPosition(int x, int y) const
{
    return m_currentMap && m_currentMap->IsValidPosition(x, y);
}

// Estado
void MapManager::SetMapModified(bool modified)
{
    if (m_currentMap) {
        m_currentMap->SetModified(modified);
        m_hasUnsavedChanges = modified;
    }
}

// Utilidades
wxString MapManager::GetMapDisplayTitle() const
{
    if (!m_currentMap) {
        return UTF8("Lumy Editor");
    }
    
    wxString title = m_currentMap->GetMetadata().name;
    
    if (m_currentMap->IsModified() || m_hasUnsavedChanges) {
        title += " *";
    }
    
    if (!m_currentMapPath.IsEmpty()) {
        wxFileName fileName(m_currentMapPath);
        title += wxString::Format(" [%s]", fileName.GetFullName());
    }
    
    return title;
}

bool MapManager::PromptSaveIfModified(wxWindow* parent)
{
    if (!m_currentMap || (!m_currentMap->IsModified() && !m_hasUnsavedChanges)) {
        return true; // Nada para salvar
    }
    
    wxString message = wxString::Format(
        UTF8("O mapa '%s' foi modificado.\n\nDeseja salvar as alterações antes de continuar?"),
        GetCurrentMapName()
    );
    
    int result = wxMessageBox(
        message,
        UTF8("Salvar alterações?"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION,
        parent
    );
    
    if (result == wxYES) {
        if (m_currentMapPath.IsEmpty()) {
            // Precisa de Save As - deixar para o chamador lidar com isso
            return false; // Indica que precisa de Save As
        } else {
            return SaveMap();
        }
    } else if (result == wxNO) {
        return true; // Continuar sem salvar
    } else {
        return false; // Cancelar operação
    }
}

// Debug
void MapManager::PrintDebugInfo() const
{
    wxLogMessage("=== MapManager Debug Info ===");
    wxLogMessage("Has Map: %s", HasMap() ? "Yes" : "No");
    wxLogMessage("Map Path: %s", m_currentMapPath.IsEmpty() ? "(none)" : m_currentMapPath.c_str());
    wxLogMessage("Has Unsaved Changes: %s", m_hasUnsavedChanges ? "Yes" : "No");
    
    if (m_currentMap) {
        wxLogMessage("Current Map:");
        m_currentMap->PrintDebugInfo();
    }
    
    wxLogMessage("Display Title: %s", GetMapDisplayTitle());
    wxLogMessage("=============================");
}

// Métodos privados
bool MapManager::LoadMapFromFile(const wxString& filePath)
{
    try {
        wxFFile file(filePath, "r");
        if (!file.IsOpened()) {
            wxLogError("Não foi possível abrir o arquivo: %s", filePath);
            return false;
        }
        
        wxString jsonData;
        if (!file.ReadAll(&jsonData)) {
            wxLogError("Erro ao ler o arquivo: %s", filePath);
            return false;
        }
        
        file.Close();
        
        // Criar novo mapa e carregar dados
        auto newMap = std::make_unique<Map>();
        if (!newMap->LoadFromJson(jsonData)) {
            wxLogError("Erro ao carregar mapa do arquivo: %s", filePath);
            return false;
        }
        
        // Sucesso - substituir mapa atual
        m_currentMap = std::move(newMap);
        m_currentMapPath = filePath;
        m_hasUnsavedChanges = false;
        
        wxFileName fileName(filePath);
        wxLogMessage("Mapa carregado com sucesso: %s", fileName.GetFullName());
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Exceção ao carregar mapa: %s", e.what());
        return false;
    } catch (...) {
        wxLogError("Erro inesperado ao carregar mapa");
        return false;
    }
}

bool MapManager::SaveMapToFile(const wxString& filePath)
{
    if (!m_currentMap) {
        wxLogError("Nenhum mapa para salvar");
        return false;
    }
    
    try {
        wxString jsonData = m_currentMap->SaveToJson();
        if (jsonData.IsEmpty()) {
            wxLogError("Falha ao serializar mapa para JSON");
            return false;
        }
        
        wxFFile file(filePath, "w");
        if (!file.IsOpened()) {
            wxLogError("Não foi possível criar o arquivo: %s", filePath);
            return false;
        }
        
        if (!file.Write(jsonData)) {
            wxLogError("Erro ao escrever no arquivo: %s", filePath);
            file.Close();
            return false;
        }
        
        file.Close();
        
        // Marcar como salvo
        m_currentMap->MarkSaved();
        m_hasUnsavedChanges = false;
        
        wxFileName fileName(filePath);
        wxLogMessage("Mapa salvo com sucesso: %s", fileName.GetFullName());
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Exceção ao salvar mapa: %s", e.what());
        return false;
    } catch (...) {
        wxLogError("Erro inesperado ao salvar mapa");
        return false;
    }
}

wxString MapManager::GenerateDefaultName() const
{
    static int counter = 1;
    return wxString::Format("Mapa %d", counter++);
}
