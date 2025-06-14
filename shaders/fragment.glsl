#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform float time;
uniform bool isSelected;
uniform bool isPlaying;
uniform vec3 baseColor;

void main()
{
    vec3 color = baseColor;
    
    if (isSelected) {
        // Pulsing effect for selected notes
        float pulse = (sin(time * 4.0) + 1.0) * 0.5;
        color = mix(baseColor, vec3(1.0), pulse * 0.3);
    }
    
    if (isPlaying) {
        // Bright flash effect when playing
        float flash = exp(-fract(time * 2.0) * 3.0);
        color = mix(color, vec3(1.0), flash * 0.7);
    }
    
    // Add a subtle gradient
    float gradient = 1.0 - (TexCoord.y * 0.2);
    color *= gradient;
    
    // Add a border
    float border = 0.05;
    if (TexCoord.x < border || TexCoord.x > 1.0 - border ||
        TexCoord.y < border || TexCoord.y > 1.0 - border) {
        color *= 0.8;
    }
    
    FragColor = vec4(color, 1.0);
} 