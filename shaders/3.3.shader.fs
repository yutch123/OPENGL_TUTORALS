#version 330 core

// вход из вершинного шейдера
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// uniform из программы
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D texture1;

out vec4 FragColor;

void main()
{
    // нормаль в мировых координатах
    vec3 norm = normalize(Normal);

    // вектор к источнику света
    vec3 lightDir = normalize(lightPos - FragPos);

    // diffuse
    float diff = max(dot(norm, lightDir), 0.0);

    // view direction
    vec3 viewDir = normalize(viewPos - FragPos);

    // half vector для Blinn-Phong
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0); // shininess 32

    vec3 color = texture(texture1, TexCoords).rgb;

    vec3 ambient = 0.1 * color;
    vec3 diffuse = diff * color;
    vec3 specular = spec * vec3(1.0); // белый свет

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}