#version 330 core

// Inputs do vertex shader
in vec2 vTexCoord;
in vec4 vColor;

// Uniform
uniform sampler2D uTexture; // Textura do tileset

// Output
out vec4 FragColor;

void main()
{
    // Amostra a textura e multiplica pela cor/opacidade
    vec4 texColor = texture(uTexture, vTexCoord);
    
    // Multiplicar pela cor do vértice (inclui opacidade da layer)
    FragColor = texColor * vColor;
    
    // Descartar fragmentos completamente transparentes
    if (FragColor.a < 0.01) {
        discard;
    }
}
