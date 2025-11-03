/**
 * Implementação de GLBuffer - Helper para VBOs/EBOs OpenGL
 */

#include "gl_buffer.h"
#include <cstring> // para memcpy

// ============================================================================
// GLBuffer - Vertex Buffer Object
// ============================================================================

GLBuffer::GLBuffer()
    : m_vbo(0)
    , m_size(0)
    , m_usage(BufferUsage::Static)
{
}

GLBuffer::~GLBuffer()
{
    Destroy();
}

GLBuffer::GLBuffer(GLBuffer&& other) noexcept
    : m_vbo(other.m_vbo)
    , m_size(other.m_size)
    , m_usage(other.m_usage)
{
    other.m_vbo = 0;
    other.m_size = 0;
}

GLBuffer& GLBuffer::operator=(GLBuffer&& other) noexcept
{
    if (this != &other) {
        Destroy();
        
        m_vbo = other.m_vbo;
        m_size = other.m_size;
        m_usage = other.m_usage;
        
        other.m_vbo = 0;
        other.m_size = 0;
    }
    return *this;
}

void GLBuffer::Create()
{
    if (m_vbo != 0) return; // Já criado
    
    glGenBuffers(1, &m_vbo);
}

void GLBuffer::Destroy()
{
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
        m_size = 0;
    }
}

void GLBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
}

void GLBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLBuffer::Upload(const void* data, size_t size, BufferUsage usage)
{
    if (m_vbo == 0) {
        Create();
    }
    
    m_usage = usage;
    m_size = size;
    
    Bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, ToGLUsage(usage));
    Unbind();
}

void GLBuffer::UploadVertices(const std::vector<TileVertex>& vertices, BufferUsage usage)
{
    if (vertices.empty()) {
        m_size = 0;
        return;
    }
    
    Upload(vertices.data(), vertices.size() * sizeof(TileVertex), usage);
}

void GLBuffer::UpdateRegion(const void* data, size_t size, size_t offset)
{
    if (m_vbo == 0 || data == nullptr || size == 0) return;
    if (offset + size > m_size) return; // Out of bounds
    
    Bind();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    Unbind();
}

void GLBuffer::SetupVertexAttributes()
{
    // Layout do TileVertex:
    // - 3 floats: posição (x, y, z)       -> location 0
    // - 2 floats: UV (u, v)               -> location 1
    // - 4 floats: cor/opacidade (r,g,b,a) -> location 2
    
    const size_t stride = sizeof(TileVertex);
    
    // Location 0: posição (x, y, z)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    
    // Location 1: UV (u, v)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    
    // Location 2: cor/opacidade (r, g, b, a)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
}

GLenum GLBuffer::ToGLUsage(BufferUsage usage)
{
    switch (usage) {
        case BufferUsage::Static:  return GL_STATIC_DRAW;
        case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
        case BufferUsage::Stream:  return GL_STREAM_DRAW;
        default:                   return GL_STATIC_DRAW;
    }
}

// ============================================================================
// IndexBuffer - Element Buffer Object
// ============================================================================

IndexBuffer::IndexBuffer()
    : m_ebo(0)
    , m_count(0)
{
}

IndexBuffer::~IndexBuffer()
{
    Destroy();
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : m_ebo(other.m_ebo)
    , m_count(other.m_count)
{
    other.m_ebo = 0;
    other.m_count = 0;
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
{
    if (this != &other) {
        Destroy();
        
        m_ebo = other.m_ebo;
        m_count = other.m_count;
        
        other.m_ebo = 0;
        other.m_count = 0;
    }
    return *this;
}

void IndexBuffer::Create()
{
    if (m_ebo != 0) return;
    
    glGenBuffers(1, &m_ebo);
}

void IndexBuffer::Destroy()
{
    if (m_ebo != 0) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
        m_count = 0;
    }
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
}

void IndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::Upload(const std::vector<unsigned int>& indices, BufferUsage usage)
{
    if (indices.empty()) {
        m_count = 0;
        return;
    }
    
    if (m_ebo == 0) {
        Create();
    }
    
    m_count = indices.size();
    
    Bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 indices.size() * sizeof(unsigned int), 
                 indices.data(), 
                 GLBuffer::ToGLUsage(usage));
    Unbind();
}
