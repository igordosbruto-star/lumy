/**
 * Implementação do File Watcher
 */

#include "file_watcher.h"
#include <wx/filename.h>
#include <wx/log.h>

wxBEGIN_EVENT_TABLE(FileWatcher, wxEvtHandler)
    EVT_FSWATCHER(wxID_ANY, FileWatcher::OnFileSystemEvent)
    EVT_TIMER(wxID_ANY, FileWatcher::OnDebounceTimer)
wxEND_EVENT_TABLE()

FileWatcher::FileWatcher()
    : m_watcher(nullptr)
    , m_isWatching(false)
    , m_debounceTimer(nullptr)
{
    m_watcher = new wxFileSystemWatcher();
    m_watcher->SetOwner(this);
    
    m_debounceTimer = new wxTimer(this);
}

FileWatcher::~FileWatcher()
{
    StopWatching();
    delete m_debounceTimer;
    delete m_watcher;
}

bool FileWatcher::StartWatching(const wxString& projectPath)
{
    if (m_isWatching) {
        StopWatching();
    }
    
    if (!wxDirExists(projectPath)) {
        wxLogError("Diretório do projeto não existe: %s", projectPath);
        return false;
    }
    
    // Monitorar o diretório inteiro recursivamente
    if (!m_watcher->Add(wxFileName::DirName(projectPath), wxFSW_EVENT_MODIFY | wxFSW_EVENT_CREATE)) {
        wxLogError("Falha ao iniciar monitoramento de: %s", projectPath);
        return false;
    }
    
    m_isWatching = true;
    m_watchedPath = projectPath;
    
    wxLogMessage("Hot-reload ativo: %s", projectPath);
    return true;
}

void FileWatcher::StopWatching()
{
    if (m_isWatching) {
        m_watcher->RemoveAll();
        m_isWatching = false;
        m_watchedPath.Clear();
        
        if (m_debounceTimer->IsRunning()) {
            m_debounceTimer->Stop();
        }
        
        wxLogMessage("Hot-reload desativado");
    }
}

void FileWatcher::RegisterCallback(const wxString& extension, FileChangeCallback callback)
{
    std::string ext = extension.Lower().ToStdString();
    m_callbacks[ext] = callback;
    
    wxLogMessage("Callback registrado para arquivos: %s", extension);
}

void FileWatcher::UnregisterCallback(const wxString& extension)
{
    std::string ext = extension.Lower().ToStdString();
    auto it = m_callbacks.find(ext);
    if (it != m_callbacks.end()) {
        m_callbacks.erase(it);
        wxLogMessage("Callback removido para: %s", extension);
    }
}

void FileWatcher::OnFileSystemEvent(wxFileSystemWatcherEvent& event)
{
    wxFileName filename = event.GetPath();
    wxString path = filename.GetFullPath();
    wxString name = filename.GetFullName();
    
    // Debug: mostrar eventos de arquivo
    wxString eventType;
    switch (event.GetChangeType()) {
        case wxFSW_EVENT_CREATE: eventType = "CREATED"; break;
        case wxFSW_EVENT_DELETE: eventType = "DELETED"; break;
        case wxFSW_EVENT_RENAME: eventType = "RENAMED"; break;
        case wxFSW_EVENT_MODIFY: eventType = "MODIFIED"; break;
        default: eventType = "UNKNOWN"; break;
    }
    
    wxLogMessage("File %s: %s", eventType, path);
    
    // Usar debounce para evitar múltiplos eventos seguidos
    m_pendingFile = path;
    
    if (m_debounceTimer->IsRunning()) {
        m_debounceTimer->Stop();
    }
    m_debounceTimer->Start(500, wxTIMER_ONE_SHOT); // 500ms de debounce
}

void FileWatcher::OnDebounceTimer(wxTimerEvent& WXUNUSED(event))
{
    if (!m_pendingFile.IsEmpty()) {
        wxFileName filename(m_pendingFile);
        ProcessFileChange(m_pendingFile, filename.GetFullName());
        m_pendingFile.Clear();
    }
}

void FileWatcher::ProcessFileChange(const wxString& path, const wxString& filename)
{
    wxFileName fn(path);
    wxString extension = fn.GetExt().Lower();
    
    auto it = m_callbacks.find(extension.ToStdString());
    if (it != m_callbacks.end()) {
        // Executar callback para esta extensão
        try {
            it->second(path, filename);
            wxLogMessage("Hot-reload processado: %s", filename);
        }
        catch (const std::exception& e) {
            wxLogError("Erro no hot-reload de %s: %s", filename, e.what());
        }
    }
}
