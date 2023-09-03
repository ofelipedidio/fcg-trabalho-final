#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <iostream>
#include <map>
#include <ostream>
#include <stack>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>
#include <vector>

// Headers das bibliotecas OpenGL
#include "glad/glad.h"   // Criação de contexto OpenGL 3.3
#include "GLFW/glfw3.h"  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/type_ptr.hpp"

// Headers locais, definidos na pasta "include/"
#include "renderer.h"
#include "utils.h"
#include "matrices.h"

#include "random.h"

#include "collisions.h"
#include "emitter.h"

#define print_vec4(v) "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")"
#define debug_var(var) std::cout << #var " = " << var << std::endl;


GLint model_uniform           ; // Variável da matriz "model"
GLint view_uniform            ; // Variável da matriz "view" em shader_vertex.glsl
GLint projection_uniform      ; // Variável da matriz "projection" em shader_vertex.glsl
GLint render_as_black_uniform ; // Variável booleana em shader_vertex.glsl


// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void DrawCube(GLint render_as_black_uniform); // Desenha um cubo
GLuint BuildTriangles(); // Constrói triângulos para renderização
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
GLuint loadVertexShader(const char* filename);   // Carrega um vertex shader
GLuint loadFragmentShader(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject {
    const char*  name;
    void*        first_index;
    int          num_indices;
    GLenum       rendering_mode;
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTriangles() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<const char*, SceneObject> g_VirtualScene;

struct Camera {
    bool usePerspectiveProjection = true;

    // Perspective parameters
    float theta = 0.0f;
    float phi = 0.0f;
    float distance = 50.0f;
    float field_of_view = 3.141592 / 3.0f;

    // General parameters
    float nearPlane = -0.1f;
    float farPlane = -100000.0f;
    float screenRatio = 1.0f;

    float width = 800.0f;
    float height = 800.0f;

    bool isFreeCamera = false;
    glm::vec4 position = glm::vec4(0,0,0,1);
    glm::vec4 upVector = glm::vec4(0, 1, 0, 0);

    glm::vec4 viewVector = glm::vec4(0, 0, 1, 0);
    glm::vec4 rightVector = glm::vec4(1, 0, 0, 0);
    glm::vec4 topVector = glm::vec4(0, 1, 0, 0);

    void computeMatrices(glm::mat4 &view, glm::mat4 &projection) {
        // View
        {
            if (!isFreeCamera) {
                float r = distance;
                float y = r * std::sin(phi);
                float z = r * std::cos(phi) * std::cos(theta);
                float x = r * std::cos(phi) * std::sin(theta);
                position = glm::vec4(x, y, z, 1.0f);
                glm::vec4 lookAtPoint = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                viewVector = lookAtPoint - position;
                view = Matrix_Camera_View(position, viewVector, upVector);
                glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view));
            }
            view = Matrix_Camera_View(position, viewVector, upVector);
            glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        }

        // Projection
        {
            if (usePerspectiveProjection) {
                projection = Matrix_Perspective(field_of_view, screenRatio, nearPlane, farPlane);
            } else {
                float t = 1.5f*distance/2.5f;
                float b = -t;
                float r = t*screenRatio;
                float l = -r;
                projection = Matrix_Orthographic(l, r, b, t, nearPlane, farPlane);
            }
            glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
        }
    }
};

Camera camera;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;

