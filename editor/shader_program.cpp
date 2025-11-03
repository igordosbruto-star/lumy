/**
 * Implementação do sistema de shaders OpenGL
 */

#include "shader_program.h"
#include <wx/log.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <fstream>
#include <sstream>

// ============================================================================
// ShaderProgram
// ============================================================================

ShaderProgram::ShaderProgram()
    : m_programId(0)
    , m_vertexShaderId(0)
    , m_fragmentShaderId(0)
{
}

ShaderProgram::~ShaderProgram()
{
    Clear();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : m_programId(other.m_programId)
    , m_vertexShaderId(other.m_vertexShaderId)
    , m_fragmentShaderId(other.m_fragmentShaderId)
    , m_uniformCache(std::move(other.m_uniformCache))
{
    other.m_programId = 0;
    other.m_vertexShaderId = 0;
    other.m_fragmentShaderId = 0;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    if (this != &other) {
        Clear();
        
        m_programId = other.m_programId;
        m_vertexShaderId = other.m_vertexShaderId;
        m_fragmentShaderId = other.m_fragmentShaderId;
        m_uniformCache = std::move(other.m_uniformCache);
        
        other.m_programId = 0;
        other.m_vertexShaderId = 0;
        other.m_fragmentShaderId = 0;
    }
    return *this;
}

bool ShaderProgram::LoadFromFiles(const wxString& vertexPath, const wxString& fragmentPath)
{
    // Carregar código-fonte dos arquivos
    std::string vertexSrc = LoadTextFile(vertexPath);
    std::string fragmentSrc = LoadTextFile(fragmentPath);
    
    if (vertexSrc.empty() || fragmentSrc.empty()) {
        wxLogError("ShaderProgram: Falha ao carregar arquivos de shader");
        return false;
    }
    
    return LoadFromSource(vertexSrc, fragmentSrc);
}

bool ShaderProgram::LoadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    // Limpar shader anterior se existir
    Clear();
    
    // Compilar shaders
    m_vertexShaderId = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    if (m_vertexShaderId == 0) {
        wxLogError("ShaderProgram: Falha ao compilar vertex shader");
        return false;
    }
    
    m_fragmentShaderId = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (m_fragmentShaderId == 0) {
        wxLogError("ShaderProgram: Falha ao compilar fragment shader");
        glDeleteShader(m_vertexShaderId);
        m_vertexShaderId = 0;
        return false;
    }
    
    // Linkar programa
    m_programId = LinkProgram(m_vertexShaderId, m_fragmentShaderId);
    if (m_programId == 0) {
        wxLogError("ShaderProgram: Falha ao linkar programa shader");
        glDeleteShader(m_vertexShaderId);
        glDeleteShader(m_fragmentShaderId);
        m_vertexShaderId = 0;
        m_fragmentShaderId = 0;
        return false;
    }
    
    wxLogMessage("ShaderProgram: Shader carregado com sucesso (ID: %u)", m_programId);
    return true;
}

void ShaderProgram::Use() const
{
    if (m_programId != 0) {
        glUseProgram(m_programId);
    }
}

void ShaderProgram::Unbind() const
{
    glUseProgram(0);
}

void ShaderProgram::SetInt(const std::string& name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void ShaderProgram::SetFloat(const std::string& name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void ShaderProgram::SetVec2(const std::string& name, float x, float y)
{
    glUniform2f(GetUniformLocation(name), x, y);
}

void ShaderProgram::SetVec3(const std::string& name, float x, float y, float z)
{
    glUniform3f(GetUniformLocation(name), x, y, z);
}

void ShaderProgram::SetVec4(const std::string& name, float x, float y, float z, float w)
{
    glUniform4f(GetUniformLocation(name), x, y, z, w);
}

void ShaderProgram::SetMat4(const std::string& name, const float* value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, value);
}

GLint ShaderProgram::GetUniformLocation(const std::string& name)
{
    // Verificar cache
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }
    
    // Obter localização do OpenGL
    GLint location = glGetUniformLocation(m_programId, name.c_str());
    
    if (location == -1) {
        wxLogWarning("ShaderProgram: Uniform '%s' não encontrado", name.c_str());
    }
    
    // Cachear para futuras chamadas
    m_uniformCache[name] = location;
    
    return location;
}

void ShaderProgram::Clear()
{
    if (m_programId != 0) {
        glDeleteProgram(m_programId);
        m_programId = 0;
    }
    
    if (m_vertexShaderId != 0) {
        glDeleteShader(m_vertexShaderId);
        m_vertexShaderId = 0;
    }
    
    if (m_fragmentShaderId != 0) {
        glDeleteShader(m_fragmentShaderId);
        m_fragmentShaderId = 0;
    }
    
    m_uniformCache.clear();
}

