#version 330 core

// Atributos de entrada
layout(location = 0) in vec3 aPos;   // Posição (x, y, z)
layout(location = 1) in vec4 aColor; // Cor da linha

// Uniforms
uniform mat4 uViewProj;

// Output
out vec4 vColor;

void main()
{
    gl_Position = uViewProj * vec4(aPos, 1.0);
    vColor = aColor;
}