GLFWwindow* setup() {
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(ErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow(800, 800, "INF01047 - 297033 - Rafael Lacerda Busatta", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetWindowSize(window, 800, 800);

    glfwMakeContextCurrent(window);

    return window;
}

void displaySystemInfo() {
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);
}

#include "particle.h"
Particle::ParticleEmitter emitter(10000);

#include "emitter.h"
Emitter::ParticleEmitter *e1;

int main() {
    camera.usePerspectiveProjection = true;
    camera.phi = 0.0f;
    camera.theta = 0.0f;
    camera.distance = 50.0f;
    camera.field_of_view = 3.141592f / 3.0f;
    camera.nearPlane = -0.1f;
    camera.farPlane = -100000.0f;

    GLFWwindow *window = setup();
    displaySystemInfo();
    LoadShadersFromFiles();

    GLuint vertex_array_object_id = BuildTriangles();

    TextRendering_Init();

    model_uniform           = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    view_uniform            = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    projection_uniform      = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    render_as_black_uniform = glGetUniformLocation(g_GpuProgramID, "render_as_black"); // Variável booleana em shader_vertex.glsl

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    float previousTime = glfwGetTime();
    Random::Init();

    Emitter::ParticleProprieties emitterProprieties;
    emitterProprieties.xa = 0.0f;
    emitterProprieties.ya = -9.8f;
    emitterProprieties.za = 0.0f;
    emitterProprieties.rotationSpeedX = 10.0f;
    emitterProprieties.rotationSpeedY = 10.0f;
    emitterProprieties.rotationSpeedZ = 1.0f;
    emitterProprieties.initialSize = 1.0f;
    emitterProprieties.finalSize = 0.0f;
    emitterProprieties.duration = 4.0f;
    RenderObject ro((void*)g_VirtualScene["cube_faces"].first_index, g_VirtualScene["cube_faces"].num_indices,  g_VirtualScene["cube_faces"].rendering_mode);
    emitterProprieties.object = ro;

    Emitter::ParticleEmitter pa(10000, emitterProprieties);
    e1 = &pa;

    while (!glfwWindowShouldClose(window))
    {
        e1->emit(0, 0, 0, 10, 0, 0);

        double currentTime = glfwGetTime();
        float dt = currentTime - previousTime;
        previousTime = currentTime;

        // Clear screen
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Usar shader
        glUseProgram(g_GpuProgramID);

        // Usar objeto (cubo)
        glBindVertexArray(vertex_array_object_id);

        // Criar um Renderer
        Renderer renderer(g_GpuProgramID);

        glm::mat4 view;
        glm::mat4 projection;
        camera.computeMatrices(view, projection);
        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

        emitter.onUpdate(dt);
        emitter.onRender(renderer);

        pa.onUpdate(dt);
        pa.onRender(renderer);
        
        // Overlay text
        {
            glm::mat4 model = Matrix_Identity();
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glLineWidth(10.0f);
            glUniform1i(render_as_black_uniform, false);
            glDrawElements(
                    g_VirtualScene["axes"].rendering_mode,
                    g_VirtualScene["axes"].num_indices,
                    GL_UNSIGNED_INT,
                    (void*)g_VirtualScene["axes"].first_index
                    );
            glBindVertexArray(0);
            TextRendering_ShowProjection(window);
            TextRendering_ShowFramesPerSecond(window);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void DrawCube(GLint render_as_black_uniform) {
    glUniform1i(render_as_black_uniform, false);
    // Cube
    {
        glDrawElements(
                g_VirtualScene["cube_faces"].rendering_mode, // Veja slides 182-188 do documento Aula_04_Modelagem_Geometrica_3D.pdf
                g_VirtualScene["cube_faces"].num_indices,    //
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["cube_faces"].first_index
                );
    }
    return;
    // Axes
    {
        glLineWidth(4.0f);
        glDrawElements(
                g_VirtualScene["axes"].rendering_mode,
                g_VirtualScene["axes"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["axes"].first_index
                );
    }
    // Edges
    {
        glUniform1i(render_as_black_uniform, true);
        glDrawElements(
                g_VirtualScene["cube_edges"].rendering_mode,
                g_VirtualScene["cube_edges"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["cube_edges"].first_index
                );
    }
}

GLuint BuildTriangles() {
    GLfloat model_coefficients[] = {
        // Vértices de um cubo
        //    X      Y     Z     W
        -0.5f,  0.0f,  0.5f, 1.0f, // posição do vértice 0
        -0.5f, -1.0f,  0.5f, 1.0f, // posição do vértice 1
        0.5f, -1.0f,  0.5f, 1.0f, // posição do vértice 2
        0.5f,  0.0f,  0.5f, 1.0f, // posição do vértice 3
        -0.5f,  0.0f, -0.5f, 1.0f, // posição do vértice 4
        -0.5f, -1.0f, -0.5f, 1.0f, // posição do vértice 5
        0.5f, -1.0f, -0.5f, 1.0f, // posição do vértice 6
        0.5f,  0.0f, -0.5f, 1.0f, // posição do vértice 7
                                  // Vértices para desenhar o eixo X
                                  //    X      Y     Z     W
        0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 8
        1.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 9
                                  // Vértices para desenhar o eixo Y
                                  //    X      Y     Z     W
        0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 10
        0.0f,  1.0f,  0.0f, 1.0f, // posição do vértice 11
                                  // Vértices para desenhar o eixo Z
                                  //    X      Y     Z     W
        0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 12
        0.0f,  0.0f,  1.0f, 1.0f, // posição do vértice 13
    };
    // Setup (just so things get to exist)
    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);
    // Transfer data
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(model_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(model_coefficients), model_coefficients);
    // Setup shader stuff
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLfloat color_coefficients[] = {
        // Cores dos vértices do cubo
        //  R     G     B     A
        1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 0
        1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 1
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 2
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 3
        1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 4
        1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 5
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 6
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 7
                                // Cores para desenhar o eixo X
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 8
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 9
                                // Cores para desenhar o eixo Y
        0.0f, 1.0f, 0.0f, 1.0f, // cor do vértice 10
        0.0f, 1.0f, 0.0f, 1.0f, // cor do vértice 11
                                // Cores para desenhar o eixo Z
        0.0f, 0.0f, 1.0f, 1.0f, // cor do vértice 12
        0.0f, 0.0f, 1.0f, 1.0f, // cor do vértice 13
    };
    GLuint VBO_color_coefficients_id;
    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(color_coefficients), color_coefficients);
    location = 1; // "(location = 1)" em "shader_vertex.glsl"
    number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLuint indices[] = {
        // Definimos os índices dos vértices que definem as FACES de um cubo
        // através de 12 triângulos que serão desenhados com o modo de renderização
        // GL_TRIANGLES.
        0, 1, 2, // triângulo 1
        7, 6, 5, // triângulo 2
        3, 2, 6, // triângulo 3
        4, 0, 3, // triângulo 4
        4, 5, 1, // triângulo 5
        1, 5, 6, // triângulo 6
        0, 2, 3, // triângulo 7
        7, 5, 4, // triângulo 8
        3, 6, 7, // triângulo 9
        4, 3, 7, // triângulo 10
        4, 1, 0, // triângulo 11
        1, 6, 2, // triângulo 12
                 // Definimos os índices dos vértices que definem as ARESTAS de um cubo
                 // através de 12 linhas que serão desenhadas com o modo de renderização
                 // GL_LINES.
        0, 1, // linha 1
        1, 2, // linha 2
        2, 3, // linha 3
        3, 0, // linha 4
        0, 4, // linha 5
        4, 7, // linha 6
        7, 6, // linha 7
        6, 2, // linha 8
        6, 5, // linha 9
        5, 4, // linha 10
        5, 1, // linha 11
        7, 3, // linha 12
              // Definimos os índices dos vértices que definem as linhas dos eixos X, Y,
              // Z, que serão desenhados com o modo GL_LINES.
        8 , 9 , // linha 1
        10, 11, // linha 2
        12, 13  // linha 3
    };
    SceneObject cube_faces;
    cube_faces.name           = "Cubo (faces coloridas)";
    cube_faces.first_index    = (void*)0; // Primeiro índice está em indices[0]
    cube_faces.num_indices    = 36;       // Último índice está em indices[35]; total de 36 índices.
    cube_faces.rendering_mode = GL_TRIANGLES; // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
    g_VirtualScene["cube_faces"] = cube_faces;
    SceneObject cube_edges;
    cube_edges.name           = "Cubo (arestas pretas)";
    cube_edges.first_index    = (void*)(36*sizeof(GLuint)); // Primeiro índice está em indices[36]
    cube_edges.num_indices    = 24; // Último índice está em indices[59]; total de 24 índices.
    cube_edges.rendering_mode = GL_LINES; // Índices correspondem ao tipo de rasterização GL_LINES.
    // Adicionamos o objeto criado acima na nossa cena virtual (g_VirtualScene).
    g_VirtualScene["cube_edges"] = cube_edges;
    // Criamos um terceiro objeto virtual (SceneObject) que se refere aos eixos XYZ.
    SceneObject axes;
    axes.name           = "Eixos XYZ";
    axes.first_index    = (void*)(60*sizeof(GLuint)); // Primeiro índice está em indices[60]
    axes.num_indices    = 6; // Último índice está em indices[65]; total de 6 índices.
    axes.rendering_mode = GL_LINES; // Índices correspondem ao tipo de rasterização GL_LINES.
    g_VirtualScene["axes"] = axes;
    // Criamos um buffer OpenGL para armazenar os índices acima
    GLuint indices_id;
    glGenBuffers(1, &indices_id);
    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    // Alocamos memória para o buffer.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);
    // Copiamos os valores do array indices[] para dentro do buffer.
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
    // NÃO faça a chamada abaixo! Diferente de um VBO (GL_ARRAY_BUFFER), um
    // array de índices (GL_ELEMENT_ARRAY_BUFFER) não pode ser "desligado",
    // caso contrário o VAO irá perder a informação sobre os índices.
    //
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //
    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
    // Retornamos o ID do VAO. Isso é tudo que será necessário para renderizar
    // os triângulos definidos acima. Veja a chamada glDrawElements() em main().
    return vertex_array_object_id;
}

GLuint loadVertexShader(const char* filename) {
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    LoadShader(filename, vertex_shader_id);
    return vertex_shader_id;
}

GLuint loadFragmentShader(const char* filename) {
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    LoadShader(filename, fragment_shader_id);
    return fragment_shader_id;
}

void LoadShader(const char* filename, GLuint shader_id) {
    // Le o arquivo do shader
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );
    // Compila o shader
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);
    glCompileShader(shader_id);
    // Verifica se compilou
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);
    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);
    if ( log_length != 0 ) {
        std::string  output;
        // Loca o erro
        if ( !compiled_ok ) {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        } else {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        fprintf(stderr, "%s", output.c_str());
    }
    delete [] log;
}

void LoadShadersFromFiles() {
    GLuint vertex_shader_id = loadVertexShader("../assets/shader_vertex.glsl");
    GLuint fragment_shader_id = loadFragmentShader("../assets/shader_fragment.glsl");
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);
}

GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id) {
    // Create program
    GLuint program_id = glCreateProgram();
    // Link shaders
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);
    // Check ok
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);
    if (linked_ok == GL_FALSE) {
        // Log error
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
        GLchar* log = new GLchar[log_length];
        glGetProgramInfoLog(program_id, log_length, &log_length, log);
        std::string output;
        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";
        delete [] log;
        fprintf(stderr, "%s", output.c_str());
    }
    return program_id;
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    camera.screenRatio = ((float)width) / ((float) height);
    camera.width = (float) width;
    camera.height = (float) height;
}

bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false;
bool g_MiddleMouseButtonPressed = false;
double g_LastCursorPosX, g_LastCursorPosY;

void spawnParticleAt(glm::vec4 position) {
    Particle::ParticleProprieties particle;

    // Configurar o particleProprieties
    particle.x = position.x;
    particle.y = position.y;
    particle.z = position.z;
    particle.xs = 0;
    particle.ys = 0;
    particle.zs = 0;
    particle.xa = 0;
    particle.ya = 0;
    particle.za = 0;
    particle.rotationX = 0;
    particle.rotationY = 0;
    particle.rotationZ = 0;
    particle.rotationSpeedX = 10000;
    particle.rotationSpeedY = 10000;
    particle.rotationSpeedZ = 10000;
    particle.size = 0.3f;
    particle.sizeChange = 0.0f;
    particle.duration = 100000.0f;

    // Carregar o objeto da particula (nesse caso o cubo)
    RenderObject ro((void*)g_VirtualScene["cube_faces"].first_index, g_VirtualScene["cube_faces"].num_indices,  g_VirtualScene["cube_faces"].rendering_mode);
    particle.object = ro;

    emitter.emit(particle);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;

        glm::vec4 camera_position_c;
        glm::vec4 camera_view_vector;
        glm::vec4 camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
        {
            float r = camera.distance;
            float y = r * std::sin(camera.phi);
            float z = r * std::cos(camera.phi) * std::cos(camera.theta);
            float x = r * std::cos(camera.phi) * std::sin(camera.theta);
            camera_position_c = glm::vec4(x, y, z, 1.0f);
            glm::vec4 camera_lookat_l = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            camera_view_vector = camera_lookat_l - camera_position_c;
        }

        /*
        camera_view_vector /= -norm(camera_view_vector);

        float t = std::fabs(camera.nearPlane) * tanf(camera.field_of_view / 2.0f);
        float b = -t;
        float r = t * camera.screenRatio;
        float l = -r;

        glm::vec4 point_right = crossproduct(camera_up_vector, camera_view_vector);
        glm::vec4 point_up = crossproduct(camera_view_vector, point_right);

        camera_view_vector *= camera.nearPlane;

        point_right *= std::abs(r) * 1.333333f;
        point_up *= std::abs(t) * 1.333333f;

        glm::vec4 bl = camera_position_c - (point_right) - (point_up) + (camera_view_vector);
        glm::vec4 tr = camera_position_c + (point_right) + (point_up) + (camera_view_vector);

        glm::mat4 view;
        glm::mat4 projection;
        camera.computeMatrices(view, projection);
        view = glm::inverse(view);

        //spawnParticleAt(bl);
        //spawnParticleAt(tr);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float xFactor = (float) ((mouseX / camera.width) * 2.0f - 1.0f);
        float yFactor = (float) ((mouseY / camera.height) * -2.0f + 1.0f);

        glm::vec4 spacePosition = camera_position_c + (xFactor*point_right) + (yFactor*point_up) + (camera_view_vector);
        // spawnParticleAt(spacePosition);

        glm::vec4 a = spacePosition - camera_position_c;
         */

        collision::Plane floor = {{0, 0 ,0}, {0, 1, 0}};
        collision::Ray ray = {{camera_position_c.x, camera_position_c.y, camera_position_c.z}, {camera_view_vector.x, camera_view_vector.y, camera_view_vector.z}};
        // collision::Ray ray = {{camera_position_c.x, camera_position_c.y, camera_position_c.z}, {camera_view_vector.x, camera_view_vector.y, camera_view_vector.z}};

        float time = collision::collide(floor, ray);

        collision::Point p = ray.at(time);
        glm::vec4 pp = glm::vec4(p.x, p.y, p.z, 1.0f);
        spawnParticleAt(pp);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        g_LeftMouseButtonPressed = false;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        g_RightMouseButtonPressed = false;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
        g_MiddleMouseButtonPressed = false;
    }
}

