#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 ceilingPos;
uniform vec3 ceilingColor;
uniform float ceilingIntensity;
uniform vec3 objectColor;
uniform bool useTexture;
uniform sampler2D texture_diffuse1;
uniform vec3 spotLights[5];
uniform vec3 spotDirs[5];
uniform float intensities[5];

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lighting = vec3(0.0);

    for (int i = 0; i < 5; ++i)
    {
        // Spot ýþýk yönü
        vec3 lightDir = normalize(spotLights[i] - FragPos);

        float diff = max(dot(norm, lightDir), 0.0);

        lighting += diff * intensities[i] * lightColor;
    }

    vec3 ceilingDir = normalize(ceilingPos - FragPos);
    float ceilingDiff = max(dot(norm, ceilingDir), 0.0);
    vec3 ceilingLight = ceilingDiff * ceilingIntensity * ceilingColor;
    lighting += ceilingLight;

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
    lighting += ambient;

    vec4 baseColor;
    if (useTexture)
        baseColor = texture(texture_diffuse1, vec2(FragPos.x, FragPos.z));
    else
        baseColor = vec4(objectColor, 1.0);

    FragColor = vec4(lighting, 1.0) * baseColor;
}
