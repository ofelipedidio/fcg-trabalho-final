#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define BUNNY 1
#define PLANE 2
#define ROCK 3
#define FERN 4
#define BUILD1 5
#define FERRIS 6
#define ACACIA 7
#define FIREWORK 8
uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3;
uniform sampler2D TextureImage4;
uniform sampler2D TextureImage5;
uniform sampler2D TextureImage6;
uniform sampler2D TextureImage7;
uniform sampler2D TextureImage8;
uniform sampler2D TextureImage9;
uniform sampler2D TextureImage10;
uniform sampler2D TextureImage11;
uniform sampler2D TextureImage12;
uniform sampler2D TextureImage13;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = -l + 2 * n * dot(n,l); // PREENCHA AQUI o vetor de reflexão especular ideal

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd;
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    // Espectro da fonte de iluminação
    vec3 I = vec3(1.0,1.0,1.0); // PREENCH AQUI o espectro da fonte de luz

    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2,0.2,0.2); // PREENCHA AQUI o espectro da luz ambiente

    vec3 lambert_diffuse_term = vec3(1.0);

    if ( object_id == SPHERE )
    {
        Ks = vec3(0.0,0.0,0.0);
        Ka = vec3(0.0, 0.0, 0.0);
        q = 1.0;

        int sphere_radius = 5;
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        vec4 pl = bbox_center + sphere_radius*(normalize(position_model - bbox_center));
        vec4 vector_p = pl - bbox_center;
        float px = vector_p.x;
        float py = vector_p.y;
        float pz = vector_p.z;

        float theta = atan(px,pz);
        float phi = asin(py/sphere_radius);
        U = (theta + M_PI)/(2*M_PI);
        V = (phi + M_PI_2) / M_PI;
        Kd = texture(TextureImage11, vec2(U,V)).rgb;;
    }
    else if ( object_id == BUNNY )
    {
        Ks = vec3(0.8, 0.8, 0.8);
        Ka = vec3(0.0, 0.0, 0.0);
        q = 32.0;

        int sphere_radius = 5;
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        vec4 pl = bbox_center + sphere_radius*(normalize(position_model - bbox_center));
        vec4 vector_p = pl - bbox_center;
        float px = vector_p.x;
        float py = vector_p.y;
        float pz = vector_p.z;

        float theta = atan(px,pz);
        float phi = asin(py/sphere_radius);
        U = (theta + M_PI)/(2*M_PI);
        V = (phi + M_PI_2) / M_PI;
       Kd = texture(TextureImage8, vec2(U,V)).rgb;;
    }
    else if ( object_id == PLANE )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;
        float repeatedU = mod(U * 40.0,1.0);
        float repeatedV = mod(V * 40.0,1.0);

        vec3 Kd4 = texture(TextureImage4, vec2(repeatedU,repeatedV)).rgb;
       Kd = Kd4;
    }
    else if ( object_id == ROCK )
    {
        // Propriedades espectrais da pedra
        Ks = vec3(0.0, 0.0, 0.0);
        Ka = vec3(0.0,0.0,0.0);
        q = 1.0;

        int sphere_radius = 5;
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        vec4 pl = bbox_center + sphere_radius*(normalize(position_model - bbox_center));
        vec4 vector_p = pl - bbox_center;
        float px = vector_p.x;
        float py = vector_p.y;
        float pz = vector_p.z;

        float theta = atan(px,pz);
        float phi = asin(py/sphere_radius);
        U = (theta + M_PI)/(2*M_PI);
        V = (phi + M_PI_2) / M_PI;
        U = texcoords.x;
        V = texcoords.y;
        vec3 Kd7 = texture(TextureImage7, vec2(U,V)).rgb;
        Kd = Kd7;

    }
    else if ( object_id == FERN )
    {
        // Propriedades espectrais do fern
        Ks = vec3(0.0, 0.0, 0.0);
        Ka = vec3(0.0, 0.0, 0.0);
        q = 15.0;

        U = texcoords.x;
        V = texcoords.y;
        Kd = texture(TextureImage0, vec2(U,V)).rgb;;
    }
    else if ( object_id == BUILD1 )
    {
        U = texcoords.x;
        V = texcoords.y;
        // Propriedades espectrais do fern
        Ks = vec3(0.0, 0.0, 0.0);
        Ka = vec3(0.0, 0.0, 0.0);
        q = 15.0;
        vec3 Kd3 = texture(TextureImage3, vec2(U, V)).rgb;
        Kd = Kd3;
    }
    else if ( object_id == FERRIS )
    {
        U = texcoords.x;
        V = texcoords.y;
        // Propriedades espectrais do fern
        Ks = vec3(0.9, 0.9, 0.9);
        Ka = vec3(0.1, 0.1, 0.1);
        q = 30.0;

        vec3 Kd3 = texture(TextureImage3, vec2(U,V)).rgb;
        Kd = Kd3;
        Kd = vec3(0.8, 0.8, 0.8);
    } else if (object_id == FIREWORK) {
        Ks = vec3(0.0,0.0,0.0);
        Ka = vec3(0.0, 0.0, 0.0);
        q = 1.0;

        int sphere_radius = 5;
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        vec4 pl = bbox_center + sphere_radius*(normalize(position_model - bbox_center));
        vec4 vector_p = pl - bbox_center;
        float px = vector_p.x;
        float py = vector_p.y;
        float pz = vector_p.z;

        float theta = atan(px,pz);
        float phi = asin(py/sphere_radius);
        U = (theta + M_PI)/(2*M_PI);
        V = (phi + M_PI_2) / M_PI;
        Kd = texture(TextureImage13, vec2(U,V)).rgb;;
    }

    float aaa = object_id;
    // Kd = vec3(aaa / 10.0, 0, 0);

    vec3 ambient_term = Ka*Ia;
    vec3 phong_specular_term  = Ks*I*pow(max(0,dot(r,v)),q);
    if ( object_id == SPHERE ) {
        lambert_diffuse_term =Kd*I;
    } else {
        lambert_diffuse_term =Kd*I*max(0, dot(n, l));
    }
    color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;

    color.a = 1;

    // Cor final com correção gamma, considerando monitor sRGB.
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
}

