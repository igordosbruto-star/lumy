/**
 * GLBuffer - Helper class para gerenciar Vertex Buffer Objects (VBOs)
 * Simplifica criação, upload e binding de buffers OpenGL
 */

#pragma once

// Usar GLEW para extensões OpenGL modernas
#include <GL/glew.h>

#include <vector>
#include <cstddef>

/**
 * Estrutura de vértice para renderização de tiles
 * Layout: posição (3 floats) + UV (2 floats) + cor/opacidade (4 floats)
 */
struct TileVertex {
    // Posição no espaço 3D
    float x, y, z;
    
    // Coordenadas de textura (UV mapping)
    float u, v;
    
    // Cor e opacidade da layer (RGBA)
    float r, g, b, a;
    
    TileVertex() 
        : x(0), y(0), z(0)
        , u(0), v(0)
        , r(1), g(1), b(1), a(1) 
    {}
    
    TileVertex(float px, float py, float pz, 
               float tu, float tv,
               float cr = 1.0f, float cg = 1.0f, float cb = 1.0f, float ca = 1.0f)
        : x(px), y(py), z(pz)
        , u(tu), v(tv)
        , r(cr), g(cg), b(cb), a(ca)
    {}
};

/**
 * Tipo de uso do buffer (hint para OpenGL sobre como o buffer será usado)
 */
enum class BufferUsage {
    Static,   // Dados não mudam após upload inicial (GL_STATIC_DRAW)
    Dynamic,  // Dados mudam frequentemente (GL_DYNAMIC_DRAW)
    Stream    // Dados mudam a cada frame (GL_STREAM_DRAW)
};

/**
 * GLBuffer - Wrapper RAII para OpenGL Vertex Buffer Objects
 * 
 * Gerencia automaticamente a vida útil do buffer OpenGL e fornece
 * interface simples para upload e binding de dados de vértices.
 */
class GLBuffer {
public:
    /**
     * Construtor padrão - cria buffer vazio
     */
    GLBuffer();
    
    /**
     * Destrutor - libera recursos OpenGL
     */
    ~GLBuffer();
    
    // Desabilitar cópia (resource ownership)
    GLBuffer(const GLBuffer&) = delete;
    GLBuffer& operator=(const GLBuffer&) = delete;
    
    // Permitir move
    GLBuffer(GLBuffer&& other) noexcept;
    GLBuffer& operator=(GLBuffer&& other) noexcept;
    
    /**
     * Inicializa o buffer OpenGL
     * Deve ser chamado com contexto OpenGL ativo
     */
    void Create();
    
    /**
     * Libera o buffer OpenGL
     */
    void Destroy();
    
    /**
     * Verifica se o buffer foi criado
     */
    bool IsValid() const { return m_vbo != 0; }
    
    /**
     * Faz bind do buffer como GL_ARRAY_BUFFER
     */
    void Bind() const;
    
    /**
     * Unbind do buffer (bind 0)
     */
    void Unbind() const;
    
    /**
     * Upload de dados para o buffer
     * 
     * @param data Ponteiro para os dados
     * @param size Tamanho em bytes
     * @param usage Hint de uso do buffer
     */
    void Upload(const void* data, size_t size, BufferUsage usage = BufferUsage::Static);
    
    /**
     * Upload de vetor de TileVertex para o buffer
     * 
     * @param vertices Vetor de vértices
     * @param usage Hint de uso do buffer
     */
    void UploadVertices(const std::vector<TileVertex>& vertices, 
                        BufferUsage usage = BufferUsage::Static);
    
    /**
     * Atualiza uma região do buffer (partial update)
     * Útil para atualizar apenas parte dos dados sem realocar o buffer inteiro
     * 
     * @param data Ponteiro para os novos dados
     * @param size Tamanho em bytes
     * @param offset Offset em bytes no buffer
     */
    void UpdateRegion(const void* data, size_t size, size_t offset = 0);
    
    /**
     * Retorna o ID do VBO OpenGL
     */
    GLuint GetID() const { return m_vbo; }
    
    /**
     * Retorna o tamanho atual do buffer em bytes
     */
    size_t GetSize() const { return m_size; }
    
    /**
     * Retorna o número de vértices armazenados
     * (assumindo que o buffer contém TileVertex)
     */
    size_t GetVertexCount() const { return m_size / sizeof(TileVertex); }
    
    /**
     * Configura atributos de vértice para TileVertex
     * Deve ser chamado após Bind() e antes do draw call
     * 
     * Layout:
     * - location 0: posição (x, y, z)
     * - location 1: UV (u, v)
     * - location 2: cor/opacidade (r, g, b, a)
     */
    static void SetupVertexAttributes();

    /**
     * Converte BufferUsage para GLenum OpenGL
     */
    static GLenum ToGLUsage(BufferUsage usage);

private:
    GLuint m_vbo;      // ID do Vertex Buffer Object
    size_t m_size;     // Tamanho atual do buffer em bytes
    BufferUsage m_usage; // Hint de uso
};

/**
 * IndexBuffer - Wrapper para Element Buffer Objects (EBO/IBO)
 * Usado para indexed drawing (reduz duplicação de vértices)
 */
class IndexBuffer {
public:
    IndexBuffer();
    ~IndexBuffer();
    
    // Desabilitar cópia
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    
    // Permitir move
    IndexBuffer(IndexBuffer&& other) noexcept;
    IndexBuffer& operator=(IndexBuffer&& other) noexcept;
    
    void Create();
    void Destroy();
    bool IsValid() const { return m_ebo != 0; }
    
    void Bind() const;
    void Unbind() const;
    
    /**
     * Upload de índices (uint32)
     */
    void Upload(const std::vector<unsigned int>& indices, 
                BufferUsage usage = BufferUsage::Static);
    
    GLuint GetID() const { return m_ebo; }
    size_t GetIndexCount() const { return m_count; }

private:
    GLuint m_ebo;
    size_t m_count;
};
