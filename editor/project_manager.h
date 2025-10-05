/**
 * Project Manager - Sistema de gerenciamento de projetos Lumy
 */

#pragma once

#include <wx/wx.h>
#include <wx/filename.h>
#include <memory>
#include <vector>

// Estrutura de informações do projeto
struct ProjectInfo
{
    wxString projectPath;
    wxString projectName;
    wxString version;
    wxString description;
    wxString author;
    wxDateTime created;
    wxDateTime lastModified;
    
    // Diretórios do projeto
    wxString mapsDir;
    wxString dataDir;
    wxString assetsDir;
    wxString configFile;
    
    ProjectInfo() : version("0.1.0") {}
};

// Estrutura de um template de projeto
struct ProjectTemplate
{
    wxString name;
    wxString description;
    wxString templateDir;
    wxArrayString requiredDirs;
    wxArrayString templateFiles;
};

class ProjectManager
{
public:
    ProjectManager();
    ~ProjectManager();
    
    // Métodos principais
    bool CreateNewProject(const wxString& projectPath, const ProjectInfo& info);
    bool OpenProject(const wxString& projectPath);
    bool SaveProject();
    bool SaveProjectAs(const wxString& projectPath);
    void CloseProject();
    
    // Getters
    const ProjectInfo& GetCurrentProject() const { return m_currentProject; }
    bool HasOpenProject() const { return m_projectOpen; }
    wxString GetProjectName() const;
    wxString GetProjectPath() const;
    
    // Estrutura de diretórios
    bool CreateProjectStructure(const wxString& projectPath);
    bool ValidateProjectStructure(const wxString& projectPath);
    
    // Templates de projeto
    void LoadProjectTemplates();
    const std::vector<ProjectTemplate>& GetAvailableTemplates() const;
    bool CreateFromTemplate(const wxString& projectPath, const ProjectInfo& info, const wxString& templateName);
    
    // Configuração do projeto
    bool LoadProjectConfig(const wxString& projectPath);
    bool SaveProjectConfig();
    
    // Utilitários
    wxArrayString GetRecentProjects() const;
    void AddToRecentProjects(const wxString& projectPath);
    bool IsValidProjectPath(const wxString& path) const;
    
private:
    // Estado atual
    ProjectInfo m_currentProject;
    bool m_projectOpen;
    bool m_projectModified;
    
    // Templates disponíveis
    std::vector<ProjectTemplate> m_projectTemplates;
    
    // Configurações
    wxString m_configDir;
    wxString m_templatesDir;
    wxString m_recentProjectsFile;
    
    // Métodos auxiliares
    bool CreateDirectory(const wxString& path);
    bool CreateConfigFile(const wxString& projectPath, const ProjectInfo& info);
    bool LoadConfigFile(const wxString& configPath, ProjectInfo& info);
    void InitializeTemplates();
    wxString GetDefaultConfigContent(const ProjectInfo& info);
    
    // Validação
    bool ValidateProjectInfo(const ProjectInfo& info);
    bool CheckRequiredFiles(const wxString& projectPath);
    
    // Criação de conteúdo padrão
    void CreateDefaultMap(const wxString& mapPath);
};
