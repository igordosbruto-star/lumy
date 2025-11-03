#version 330 core

// Atributos de entrada (vertex attributes)
layout(location = 0) in vec3 aPos;      // Posição (x, y, z)
layout(location = 1) in vec2 aTexCoord; // Coordenadas de textura (u, v)
layout(location = 2) in vec4 aColor;    // Cor/opacidade (r, g, b, a)

// Uniforms
uniform mat4 uViewProj; // Matriz de projeção ortográfica (view * projection)

// Outputs para o fragment shader
out vec2 vTexCoord;
out vec4 vColor;

void main()
{
    // Transformar posição para clip space
    gl_Position = uViewProj * vec4(aPos, 1.0);
    
    // Passar coordenadas de textura e cor para o fragment shader
    vTexCoord = aTexCoord;
    vColor = aColor;
}
