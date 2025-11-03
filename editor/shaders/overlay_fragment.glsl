#version 330 core

// Input
in vec4 vColor;

// Output
out vec4 FragColor;

void main()
{
    // Usar cor sólida com alpha para overlay transparente
    FragColor = vColor;
}