GLuint ShaderProgram::CompileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        wxLogError("ShaderProgram: Falha ao criar shader");
        return 0;
    }
    
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    
    if (!CheckCompileErrors(shader, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")) {
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint ShaderProgram::LinkProgram(GLuint vertexId, GLuint fragmentId)
{
    GLuint program = glCreateProgram();
    if (program == 0) {
        wxLogError("ShaderProgram: Falha ao criar programa");
        return 0;
    }
    
    glAttachShader(program, vertexId);
    glAttachShader(program, fragmentId);
    glLinkProgram(program);
    
    if (!CheckLinkErrors(program)) {
        glDeleteProgram(program);
        return 0;
    }
    
    // Shaders já estão linkados, podem ser deletados
    // (mas vamos mantê-los para referência)
    
    return program;
}

bool ShaderProgram::CheckCompileErrors(GLuint shader, const std::string& type)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        wxLogError("ShaderProgram: Erro de compilação (%s):\n%s", type, infoLog);
        return false;
    }
    
    return true;
}

bool ShaderProgram::CheckLinkErrors(GLuint program)
{
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        wxLogError("ShaderProgram: Erro de linking:\n%s", infoLog);
        return false;
    }
    
    return true;
}

std::string ShaderProgram::LoadTextFile(const wxString& filePath)
{
    if (!wxFileExists(filePath)) {
        wxLogError("ShaderProgram: Arquivo não encontrado: %s", filePath);
        return "";
    }
    
    std::ifstream file(filePath.ToStdString());
    if (!file.is_open()) {
        wxLogError("ShaderProgram: Falha ao abrir arquivo: %s", filePath);
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return buffer.str();
}

// ============================================================================
// ShaderManager - Singleton
// ============================================================================

ShaderManager& ShaderManager::Get()
{
    static ShaderManager instance;
    return instance;
}

ShaderManager::~ShaderManager()
{
    UnloadAll();
}

ShaderProgram* ShaderManager::LoadShader(const std::string& name, 
                                          const wxString& vertexPath, 
                                          const wxString& fragmentPath)
{
    // Verificar se já existe
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        wxLogMessage("ShaderManager: Shader '%s' já está carregado", name);
        return it->second.get();
    }
    
    // Criar novo shader
    auto shader = std::make_unique<ShaderProgram>();
    if (!shader->LoadFromFiles(vertexPath, fragmentPath)) {
        wxLogError("ShaderManager: Falha ao carregar shader '%s'", name);
        return nullptr;
    }
    
    // Adicionar ao cache
    ShaderProgram* ptr = shader.get();
    m_shaders[name] = std::move(shader);
    
    wxLogMessage("ShaderManager: Shader '%s' carregado (total: %zu)", name, m_shaders.size());
    
    return ptr;
}

ShaderProgram* ShaderManager::GetShader(const std::string& name)
{
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second.get();
    }
    
    wxLogWarning("ShaderManager: Shader '%s' não encontrado", name);
    return nullptr;
}

void ShaderManager::UnloadShader(const std::string& name)
{
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        wxLogMessage("ShaderManager: Removendo shader '%s'", name);
        m_shaders.erase(it);
    }
}

void ShaderManager::UnloadAll()
{
    if (!m_shaders.empty()) {
        wxLogMessage("ShaderManager: Limpando todos os shaders (%zu)", m_shaders.size());
        m_shaders.clear();
    }
}

bool ShaderManager::LoadDefaultShaders(const wxString& shadersDir)
{
    wxLogMessage("ShaderManager: Carregando shaders padrão de: %s", shadersDir);
    
    // Garantir que o diretório existe
    if (!wxDirExists(shadersDir)) {
        wxLogError("ShaderManager: Diretório de shaders não encontrado: %s", shadersDir);
        return false;
    }
    
    bool success = true;
    
    // Carregar shader de tiles
    wxString tileVert = shadersDir + "/tile_vertex.glsl";
    wxString tileFrag = shadersDir + "/tile_fragment.glsl";
    if (LoadShader("tile", tileVert, tileFrag) == nullptr) {
        wxLogError("ShaderManager: Falha ao carregar shader 'tile'");
        success = false;
    }
    
    // Carregar shader de grid
    wxString gridVert = shadersDir + "/grid_vertex.glsl";
    wxString gridFrag = shadersDir + "/grid_fragment.glsl";
    if (LoadShader("grid", gridVert, gridFrag) == nullptr) {
        wxLogError("ShaderManager: Falha ao carregar shader 'grid'");
        success = false;
    }
    
    // Carregar shader de overlay (usa os mesmos vertex que grid)
    wxString overlayFrag = shadersDir + "/overlay_fragment.glsl";
    if (LoadShader("overlay", gridVert, overlayFrag) == nullptr) {
        wxLogError("ShaderManager: Falha ao carregar shader 'overlay'");
        success = false;
    }
    
    if (success) {
        wxLogMessage("ShaderManager: Todos os shaders padrão carregados com sucesso");
    }
    
    return success;
}
