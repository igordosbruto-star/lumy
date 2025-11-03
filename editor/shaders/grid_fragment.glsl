#version 330 core

// Input
in vec4 vColor;

// Output
out vec4 FragColor;

void main()
{
    // Simplesmente usar a cor passada do vertex shader
    // Anti-aliasing será feito via GL_LINE_SMOOTH se necessário
    FragColor = vColor;
}