void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    float dx = xpos - g_LastCursorPosX;
    float dy = ypos - g_LastCursorPosY;
    if (g_LeftMouseButtonPressed) {
        // Atualizamos parâmetros da câmera com os deslocamentos
        camera.theta -= 0.01f*dx;
        camera.phi   += 0.01f*dy;
        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;
        if (camera.phi > phimax)
            camera.phi = phimax;
        if (camera.phi < phimin)
            camera.phi = phimin;
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
    if (g_RightMouseButtonPressed) {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
    }
    if (g_MiddleMouseButtonPressed) {
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.distance-= 1.0f*yoffset;
    const float verySmallNumber = std::numeric_limits<float>::epsilon();
    if (camera.distance < verySmallNumber) {
        camera.distance = verySmallNumber;
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
    } else if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
    } else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
    } else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    } else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        camera.usePerspectiveProjection = true;
    } else if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        camera.usePerspectiveProjection = false;
    } else if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        g_ShowInfoText = !g_ShowInfoText;
    } else if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        Particle::ParticleProprieties particle;

        // Configurar o particleProprieties
        particle.x = 0;
        particle.y = 0;
        particle.z = 0;
        particle.xs = 0;
        particle.ys = 0;
        particle.zs = 0;
        particle.xa = 0;
        particle.ya = 0;
        particle.za = 0;
        particle.rotationX = 0;
        particle.rotationY = 0;
        particle.rotationZ = 0;
        particle.rotationSpeedX = 0;
        particle.rotationSpeedY = 0;
        particle.rotationSpeedZ = 0;
        particle.size = 1.0f;
        particle.sizeChange = -1.0f;
        particle.duration = 8.0f;

        // Carregar o objeto da particula (nesse caso o cubo)
        RenderObject ro((void*)g_VirtualScene["cube_faces"].first_index, g_VirtualScene["cube_faces"].num_indices,  g_VirtualScene["cube_faces"].rendering_mode);
        particle.object = ro;

