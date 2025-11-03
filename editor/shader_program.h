/**
 * ShaderProgram - Sistema de carregamento e gerenciamento de shaders OpenGL
 */

#pragma once

#include <GL/glew.h>
#include <wx/wx.h>
#include <string>
#include <unordered_map>

/**
 * ShaderProgram - Wrapper para programas shader OpenGL
 * 
 * Gerencia compilação, linking e uso de shaders GLSL.
 * Suporta uniforms e fornece interface simplificada.
 */
class ShaderProgram {
public:
    /**
     * Construtor padrão
     */
    ShaderProgram();
    
    /**
     * Destrutor - libera recursos OpenGL
     */
    ~ShaderProgram();
    
    // Desabilitar cópia
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    
    // Permitir move
    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;
    
    /**
     * Carrega e compila shaders a partir de arquivos
     * 
     * @param vertexPath Caminho do vertex shader (.glsl)
     * @param fragmentPath Caminho do fragment shader (.glsl)
     * @return true se compilado com sucesso
     */
    bool LoadFromFiles(const wxString& vertexPath, const wxString& fragmentPath);
    
    /**
     * Cria shader a partir de strings de código-fonte
     * 
     * @param vertexSrc Código-fonte do vertex shader
     * @param fragmentSrc Código-fonte do fragment shader
     * @return true se compilado com sucesso
     */
    bool LoadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc);
    
    /**
     * Ativa o programa shader para uso
     */
    void Use() const;
    
    /**
     * Desativa o programa shader
     */
    void Unbind() const;
    
    /**
     * Verifica se o shader está válido e pronto para uso
     */
    bool IsValid() const { return m_programId != 0; }
    
    /**
     * Retorna o ID do programa OpenGL
     */
    GLuint GetProgramID() const { return m_programId; }
    
    /**
     * Define valor de uniform (int)
     */
    void SetInt(const std::string& name, int value);
    
    /**
     * Define valor de uniform (float)
     */
    void SetFloat(const std::string& name, float value);
    
    /**
     * Define valor de uniform (vec2)
     */
    void SetVec2(const std::string& name, float x, float y);
    
    /**
     * Define valor de uniform (vec3)
     */
    void SetVec3(const std::string& name, float x, float y, float z);
    
    /**
     * Define valor de uniform (vec4)
     */
    void SetVec4(const std::string& name, float x, float y, float z, float w);
    
    /**
     * Define valor de uniform (mat4)
     */
    void SetMat4(const std::string& name, const float* value);
    
    /**
     * Obtém localização de um uniform
     * Usa cache interno para evitar chamadas desnecessárias a glGetUniformLocation
     */
    GLint GetUniformLocation(const std::string& name);
    
    /**
     * Limpa recursos
     */
    void Clear();

private:
    GLuint m_programId;      // ID do programa shader
    GLuint m_vertexShaderId; // ID do vertex shader
    GLuint m_fragmentShaderId; // ID do fragment shader
    
    // Cache de localizações de uniforms
    std::unordered_map<std::string, GLint> m_uniformCache;
    
    /**
     * Compila um shader a partir de código-fonte
     * 
     * @param type Tipo do shader (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
     * @param source Código-fonte GLSL
     * @return ID do shader compilado (0 se falhar)
     */
    GLuint CompileShader(GLenum type, const std::string& source);
    
    /**
     * Linka o programa shader
     * 
     * @param vertexId ID do vertex shader
     * @param fragmentId ID do fragment shader
     * @return ID do programa linkado (0 se falhar)
     */
    GLuint LinkProgram(GLuint vertexId, GLuint fragmentId);
    
    /**
     * Verifica erros de compilação de shader
     */
    bool CheckCompileErrors(GLuint shader, const std::string& type);
    
    /**
     * Verifica erros de linking
     */
    bool CheckLinkErrors(GLuint program);
    
    /**
     * Carrega arquivo de texto
     */
    static std::string LoadTextFile(const wxString& filePath);
};

/**
 * ShaderManager - Gerencia múltiplos shaders com cache
 * Singleton para fácil acesso global
 */
class ShaderManager {
public:
    /**
     * Retorna instância singleton
     */
    static ShaderManager& Get();
    
    /**
     * Carrega um shader e adiciona ao cache
     * 
     * @param name Nome/identificador do shader
     * @param vertexPath Caminho do vertex shader
     * @param fragmentPath Caminho do fragment shader
     * @return Ponteiro para o shader (nullptr se falhar)
     */
    ShaderProgram* LoadShader(const std::string& name, 
                              const wxString& vertexPath, 
                              const wxString& fragmentPath);
    
    /**
     * Obtém um shader do cache
     * 
     * @param name Nome do shader
     * @return Ponteiro para o shader (nullptr se não encontrado)
     */
    ShaderProgram* GetShader(const std::string& name);
    
    /**
     * Remove um shader do cache
     */
    void UnloadShader(const std::string& name);
    
    /**
     * Remove todos os shaders
     */
    void UnloadAll();
    
    /**
     * Carrega shaders padrão do editor
     * (tile, grid, overlay)
     */
    bool LoadDefaultShaders(const wxString& shadersDir);

private:
    ShaderManager() = default;
    ~ShaderManager();
    
    // Desabilitar cópia e move
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;
    
    std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> m_shaders;
};
