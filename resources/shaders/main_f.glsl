#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform vec3 lightPos;

float ambientStrength = 0.2;
vec3 lightColor = vec3(1.0, 1.0, 1.0);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 ambient = ambientStrength * lightColor;

    vec3 result = (ambient + diffuse) * texture(ourTexture, TexCoord).xyz;
    // vec3 result = texture(ourTexture, TexCoord).xyz;
    FragColor = vec4(result, 1.0);
}