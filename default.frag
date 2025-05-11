#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos = vec3(1.2f, 1.0f, 2.0f);
uniform vec3 viewPos = vec3(0.0f, 0.0f, 3.0f);
uniform vec3 lightColor = vec3(1.0f);
uniform vec3 objectColor = vec3(0.8, 0.3, 0.3);

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

    // Final color
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}
