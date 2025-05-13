#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D texture_diffuse1;
uniform bool useTexture;

void main()
{
    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Texture veya objectColor seçimi
    vec4 baseColor;
    if (useTexture)
        baseColor = texture(texture_diffuse1, TexCoord);
    else
        baseColor = vec4(objectColor, 1.0);

    // Final renklendirme
    vec3 lighting = ambient + diffuse;
    FragColor = vec4(lighting, 1.0) * baseColor;
}
