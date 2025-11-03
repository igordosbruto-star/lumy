/**
 * Implementação do Project Manager
 */

#include "project_manager.h"
#include <wx/dir.h>
#include <wx/textfile.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ProjectManager::ProjectManager()
    : m_projectOpen(false)
    , m_projectModified(false)
{
    // Configurar diretórios do sistema
    wxStandardPaths& paths = wxStandardPaths::Get();
    m_configDir = paths.GetUserConfigDir() + wxFileName::GetPathSeparator() + "LumyEditor";
    m_templatesDir = m_configDir + wxFileName::GetPathSeparator() + "templates";
    m_recentProjectsFile = m_configDir + wxFileName::GetPathSeparator() + "recent_projects.txt";
    
    // Criar diretórios se não existem
    if (!wxDir::Exists(m_configDir)) {
        wxDir::Make(m_configDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    
    if (!wxDir::Exists(m_templatesDir)) {
        wxDir::Make(m_templatesDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    
    // Inicializar templates
    InitializeTemplates();
}

ProjectManager::~ProjectManager()
{
    if (m_projectOpen) {
        CloseProject();
    }
}

bool ProjectManager::CreateNewProject(const wxString& projectPath, const ProjectInfo& info)
{
    if (!ValidateProjectInfo(info)) {
        wxLogError("Informações do projeto inválidas");
        return false;
    }
    
    // Verificar se o diretório já existe
    if (wxDir::Exists(projectPath)) {
        wxDir dir(projectPath);
        if (dir.IsOpened() && dir.HasFiles() || dir.HasSubDirs()) {
            wxLogError("Diretório do projeto deve estar vazio: %s", projectPath);
            return false;
        }
    }
    
    // Criar estrutura de diretórios
    if (!CreateProjectStructure(projectPath)) {
        wxLogError("Falha ao criar estrutura do projeto");
        return false;
    }
    
    // Configurar informações do projeto
    ProjectInfo newProject = info;
    newProject.projectPath = projectPath;
    newProject.created = wxDateTime::Now();
    newProject.lastModified = wxDateTime::Now();
    
    // Configurar diretórios padrão
    newProject.mapsDir = projectPath + wxFileName::GetPathSeparator() + "maps";
    newProject.dataDir = projectPath + wxFileName::GetPathSeparator() + "data";
    newProject.assetsDir = projectPath + wxFileName::GetPathSeparator() + "assets";
    newProject.configFile = projectPath + wxFileName::GetPathSeparator() + "project.lumy";
    
    // Criar arquivo de configuração
    if (!CreateConfigFile(projectPath, newProject)) {
        wxLogError("Falha ao criar arquivo de configuração");
        return false;
    }
    
    // Definir como projeto atual
    m_currentProject = newProject;
    m_projectOpen = true;
    m_projectModified = false;
    
    // Adicionar aos projetos recentes
    AddToRecentProjects(projectPath);
    
    wxLogMessage("Novo projeto criado: %s", projectPath);
    return true;
}

bool ProjectManager::OpenProject(const wxString& projectPath)
{
    if (!IsValidProjectPath(projectPath)) {
        wxLogError("Caminho do projeto inválido: %s", projectPath);
        return false;
    }
    
    // Fechar projeto atual se houver
    if (m_projectOpen) {
        CloseProject();
    }
    
    // Carregar configuração do projeto
    if (!LoadProjectConfig(projectPath)) {
        wxLogError("Falha ao carregar configuração do projeto");
        return false;
    }
    
    // Validar estrutura
    if (!ValidateProjectStructure(projectPath)) {
        wxLogWarning("Estrutura do projeto incompleta ou inválida");
        // Continuar mesmo assim para permitir recuperação
    }
    
    m_projectOpen = true;
    m_projectModified = false;
    
    // Adicionar aos projetos recentes
    AddToRecentProjects(projectPath);
    
    wxLogMessage("Projeto aberto: %s", projectPath);
    return true;
}

bool ProjectManager::SaveProject()
{
    if (!m_projectOpen) {
        wxLogError("Nenhum projeto aberto");
        return false;
    }
    
    return SaveProjectConfig();
}

bool ProjectManager::SaveProjectAs(const wxString& projectPath)
{
    if (!m_projectOpen) {
        wxLogError("Nenhum projeto aberto");
        return false;
    }
    
    // Atualizar caminho do projeto
    m_currentProject.projectPath = projectPath;
    m_currentProject.configFile = projectPath + wxFileName::GetPathSeparator() + "project.lumy";
    
    // Criar estrutura se necessário
    if (!wxDir::Exists(projectPath)) {
        if (!CreateProjectStructure(projectPath)) {
            return false;
        }
    }
    
    return SaveProjectConfig();
}

void ProjectManager::CloseProject()
{
    if (m_projectOpen) {
        if (m_projectModified) {
            int result = wxMessageBox(
                "O projeto tem mudanças não salvas. Deseja salvar?",
                "Salvar projeto",
                wxYES_NO | wxCANCEL | wxICON_QUESTION
            );
            
            if (result == wxYES) {
                SaveProject();
            } else if (result == wxCANCEL) {
                return; // Não fechar
            }
        }
        
        m_currentProject = ProjectInfo();
        m_projectOpen = false;
        m_projectModified = false;
        
        wxLogMessage("Projeto fechado");
    }
}

wxString ProjectManager::GetProjectName() const
{
    return m_projectOpen ? m_currentProject.projectName : wxString();
}

wxString ProjectManager::GetProjectPath() const
{
    return m_projectOpen ? m_currentProject.projectPath : wxString();
}

bool ProjectManager::CreateProjectStructure(const wxString& projectPath)
{
    // Criar diretório principal
    if (!CreateDirectory(projectPath)) {
        return false;
    }
    
    // Criar subdiretórios padrão
    wxArrayString dirs;
    dirs.Add("maps");
    dirs.Add("data");
    dirs.Add("assets");
    dirs.Add("assets/sprites");
    dirs.Add("assets/tilesets");
    dirs.Add("assets/audio");
    dirs.Add("assets/audio/bgm");
    dirs.Add("assets/audio/se");
    
    for (const wxString& dir : dirs) {
        wxString fullPath = projectPath + wxFileName::GetPathSeparator() + dir;
        if (!CreateDirectory(fullPath)) {
            return false;
        }
    }
    
    // Criar arquivos padrão da database
    wxArrayString dataFiles = {
        "actors.json", "items.json", "skills.json", 
        "states.json", "enemies.json", "system.json"
    };
    
    for (const wxString& dataFile : dataFiles) {
        wxString filePath = projectPath + wxFileName::GetPathSeparator() + "data" + wxFileName::GetPathSeparator() + dataFile;
        wxTextFile file(filePath);
        if (!file.Exists()) {
            file.Create();
            file.AddLine("{}"); // JSON vazio
            file.Write();
        }
    }
    
    // Criar mapa de exemplo
    wxString mapPath = projectPath + wxFileName::GetPathSeparator() + "maps" + wxFileName::GetPathSeparator() + "hello.tmx";
    CreateDefaultMap(mapPath);
    
    return true;
}

bool ProjectManager::ValidateProjectStructure(const wxString& projectPath)
{
    // Verificar diretório principal
    if (!wxDir::Exists(projectPath)) {
        return false;
    }
    
    // Verificar arquivo de configuração
    wxString configPath = projectPath + wxFileName::GetPathSeparator() + "project.lumy";
    if (!wxFile::Exists(configPath)) {
        return false;
    }
    
    // Verificar diretórios principais
    wxArrayString requiredDirs = {"maps", "data", "assets"};
    for (const wxString& dir : requiredDirs) {
        wxString dirPath = projectPath + wxFileName::GetPathSeparator() + dir;
        if (!wxDir::Exists(dirPath)) {
            return false;
        }
    }
    
    return true;
}

bool ProjectManager::LoadProjectConfig(const wxString& projectPath)
{
    wxString configPath = projectPath + wxFileName::GetPathSeparator() + "project.lumy";
    
    if (!wxFile::Exists(configPath)) {
        wxLogError("Arquivo de configuração não encontrado: %s", configPath);
        return false;
    }
    
    try {
        wxFileInputStream input(configPath);
        if (!input.IsOk()) {
            return false;
        }
        
        wxTextInputStream text(input, wxT("\t"), wxConvUTF8);
        wxString jsonContent;
        
        while (!input.Eof()) {
            jsonContent += text.ReadLine() + "\n";
        }
        
        // Parse JSON
        std::string jsonStdStr = std::string(jsonContent.utf8_str());
        json config = json::parse(jsonStdStr);
        
        // Carregar informações
        m_currentProject.projectPath = projectPath;
        m_currentProject.projectName = wxString::FromUTF8(config["name"].get<std::string>());
        m_currentProject.version = wxString::FromUTF8(config["version"].get<std::string>());
        m_currentProject.description = wxString::FromUTF8(config.value("description", ""));
        m_currentProject.author = wxString::FromUTF8(config.value("author", ""));
        
        // Configurar caminhos
        m_currentProject.mapsDir = projectPath + wxFileName::GetPathSeparator() + "maps";
        m_currentProject.dataDir = projectPath + wxFileName::GetPathSeparator() + "data";
        m_currentProject.assetsDir = projectPath + wxFileName::GetPathSeparator() + "assets";
        m_currentProject.configFile = configPath;
        
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Erro ao carregar configuração: %s", e.what());
        return false;
    }
}

bool ProjectManager::SaveProjectConfig()
{
    if (!m_projectOpen) {
        return false;
    }
    
    try {
        json config;
        config["name"] = std::string(m_currentProject.projectName.utf8_str());
        config["version"] = std::string(m_currentProject.version.utf8_str());
        config["description"] = std::string(m_currentProject.description.utf8_str());
        config["author"] = std::string(m_currentProject.author.utf8_str());
        config["created"] = std::string(m_currentProject.created.FormatISOCombined().utf8_str());
        config["lastModified"] = std::string(wxDateTime::Now().FormatISOCombined().utf8_str());
        
        // Estrutura do projeto
        config["directories"]["maps"] = "maps";
        config["directories"]["data"] = "data";
        config["directories"]["assets"] = "assets";
        
        wxFileOutputStream output(m_currentProject.configFile);
        if (!output.IsOk()) {
            return false;
        }
        
        wxTextOutputStream text(output);
        wxString jsonString = wxString::FromUTF8(config.dump(4));
        text.WriteString(jsonString);
        
        m_projectModified = false;
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Erro ao salvar configuração: %s", e.what());
        return false;
    }
}

wxArrayString ProjectManager::GetRecentProjects() const
{
    wxArrayString recentProjects;
    
    if (wxFile::Exists(m_recentProjectsFile)) {
        wxTextFile file(m_recentProjectsFile);
        if (file.Open()) {
            for (size_t i = 0; i < file.GetLineCount(); ++i) {
                wxString path = file.GetLine(i);
                if (!path.IsEmpty() && wxDir::Exists(path)) {
                    recentProjects.Add(path);
                }
            }
        }
    }
    
    return recentProjects;
}

void ProjectManager::AddToRecentProjects(const wxString& projectPath)
{
    wxArrayString recent = GetRecentProjects();
    
    // Remover se já existe
    int index = recent.Index(projectPath);
    if (index != wxNOT_FOUND) {
        recent.RemoveAt(index);
    }
    
    // Adicionar no início
    recent.Insert(projectPath, 0);
    
    // Manter apenas os 10 mais recentes
    while (recent.Count() > 10) {
        recent.RemoveAt(recent.Count() - 1);
    }
    
    // Salvar
    wxTextFile file(m_recentProjectsFile);
    if (file.Exists()) {
        file.Open();
        file.Clear();
    } else {
        file.Create();
    }
    
    for (const wxString& path : recent) {
        file.AddLine(path);
    }
    
    file.Write();
}

bool ProjectManager::IsValidProjectPath(const wxString& path) const
{
    if (!wxDir::Exists(path)) {
        return false;
    }
    
    wxString configFile = path + wxFileName::GetPathSeparator() + "project.lumy";
    return wxFile::Exists(configFile);
}

// Métodos auxiliares privados

bool ProjectManager::CreateDirectory(const wxString& path)
{
    if (wxDir::Exists(path)) {
        return true;
    }
    
    return wxDir::Make(path, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
}

bool ProjectManager::CreateConfigFile(const wxString& projectPath, const ProjectInfo& info)
{
    wxString configPath = projectPath + wxFileName::GetPathSeparator() + "project.lumy";
    
    try {
        json config;
        config["name"] = std::string(info.projectName.utf8_str());
        config["version"] = std::string(info.version.utf8_str());
        config["description"] = std::string(info.description.utf8_str());
        config["author"] = std::string(info.author.utf8_str());
        config["created"] = std::string(info.created.FormatISOCombined().utf8_str());
        config["lastModified"] = std::string(info.lastModified.FormatISOCombined().utf8_str());
        
        // Estrutura do projeto
        config["directories"]["maps"] = "maps";
        config["directories"]["data"] = "data";
        config["directories"]["assets"] = "assets";
        
        wxFileOutputStream output(configPath);
        if (!output.IsOk()) {
            return false;
        }
        
        wxTextOutputStream text(output);
        wxString jsonString = wxString::FromUTF8(config.dump(4));
        text.WriteString(jsonString);
        
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Erro ao criar arquivo de configuração: %s", e.what());
        return false;
    }
}

void ProjectManager::InitializeTemplates()
{
    // Template básico
    ProjectTemplate basicTemplate;
    basicTemplate.name = "Projeto Básico";
    basicTemplate.description = "Projeto Lumy padrão com estrutura básica";
    basicTemplate.requiredDirs = {"maps", "data", "assets"};
    
    m_projectTemplates.push_back(basicTemplate);
}

bool ProjectManager::ValidateProjectInfo(const ProjectInfo& info)
{
    if (info.projectName.IsEmpty()) {
        return false;
    }
    
    // Verificar caracteres inválidos no nome
    if (info.projectName.find_first_of("\\/:*?\"<>|") != wxString::npos) {
        return false;
    }
    
    return true;
}

void ProjectManager::CreateDefaultMap(const wxString& mapPath)
{
    // Criar um mapa TMX básico (placeholder)
    wxTextFile file(mapPath);
    file.Create();
    
    // TMX básico 20x15 tiles
    file.AddLine("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    file.AddLine("<map version=\"1.0\" orientation=\"orthogonal\" width=\"25\" height=\"15\" tilewidth=\"32\" tileheight=\"32\">");
    file.AddLine("  <tileset firstgid=\"1\" name=\"default\" tilewidth=\"32\" tileheight=\"32\">");
    file.AddLine("    <image source=\"../assets/tilesets/default.png\" width=\"256\" height=\"256\"/>");
    file.AddLine("  </tileset>");
    file.AddLine("  <layer name=\"ground\" width=\"25\" height=\"15\">");
    file.AddLine("    <data encoding=\"csv\">");
    
    // Gerar dados do mapa (bordas = 2, interior = 1)
    for (int y = 0; y < 15; ++y) {
        wxString line;
        for (int x = 0; x < 25; ++x) {
            if (x == 0 || x == 24 || y == 0 || y == 14) {
                line += "2"; // Parede
            } else {
                line += "1"; // Chão
            }
            if (x < 24) line += ",";
        }
        if (y < 14) line += ",";
        file.AddLine("      " + line);
    }
    
    file.AddLine("    </data>");
    file.AddLine("  </layer>");
    file.AddLine("</map>");
    
    file.Write();
}
