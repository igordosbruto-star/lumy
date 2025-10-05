/**
 * File Watcher - Sistema de monitoramento de arquivos para hot-reload
 * Monitora mudanças em arquivos TMX, JSON e assets do projeto
 */

#pragma once

#include <wx/wx.h>
#include <wx/fswatcher.h>
#include <functional>
#include <unordered_map>
#include <string>

class FileWatcher : public wxEvtHandler
{
public:
    // Callback para quando um arquivo é modificado
    using FileChangeCallback = std::function<void(const wxString& path, const wxString& filename)>;

    FileWatcher();
    virtual ~FileWatcher();

    // Iniciar/parar monitoramento
    bool StartWatching(const wxString& projectPath);
    void StopWatching();
    
    // Registrar callbacks para tipos de arquivo
    void RegisterCallback(const wxString& extension, FileChangeCallback callback);
    void UnregisterCallback(const wxString& extension);
    
    // Status do monitoramento
    bool IsWatching() const { return m_isWatching; }
    const wxString& GetWatchedPath() const { return m_watchedPath; }

private:
    // Event handlers
    void OnFileSystemEvent(wxFileSystemWatcherEvent& event);
    
    // Processar mudanças baseado na extensão
    void ProcessFileChange(const wxString& path, const wxString& filename);
    
    // Membros
    wxFileSystemWatcher* m_watcher;
    bool m_isWatching;
    wxString m_watchedPath;
    
    // Callbacks por extensão de arquivo
    std::unordered_map<std::string, FileChangeCallback> m_callbacks;
    
    // Debounce para evitar múltiplos eventos
    wxTimer* m_debounceTimer;
    wxString m_pendingFile;
    
    void OnDebounceTimer(wxTimerEvent& event);

    wxDECLARE_EVENT_TABLE();
};