#define PI 3.141592
#define SIDES 5
#define VSIDES 5

        {
            float speed = 1.0f;
            float r = speed;
            for (float i = 0.0f; i < 2.0f*PI; i += (2.0f*PI)/SIDES) {
                for (float j = -PI / 2.0f; j < PI * 2.0f ; j += PI / VSIDES) {
                    float y = r*sin(i);
                    float z = r*cos(i)*cos(j);
                    float x = r*cos(i)*sin(j);

                    particle.x = 0;
                    particle.y = 30.0f;
                    particle.z = 0;
                    particle.xs = x;
                    particle.ys = y;
                    particle.zs = z;

                    particle.xa = -x * 0.05;
                    particle.ya = -y * 0.05;
                    particle.za = -z * 0.05;

                    particle.ys += 1.0f;
                    particle.ya -= 1.0f;

                    particle.xs *= 20;
                    particle.ys *= 20;
                    particle.zs *= 20;
                    particle.xa *= 20;
                    particle.ya *= 20;
                    particle.za *= 20;

                    particle.xs += (Random::Float() * 2.0f - 1.0f) * 0.02;
                    particle.ys += (Random::Float() * 2.0f - 1.0f) * 0.02;
                    particle.zs += (Random::Float() * 2.0f - 1.0f) * 0.02;

                    particle.rotationSpeedX += (Random::Float() * 2.0f - 1.0f) * 0.2;
                    particle.rotationSpeedY += (Random::Float() * 2.0f - 1.0f) * 0.2;
                    particle.rotationSpeedZ += (Random::Float() * 2.0f - 1.0f) * 0.2;

                    particle.duration = 6.0f + 0.5f * (Random::Float() * 2.0f - 1.0f);

                    for (int k = 0; k < 20; k++) {
                        particle.size = (20.0-k)/20.0f;
                        particle.sizeChange = -particle.size;
                        emitter.emitIn(particle, k/20.0f);
                    }
                }
            }
        }

    }
}

void ErrorCallback(int error, const char* description) {
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    if ( camera.usePerspectiveProjection )
        TextRendering_PrintString(window, "Perspective", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
    else
        TextRendering_PrintString(window, "Orthographic", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :

