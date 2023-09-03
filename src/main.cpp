//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   PROJETO FINAL
//
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
#include "glad/glad.h"  // Criação de contexto OpenGL 3.3
#include "GLFW/glfw3.h" // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "tiny_obj_loader.h"

// Headers locais, definidos na pasta "include/"
#include "renderer.h"
#include "utils.h"
#include "matrices.h"

#include "random.h"

#include "collisions.h"
#include "emitter.h"

#define print_vec4(v) "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")"
#define debug_var(var) std::cout << #var " = " << var << std::endl;

GLint model_uniform;           // Variável da matriz "model"
GLint view_uniform;            // Variável da matriz "view" em shader_vertex.glsl
GLint projection_uniform;      // Variável da matriz "projection" em shader_vertex.glsl
GLint render_as_black_uniform; // Variável booleana em shader_vertex.glsl

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char *filename, const char *basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, então setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diretório dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i + 1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                        filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel *);                         // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel *model);                                        // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles();                                                 // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void DrawVirtualObject(const char *object_name);                             // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char *filename);                              // Carrega um vertex shader
GLuint LoadShader_Fragment(const char *filename);                            // Carrega um fragment shader
void LoadShader(const char *filename, GLuint shader_id);                     // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel *);                                          // Função para debugging

void DrawCube(GLint render_as_black_uniform); // Desenha um cubo
GLuint BuildTriangles();

// *********************************************************************************************************************
GLuint loadVertexShader(const char *filename); // Carrega um vertex shader
GLuint loadFragmentShader(const char *filename);
// *********************************************************************************************************************

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow *window);
float TextRendering_CharWidth(GLFWwindow *window);
void TextRendering_PrintString(GLFWwindow *window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow *window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow *window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow *window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow *window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow *window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow *window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowEulerAngles(GLFWwindow *window);
void TextRendering_ShowProjection(GLFWwindow *window);
void TextRendering_ShowFramesPerSecond(GLFWwindow *window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
void ErrorCallback(int error, const char *description);
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);
void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string name;      // Nome do objeto
    size_t first_index;    // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t num_indices;    // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
};

// *********************************************************************************************************************
struct SceneObjectOBJ
{
    std::string name;              // Nome do objeto
    size_t first_index;            // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t num_indices;            // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum rendering_mode;         // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
};
// *********************************************************************************************************************

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;
std::map<std::string, SceneObjectOBJ> g_VirtualSceneOBJ;

struct Camera
{
    bool usePerspectiveProjection = true;
    // Pilha que guardará as matrizes de modelagem.
    std::stack<glm::mat4> g_MatrixStack;

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
    glm::vec4 position = glm::vec4(0, 0, 0, 1);
    glm::vec4 upVector = glm::vec4(0, 1, 0, 0);

    glm::vec4 viewVector = glm::vec4(0, 0, 1, 0);
    glm::vec4 rightVector = glm::vec4(1, 0, 0, 0);
    glm::vec4 topVector = glm::vec4(0, 1, 0, 0);

    void computeMatrices(glm::mat4 &view, glm::mat4 &projection)
    {
        // View
        {
            if (!isFreeCamera)
            {
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
            if (usePerspectiveProjection)
            {
                projection = Matrix_Perspective(field_of_view, screenRatio, nearPlane, farPlane);
            }
            else
            {
                float t = 1.5f * distance / 2.5f;
                float b = -t;
                float r = t * screenRatio;
                float l = -r;
                projection = Matrix_Orthographic(l, r, b, t, nearPlane, farPlane);
            }
            glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));
        }
    }
};

Camera camera;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLFWwindow *setup()
{
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

    GLFWwindow *window;
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

void displaySystemInfo()
{
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *glversion = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);
}

#include "particle.h"
Particle::ParticleEmitter emitter(10000);

#include "emitter.h"
Emitter::ParticleEmitter *e1;

int main()
{
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

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel model();
    ObjModel spheremodel("../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);

    ObjModel bunnymodel("../data/bunny.obj");
    ComputeNormals(&bunnymodel);
    BuildTrianglesAndAddToVirtualScene(&bunnymodel);

    ObjModel planemodel("../data/tree_stump_01_4k.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    BuildTrianglesAndAddToVirtualScene(&model);

    GLuint vertex_array_object_id = BuildTriangles();

    TextRendering_Init();

    model_uniform = glGetUniformLocation(g_GpuProgramID, "model");                     // Variável da matriz "model"
    view_uniform = glGetUniformLocation(g_GpuProgramID, "view");                       // Variável da matriz "view" em shader_vertex.glsl
    projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection");           // Variável da matriz "projection" em shader_vertex.glsl
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
    RenderObject ro((void *)g_VirtualScene["cube_faces"].first_index, g_VirtualScene["cube_faces"].num_indices, g_VirtualScene["cube_faces"].rendering_mode);
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
        {
            float r = g_CameraDistance;
            float y = r * sin(g_CameraPhi);
            float z = r * cos(g_CameraPhi) * cos(g_CameraTheta);
            float x = r * cos(g_CameraPhi) * sin(g_CameraTheta);
            glm::vec4 camera_position_c = glm::vec4(x, y, z, 1.0f);
            glm::vec4 camera_lookat_l = glm::vec4(0.0f, 100.0f, 0.0f, 1.0f);
            glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c;
            glm::vec4 camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
        }

        glm::mat4 projection;
        camera.computeMatrices(view, projection);
        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

        emitter.onUpdate(dt);
        emitter.onRender(renderer);

        glUniformMatrix4fv(g_view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

#define SPHERE 0
#define BUNNY 1
#define PLANE 2

        // Desenhamos o modelo da esfera
        model = Matrix_Translate(-1.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, SPHERE);
        DrawVirtualObject("the_sphere");

        // Desenhamos o modelo do coelho
        model = Matrix_Translate(1.0f, 0.0f, 0.0f) * Matrix_Rotate_Z(g_AngleZ) * Matrix_Rotate_Y(g_AngleY) * Matrix_Rotate_X(g_AngleX);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, BUNNY);
        DrawVirtualObject("the_bunny");

        model = Matrix_Translate(10.0f, 0.0f, 0.0f) * Matrix_Rotate_Z(g_AngleZ) * Matrix_Rotate_Y(g_AngleY) * Matrix_Rotate_X(g_AngleX);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, BUNNY);
        DrawVirtualObject("the_bunny");

        // Desenhamos o modelo do plano
        model = Matrix_Translate(0.0f, -1.0f, 0.0f) * Matrix_Scale(1.01f, 1.01f, 1.01f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, PLANE);
        DrawVirtualObject("tree_stump_01");

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
                (void *)g_VirtualScene["axes"].first_index);
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

// *********************************************************************************************************************
void DrawCube(GLint render_as_black_uniform)
{
    glUniform1i(render_as_black_uniform, false);
    // Cube
    {
        glDrawElements(
            g_VirtualScene["cube_faces"].rendering_mode, // Veja slides 182-188 do documento Aula_04_Modelagem_Geometrica_3D.pdf
            g_VirtualScene["cube_faces"].num_indices,    //
            GL_UNSIGNED_INT,
            (void *)g_VirtualScene["cube_faces"].first_index);
    }
    return;
    // Axes
    {
        glLineWidth(4.0f);
        glDrawElements(
            g_VirtualScene["axes"].rendering_mode,
            g_VirtualScene["axes"].num_indices,
            GL_UNSIGNED_INT,
            (void *)g_VirtualScene["axes"].first_index);
    }
    // Edges
    {
        glUniform1i(render_as_black_uniform, true);
        glDrawElements(
            g_VirtualScene["cube_edges"].rendering_mode,
            g_VirtualScene["cube_edges"].num_indices,
            GL_UNSIGNED_INT,
            (void *)g_VirtualScene["cube_edges"].first_index);
    }
}

GLuint BuildTriangles()
{
    GLfloat model_coefficients[] = {
        // Vértices de um cubo
        //    X      Y     Z     W
        -0.5f, 0.0f, 0.5f, 1.0f,   // posição do vértice 0
        -0.5f, -1.0f, 0.5f, 1.0f,  // posição do vértice 1
        0.5f, -1.0f, 0.5f, 1.0f,   // posição do vértice 2
        0.5f, 0.0f, 0.5f, 1.0f,    // posição do vértice 3
        -0.5f, 0.0f, -0.5f, 1.0f,  // posição do vértice 4
        -0.5f, -1.0f, -0.5f, 1.0f, // posição do vértice 5
        0.5f, -1.0f, -0.5f, 1.0f,  // posição do vértice 6
        0.5f, 0.0f, -0.5f, 1.0f,   // posição do vértice 7
                                   // Vértices para desenhar o eixo X
                                   //    X      Y     Z     W
        0.0f, 0.0f, 0.0f, 1.0f,    // posição do vértice 8
        1.0f, 0.0f, 0.0f, 1.0f,    // posição do vértice 9
                                   // Vértices para desenhar o eixo Y
                                   //    X      Y     Z     W
        0.0f, 0.0f, 0.0f, 1.0f,    // posição do vértice 10
        0.0f, 1.0f, 0.0f, 1.0f,    // posição do vértice 11
                                   // Vértices para desenhar o eixo Z
                                   //    X      Y     Z     W
        0.0f, 0.0f, 0.0f, 1.0f,    // posição do vértice 12
        0.0f, 0.0f, 1.0f, 1.0f,    // posição do vértice 13
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
    GLuint location = 0;            // "(location = 0)" em "shader_vertex.glsl"
    GLint number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
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
    location = 1;             // "(location = 1)" em "shader_vertex.glsl"
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
        0, 1,    // linha 1
        1, 2,    // linha 2
        2, 3,    // linha 3
        3, 0,    // linha 4
        0, 4,    // linha 5
        4, 7,    // linha 6
        7, 6,    // linha 7
        6, 2,    // linha 8
        6, 5,    // linha 9
        5, 4,    // linha 10
        5, 1,    // linha 11
        7, 3,    // linha 12
                 // Definimos os índices dos vértices que definem as linhas dos eixos X, Y,
                 // Z, que serão desenhados com o modo GL_LINES.
        8, 9,    // linha 1
        10, 11,  // linha 2
        12, 13   // linha 3
    };
    SceneObject cube_faces;
    cube_faces.name = "Cubo (faces coloridas)";
    cube_faces.first_index = (void *)0;       // Primeiro índice está em indices[0]
    cube_faces.num_indices = 36;              // Último índice está em indices[35]; total de 36 índices.
    cube_faces.rendering_mode = GL_TRIANGLES; // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
    g_VirtualScene["cube_faces"] = cube_faces;
    SceneObject cube_edges;
    cube_edges.name = "Cubo (arestas pretas)";
    cube_edges.first_index = (void *)(36 * sizeof(GLuint)); // Primeiro índice está em indices[36]
    cube_edges.num_indices = 24;                            // Último índice está em indices[59]; total de 24 índices.
    cube_edges.rendering_mode = GL_LINES;                   // Índices correspondem ao tipo de rasterização GL_LINES.
    // Adicionamos o objeto criado acima na nossa cena virtual (g_VirtualScene).
    g_VirtualScene["cube_edges"] = cube_edges;
    // Criamos um terceiro objeto virtual (SceneObject) que se refere aos eixos XYZ.
    SceneObject axes;
    axes.name = "Eixos XYZ";
    axes.first_index = (void *)(60 * sizeof(GLuint)); // Primeiro índice está em indices[60]
    axes.num_indices = 6;                             // Último índice está em indices[65]; total de 6 índices.
    axes.rendering_mode = GL_LINES;                   // Índices correspondem ao tipo de rasterização GL_LINES.
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

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char *object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void *)(g_VirtualScene[object_name].first_index * sizeof(GLuint)));

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../assets/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../assets/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if (g_GpuProgramID != 0)
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform = glGetUniformLocation(g_GpuProgramID, "model");           // Variável da matriz "model"
    g_view_uniform = glGetUniformLocation(g_GpuProgramID, "view");             // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform = glGetUniformLocation(g_GpuProgramID, "object_id");   // Variável "object_id" em shader_fragment.glsl
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel *model)
{
    if (!model->attrib.normals.empty())
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4 vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3 * triangle + vertex];
                const float vx = model->attrib.vertices[3 * idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3 * idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3 * idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx, vy, vz, 1.0);
            }

            const glm::vec4 a = vertices[0];
            const glm::vec4 b = vertices[1];
            const glm::vec4 c = vertices[2];

            // PREENCHA AQUI o cálculo da normal de um triângulo cujos vértices
            // estão nos pontos "a", "b", e "c", definidos no sentido anti-horário.
            const glm::vec4 u = a - c;
            const glm::vec4 v = b - c;
            const glm::vec4 n = crossproduct(u, v);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3 * triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3 * triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize(3 * num_vertices);

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3 * i + 0] = n.x;
        model->attrib.normals[3 * i + 1] = n.y;
        model->attrib.normals[3 * i + 2] = n.z;
    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel *model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float> model_coefficients;
    std::vector<float> normal_coefficients;
    std::vector<float> texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3 * triangle + vertex];

                indices.push_back(first_index + 3 * triangle + vertex);

                const float vx = model->attrib.vertices[3 * idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3 * idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3 * idx.vertex_index + 2];
                // printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back(vx);   // X
                model_coefficients.push_back(vy);   // Y
                model_coefficients.push_back(vz);   // Z
                model_coefficients.push_back(1.0f); // W

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if (idx.normal_index != -1)
                {
                    const float nx = model->attrib.normals[3 * idx.normal_index + 0];
                    const float ny = model->attrib.normals[3 * idx.normal_index + 1];
                    const float nz = model->attrib.normals[3 * idx.normal_index + 2];
                    normal_coefficients.push_back(nx);   // X
                    normal_coefficients.push_back(ny);   // Y
                    normal_coefficients.push_back(nz);   // Z
                    normal_coefficients.push_back(0.0f); // W
                }

                if (idx.texcoord_index != -1)
                {
                    const float u = model->attrib.texcoords[2 * idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2 * idx.texcoord_index + 1];
                    texture_coefficients.push_back(u);
                    texture_coefficients.push_back(v);
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name = model->shapes[shape].name;
        theobject.first_index = first_index;                  // Primeiro índice
        theobject.num_indices = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;              // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0;            // "(location = 0)" em "shader_vertex.glsl"
    GLint number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!normal_coefficients.empty())
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1;             // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (!texture_coefficients.empty())
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2;             // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char *filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char *filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char *filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try
    {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar *shader_string = str.c_str();
    const GLint shader_string_length = static_cast<GLint>(str.length());

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar *log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if (log_length != 0)
    {
        std::string output;

        if (!compiled_ok)
>>>>>>> 91a1bd1 (remoção de tree2)
        {
            glUniform1i(render_as_black_uniform, true);
            glDrawElements(
                g_VirtualScene["cube_edges"].rendering_mode,
                g_VirtualScene["cube_edges"].num_indices,
                GL_UNSIGNED_INT,
                (void *)g_VirtualScene["cube_edges"].first_index);
        }
<<<<<<< HEAD
    }

    GLuint BuildTriangles()
    {
        GLfloat model_coefficients[] = {
            // Vértices de um cubo
            //    X      Y     Z     W
            -0.5f, 0.0f, 0.5f, 1.0f,   // posição do vértice 0
            -0.5f, -1.0f, 0.5f, 1.0f,  // posição do vértice 1
            0.5f, -1.0f, 0.5f, 1.0f,   // posição do vértice 2
            0.5f, 0.0f, 0.5f, 1.0f,    // posição do vértice 3
            -0.5f, 0.0f, -0.5f, 1.0f,  // posição do vértice 4
            -0.5f, -1.0f, -0.5f, 1.0f, // posição do vértice 5
            0.5f, -1.0f, -0.5f, 1.0f,  // posição do vértice 6
            0.5f, 0.0f, -0.5f, 1.0f,   // posição do vértice 7
                                       // Vértices para desenhar o eixo X
                                       //    X      Y     Z     W
            0.0f, 0.0f, 0.0f, 1.0f,    // posição do vértice 8
            1.0f, 0.0f, 0.0f, 1.0f,    // posição do vértice 9
                                       // Vértices para desenhar o eixo Y
                                       //    X      Y     Z     W
            0.0f, 0.0f, 0.0f, 1.0f,    // posição do vértice 10
            0.0f, 1.0f, 0.0f, 1.0f,    // posição do vértice 11
                                       // Vértices para desenhar o eixo Z
                                       //    X      Y     Z     W
            0.0f, 0.0f, 0.0f, 1.0f,    // posição do vértice 12
            0.0f, 0.0f, 1.0f, 1.0f,    // posição do vértice 13
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
        GLuint location = 0;            // "(location = 0)" em "shader_vertex.glsl"
        GLint number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
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
        location = 1;             // "(location = 1)" em "shader_vertex.glsl"
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
            0, 1,    // linha 1
            1, 2,    // linha 2
            2, 3,    // linha 3
            3, 0,    // linha 4
            0, 4,    // linha 5
            4, 7,    // linha 6
            7, 6,    // linha 7
            6, 2,    // linha 8
            6, 5,    // linha 9
            5, 4,    // linha 10
            5, 1,    // linha 11
            7, 3,    // linha 12
                     // Definimos os índices dos vértices que definem as linhas dos eixos X, Y,
                     // Z, que serão desenhados com o modo GL_LINES.
            8, 9,    // linha 1
            10, 11,  // linha 2
            12, 13   // linha 3
        };
        SceneObject cube_faces;
        cube_faces.name = "Cubo (faces coloridas)";
        cube_faces.first_index = (void *)0;       // Primeiro índice está em indices[0]
        cube_faces.num_indices = 36;              // Último índice está em indices[35]; total de 36 índices.
        cube_faces.rendering_mode = GL_TRIANGLES; // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        g_VirtualScene["cube_faces"] = cube_faces;
        SceneObject cube_edges;
        cube_edges.name = "Cubo (arestas pretas)";
        cube_edges.first_index = (void *)(36 * sizeof(GLuint)); // Primeiro índice está em indices[36]
        cube_edges.num_indices = 24;                            // Último índice está em indices[59]; total de 24 índices.
        cube_edges.rendering_mode = GL_LINES;                   // Índices correspondem ao tipo de rasterização GL_LINES.
        // Adicionamos o objeto criado acima na nossa cena virtual (g_VirtualScene).
        g_VirtualScene["cube_edges"] = cube_edges;
        // Criamos um terceiro objeto virtual (SceneObject) que se refere aos eixos XYZ.
        SceneObject axes;
        axes.name = "Eixos XYZ";
        axes.first_index = (void *)(60 * sizeof(GLuint)); // Primeiro índice está em indices[60]
        axes.num_indices = 6;                             // Último índice está em indices[65]; total de 6 índices.
        axes.rendering_mode = GL_LINES;                   // Índices correspondem ao tipo de rasterização GL_LINES.
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

    GLuint loadVertexShader(const char *filename)
    {
        GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        LoadShader(filename, vertex_shader_id);
        return vertex_shader_id;
    }

    GLuint loadFragmentShader(const char *filename)
    {
        GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
        LoadShader(filename, fragment_shader_id);
        return fragment_shader_id;
    }

    void LoadShader(const char *filename, GLuint shader_id)
    {
        // Le o arquivo do shader
        std::ifstream file;
        try
        {
            file.exceptions(std::ifstream::failbit);
            file.open(filename);
        }
        catch (std::exception &e)
        {
            fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
            std::exit(EXIT_FAILURE);
        }
        std::stringstream shader;
        shader << file.rdbuf();
        std::string str = shader.str();
        const GLchar *shader_string = str.c_str();
        const GLint shader_string_length = static_cast<GLint>(str.length());
        // Compila o shader
        glShaderSource(shader_id, 1, &shader_string, &shader_string_length);
        glCompileShader(shader_id);
        // Verifica se compilou
        GLint compiled_ok;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);
        GLint log_length = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
        GLchar *log = new GLchar[log_length];
        glGetShaderInfoLog(shader_id, log_length, &log_length, log);
        if (log_length != 0)
        {
            std::string output;
            // Loca o erro
            if (!compiled_ok)
            {
                output += "ERROR: OpenGL compilation of \"";
                output += filename;
                output += "\" failed.\n";
                output += "== Start of compilation log\n";
                output += log;
                output += "== End of compilation log\n";
            }
            else
            {
                output += "WARNING: OpenGL compilation of \"";
                output += filename;
                output += "\".\n";
                output += "== Start of compilation log\n";
                output += log;
                output += "== End of compilation log\n";
            }
            fprintf(stderr, "%s", output.c_str());
        }
        delete[] log;
    }

    void LoadShadersFromFiles()
    {
        GLuint vertex_shader_id = loadVertexShader("../assets/shader_vertex.glsl");
        GLuint fragment_shader_id = loadFragmentShader("../assets/shader_fragment.glsl");
        if (g_GpuProgramID != 0)
            glDeleteProgram(g_GpuProgramID);
        g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);
    }

    GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
    {
        // Create program
        GLuint program_id = glCreateProgram();
        // Link shaders
        glAttachShader(program_id, vertex_shader_id);
        glAttachShader(program_id, fragment_shader_id);
        glLinkProgram(program_id);
        // Check ok
        GLint linked_ok = GL_FALSE;
        glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);
        if (linked_ok == GL_FALSE)
        {
            // Log error
            GLint log_length = 0;
            glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
            GLchar *log = new GLchar[log_length];
            glGetProgramInfoLog(program_id, log_length, &log_length, log);
            std::string output;
            output += "ERROR: OpenGL linking of program failed.\n";
            output += "== Start of link log\n";
            output += log;
            output += "\n== End of link log\n";
            delete[] log;
            fprintf(stderr, "%s", output.c_str());
        }
        return program_id;
    }

    void FramebufferSizeCallback(GLFWwindow * window, int width, int height)
    {
        glViewport(0, 0, width, height);
        g_ScreenRatio = (float)width / height;
    }

    bool g_LeftMouseButtonPressed = false;
    bool g_RightMouseButtonPressed = false;
    bool g_MiddleMouseButtonPressed = false;
    double g_LastCursorPosX, g_LastCursorPosY;

    void MouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
            g_LeftMouseButtonPressed = true;
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            g_LeftMouseButtonPressed = false;
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
            g_RightMouseButtonPressed = true;
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        {
            g_RightMouseButtonPressed = false;
        }
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
        {
            glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
            g_MiddleMouseButtonPressed = true;
        }
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
        {
            g_MiddleMouseButtonPressed = false;
        }
    }

    void CursorPosCallback(GLFWwindow * window, double xpos, double ypos)
    {
        if (g_LeftMouseButtonPressed)
        {
            // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
            float dx = xpos - g_LastCursorPosX;
            float dy = ypos - g_LastCursorPosY;
            // Atualizamos parâmetros da câmera com os deslocamentos
            g_CameraTheta -= 0.01f * dx;
            g_CameraPhi += 0.01f * dy;
            // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
            float phimax = 3.141592f / 2;
            float phimin = -phimax;
            if (g_CameraPhi > phimax)
                g_CameraPhi = phimax;
            if (g_CameraPhi < phimin)
                g_CameraPhi = phimin;
            // Atualizamos as variáveis globais para armazenar a posição atual do
            // cursor como sendo a última posição conhecida do cursor.
            g_LastCursorPosX = xpos;
            g_LastCursorPosY = ypos;
        }
        if (g_RightMouseButtonPressed)
        {
            // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
            float dx = xpos - g_LastCursorPosX;
            float dy = ypos - g_LastCursorPosY;
            // Atualizamos parâmetros da antebraço com os deslocamentos
            g_ForearmAngleZ -= 0.01f * dx;
            g_ForearmAngleX += 0.01f * dy;
            // Atualizamos as variáveis globais para armazenar a posição atual do
            // cursor como sendo a última posição conhecida do cursor.
            g_LastCursorPosX = xpos;
            g_LastCursorPosY = ypos;
        }
        if (g_MiddleMouseButtonPressed)
        {
            // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
            float dx = xpos - g_LastCursorPosX;
            float dy = ypos - g_LastCursorPosY;
            // Atualizamos parâmetros da antebraço com os deslocamentos
            g_TorsoPositionX += 0.01f * dx;
            g_TorsoPositionY -= 0.01f * dy;
            // Atualizamos as variáveis globais para armazenar a posição atual do
            // cursor como sendo a última posição conhecida do cursor.
            g_LastCursorPosX = xpos;
            g_LastCursorPosY = ypos;
        }
    }

    void ScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
    {
        g_CameraDistance -= 1.0f * yoffset;
        const float verysmallnumber = std::numeric_limits<float>::epsilon();
        if (g_CameraDistance < verysmallnumber)
        {
            g_CameraDistance = verysmallnumber;
        }
    }

    void KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mod)
    {
        for (int i = 0; i < 10; ++i)
            if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
                std::exit(100 + i);
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        float delta = 3.141592 / 16; // 22.5 graus, em radianos.
        if (key == GLFW_KEY_X && action == GLFW_PRESS)
        {
            g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
        }
        else if (key == GLFW_KEY_Y && action == GLFW_PRESS)
        {
            g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
        }
        else if (key == GLFW_KEY_Z && action == GLFW_PRESS)
        {
            g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
        }
        else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            g_AngleX = 0.0f;
            g_AngleY = 0.0f;
            g_AngleZ = 0.0f;
            g_ForearmAngleX = 0.0f;
            g_ForearmAngleZ = 0.0f;
            g_TorsoPositionX = 0.0f;
            g_TorsoPositionY = 0.0f;
        }
        else if (key == GLFW_KEY_P && action == GLFW_PRESS)
        {
            g_UsePerspectiveProjection = true;
        }
        else if (key == GLFW_KEY_O && action == GLFW_PRESS)
        {
            g_UsePerspectiveProjection = false;
        }
        else if (key == GLFW_KEY_H && action == GLFW_PRESS)
        {
            g_ShowInfoText = !g_ShowInfoText;
        }
        else if (key == GLFW_KEY_F && action == GLFW_PRESS)
        {
            Particle::ParticleProprieties particle;

            // Configurar o particleProprieties
            particle.x = 0;
            particle.y = 0;
            particle.z = 0;
            // speed
            particle.xs = 0;
            particle.ys = 0;
            particle.zs = 0;
            // acceleration
            particle.xa = 0;
            particle.ya = 0;
            particle.za = 0;
            // initial rotation
            particle.rotationX = 0;
            particle.rotationY = 0;
            particle.rotationZ = 0;
            // final rotation
            particle.rotationSpeedX = 0;
            particle.rotationSpeedY = 0;
            particle.rotationSpeedZ = 0;
            particle.size = 1.0f;
            particle.sizeChange = -1.0f;
            particle.duration = 8.0f;
            // Carregar o objeto da particula (nesse caso o cubo)
            RenderObject ro((void *)g_VirtualScene["cube_faces"].first_index, g_VirtualScene["cube_faces"].num_indices, g_VirtualScene["cube_faces"].rendering_mode);
            particle.object = ro;

            float explosionDelay = 5.0f;
            float explosionHeight = 200.0f;

            {
                // Configurar o particleProprieties
                particle.x = 0;
                particle.y = 0;
                particle.z = 0;
                particle.xa = 0;
                particle.ya = -1.0f;
                particle.za = 0;
                particle.xs = 0;
                particle.ys = (explosionHeight / explosionDelay) - (particle.ya * explosionDelay / 2.0f);
                particle.zs = 0;
                particle.rotationX = 0;
                particle.rotationY = 0;
                particle.rotationZ = 0;
                particle.rotationSpeedX = 0;
                particle.rotationSpeedY = 0;
                particle.rotationSpeedZ = 0;
                particle.duration = explosionDelay;

                for (float i = 0; i < 0.5f; i += 0.05)
                {
                    particle.size = 5.0f * ((0.5f - i) / 0.5f);
                    particle.sizeChange = -particle.size * 0.1;
                    emitter.emitIn(particle, i);
                }
            }

            {
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

                float speed = 1.0f;
                float r = speed;
                for (float i = 0.0f; i < 2.0f * PI; i += (2.0f * PI) / SIDES)
                {
                    for (float j = -PI / 2.0f; j < PI * 2.0f; j += PI / VSIDES)
                    {
                        float y = r * sin(i);
                        float z = r * cos(i) * cos(j);
                        float x = r * cos(i) * sin(j);

                        particle.x = 0;
                        particle.y = 30.0f; // height
                        particle.z = 0;
                        particle.xs = x;
                        particle.ys = y;
                        particle.zs = z;
                        particle.x = 0;
                        particle.y = explosionHeight;
                        particle.z = 0;
                        particle.xs = x;
                        particle.ys = y;
                        particle.zs = z;

                        particle.xa = -x * 0.05;
                        particle.ya = -y * 0.05;
                        particle.za = -z * 0.05;

                        particle.ys += 1.0f;
                        particle.ya -= 1.0f;

                        // scale factor
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

                        for (int k = 0; k < 20; k++)
                        {
                            particle.size = (20.0 - k) / 20.0f;
                            particle.sizeChange = -particle.size;
                            emitter.emitIn(particle, explosionDelay + (k / 20.0f));
                        }
                    }
                }
            }
        }
        == == == =
                     else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete[] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if (linked_ok == GL_FALSE)
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar *log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete[] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    if (g_LeftMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f * dx;
        g_CameraPhi += 0.01f * dy;

        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f / 2;
        float phimin = -phimax;

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;

        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_RightMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_ForearmAngleZ -= 0.01f * dx;
        g_ForearmAngleX += 0.01f * dy;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_TorsoPositionX += 0.01f * dx;
        g_TorsoPositionY -= 0.01f * dy;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.1f * yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mod)
{
    // ==================
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // ==================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // O código abaixo implementa a seguinte lógica:
    //   Se apertar tecla X       então g_AngleX += delta;
    //   Se apertar tecla shift+X então g_AngleX -= delta;
    //   Se apertar tecla Y       então g_AngleY += delta;
    //   Se apertar tecla shift+Y então g_AngleY -= delta;
    //   Se apertar tecla Z       então g_AngleZ += delta;
    //   Se apertar tecla shift+Z então g_AngleZ -= delta;

    float delta = 3.141592 / 16; // 22.5 graus, em radianos.

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;
        g_ForearmAngleX = 0.0f;
        g_ForearmAngleZ = 0.0f;
        g_TorsoPositionX = 0.0f;
        g_TorsoPositionY = 0.0f;
    }

    // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = true;
    }

    // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = false;
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout, "Shaders recarregados!\n");
        fflush(stdout);
>>>>>>> 91a1bd1 (remoção de tree2)
    }

<<<<<<< HEAD
    void ErrorCallback(int error, const char *description) == == == =
                                                                        // Definimos o callback para impressão de erros da GLFW no terminal
        void ErrorCallback(int error, const char *description)
    {
        fprintf(stderr, "ERROR: GLFW: %s\n", description);
    }

    // Esta função recebe um vértice com coordenadas de modelo p_model e passa o
    // mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
    // view, e projection; e escreve na tela as matrizes e pontos resultantes
    // dessas transformações.
    void TextRendering_ShowModelViewProjection(
        GLFWwindow * window,
        glm::mat4 projection,
        glm::mat4 view,
        glm::mat4 model,
        glm::vec4 p_model)
    {
        if (!g_ShowInfoText)
            return;

        glm::vec4 p_world = model * p_model;
        glm::vec4 p_camera = view * p_world;
        glm::vec4 p_clip = projection * p_camera;
        glm::vec4 p_ndc = p_clip / p_clip.w;

        float pad = TextRendering_LineHeight(window);

        TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f - pad, 1.0f);
        TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f - 2 * pad, 1.0f);

        TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f - 6 * pad, 1.0f);
        TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f - 7 * pad, 1.0f);
        TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f - 8 * pad, 1.0f);

        TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f - 9 * pad, 1.0f);
        TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f - 10 * pad, 1.0f);

        TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f - 14 * pad, 1.0f);
        TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f - 15 * pad, 1.0f);
        TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f - 16 * pad, 1.0f);

        TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f - 17 * pad, 1.0f);
        TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f - 18 * pad, 1.0f);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glm::vec2 a = glm::vec2(-1, -1);
        glm::vec2 b = glm::vec2(+1, +1);
        glm::vec2 p = glm::vec2(0, 0);
        glm::vec2 q = glm::vec2(width, height);

        glm::mat4 viewport_mapping = Matrix(
            (q.x - p.x) / (b.x - a.x), 0.0f, 0.0f, (b.x * p.x - a.x * q.x) / (b.x - a.x),
            0.0f, (q.y - p.y) / (b.y - a.y), 0.0f, (b.y * p.y - a.y * q.y) / (b.y - a.y),
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f - 22 * pad, 1.0f);
        TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f - 23 * pad, 1.0f);
        TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f - 24 * pad, 1.0f);

        TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f - 25 * pad, 1.0f);
        TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f - 26 * pad, 1.0f);
    }

    // Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
    // g_AngleX, g_AngleY, e g_AngleZ.
    void TextRendering_ShowEulerAngles(GLFWwindow * window)
    {
        if (!g_ShowInfoText)
            return;

        float pad = TextRendering_LineHeight(window);

        char buffer[80];
        snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

        TextRendering_PrintString(window, buffer, -1.0f + pad / 10, -1.0f + 2 * pad / 10, 1.0f);
    }

    // Escrevemos na tela qual matriz de projeção está sendo utilizada.
    void TextRendering_ShowProjection(GLFWwindow * window)
    {
        if (!g_ShowInfoText)
            return;

        float lineheight = TextRendering_LineHeight(window);
        float charwidth = TextRendering_CharWidth(window);

        if (g_UsePerspectiveProjection)
            TextRendering_PrintString(window, "Perspective", 1.0f - 13 * charwidth, -1.0f + 2 * lineheight / 10, 1.0f);
        else
            TextRendering_PrintString(window, "Orthographic", 1.0f - 13 * charwidth, -1.0f + 2 * lineheight / 10, 1.0f);
    }

    // Escrevemos na tela o número de quadros renderizados por segundo (frames per
    // second).
    void TextRendering_ShowFramesPerSecond(GLFWwindow * window)
    {
        if (!g_ShowInfoText)
            return;

        // Variáveis estáticas (static) mantém seus valores entre chamadas
        // subsequentes da função!
        static float old_seconds = (float)glfwGetTime();
        static int ellapsed_frames = 0;
        static char buffer[20] = "?? fps";
        static int numchars = 7;

        ellapsed_frames += 1;

        // Recuperamos o número de segundos que passou desde a execução do programa
        float seconds = (float)glfwGetTime();

        // Número de segundos desde o último cálculo do fps
        float ellapsed_seconds = seconds - old_seconds;

        if (ellapsed_seconds > 1.0f)
>>>>>>> 91a1bd1 (remoção de tree2)
        {
            fprintf(stderr, "ERROR: GLFW: %s\n", description);
        }

        void TextRendering_ShowModelViewProjection(
            GLFWwindow * window,
            glm::mat4 projection,
            glm::mat4 view,
            glm::mat4 model,
            glm::vec4 p_model)
        {
            if (!g_ShowInfoText)
                return;

            glm::vec4 p_world = model * p_model;
            glm::vec4 p_camera = view * p_world;
            glm::vec4 p_clip = projection * p_camera;
            glm::vec4 p_ndc = p_clip / p_clip.w;

<<<<<<< HEAD
            float pad = TextRendering_LineHeight(window);

            TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f - pad, 1.0f);
            TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f - 2 * pad, 1.0f);

            TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f - 6 * pad, 1.0f);
            TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f - 7 * pad, 1.0f);
            TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f - 8 * pad, 1.0f);

            TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f - 9 * pad, 1.0f);
            TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f - 10 * pad, 1.0f);

            TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f - 14 * pad, 1.0f);
            TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f - 15 * pad, 1.0f);
            TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f - 16 * pad, 1.0f);

            TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f - 17 * pad, 1.0f);
            TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f - 18 * pad, 1.0f);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            glm::vec2 a = glm::vec2(-1, -1);
            glm::vec2 b = glm::vec2(+1, +1);
            glm::vec2 p = glm::vec2(0, 0);
            glm::vec2 q = glm::vec2(width, height);

            glm::mat4 viewport_mapping = Matrix(
                (q.x - p.x) / (b.x - a.x), 0.0f, 0.0f, (b.x * p.x - a.x * q.x) / (b.x - a.x),
                0.0f, (q.y - p.y) / (b.y - a.y), 0.0f, (b.y * p.y - a.y * q.y) / (b.y - a.y),
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);

            TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f - 22 * pad, 1.0f);
            TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f - 23 * pad, 1.0f);
            TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f - 24 * pad, 1.0f);

            TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f - 25 * pad, 1.0f);
            TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f - 26 * pad, 1.0f);
        }

        // Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
        // g_AngleX, g_AngleY, e g_AngleZ.
        void TextRendering_ShowEulerAngles(GLFWwindow * window)
        {
            if (!g_ShowInfoText)
                return;

            float pad = TextRendering_LineHeight(window);

            char buffer[80];
            snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

            TextRendering_PrintString(window, buffer, -1.0f + pad / 10, -1.0f + 2 * pad / 10, 1.0f);
        }

        // Escrevemos na tela qual matriz de projeção está sendo utilizada.
        void TextRendering_ShowProjection(GLFWwindow * window)
        {
            if (!g_ShowInfoText)
                return;

            float lineheight = TextRendering_LineHeight(window);
            float charwidth = TextRendering_CharWidth(window);

            if (g_UsePerspectiveProjection)
                TextRendering_PrintString(window, "Perspective", 1.0f - 13 * charwidth, -1.0f + 2 * lineheight / 10, 1.0f);
            else
                TextRendering_PrintString(window, "Orthographic", 1.0f - 13 * charwidth, -1.0f + 2 * lineheight / 10, 1.0f);
        }

        // Escrevemos na tela o número de quadros renderizados por segundo (frames per
        // second).
        void TextRendering_ShowFramesPerSecond(GLFWwindow * window)
        {
            if (!g_ShowInfoText)
                return;

            // Variáveis estáticas (static) mantém seus valores entre chamadas
            // subsequentes da função!
            static float old_seconds = (float)glfwGetTime();
            static int ellapsed_frames = 0;
            static char buffer[20] = "?? fps";
            static int numchars = 7;

            ellapsed_frames += 1;

            // Recuperamos o número de segundos que passou desde a execução do programa
            float seconds = (float)glfwGetTime();

            // Número de segundos desde o último cálculo do fps
            float ellapsed_seconds = seconds - old_seconds;

            if (ellapsed_seconds > 1.0f)
            {
                numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

                old_seconds = seconds;
                ellapsed_frames = 0;
            }

            float lineheight = TextRendering_LineHeight(window);
            float charwidth = TextRendering_CharWidth(window);

            TextRendering_PrintString(window, buffer, 1.0f - (numchars + 1) * charwidth, 1.0f - lineheight, 1.0f);
        }

        // set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
        // vim: set spell spelllang=pt_br :
        == == == =
                     // Função para debugging: imprime no terminal todas informações de um modelo
                     // geométrico carregado de um arquivo ".obj".
                     // Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
            void PrintObjModelInfo(ObjModel * model)
        {
            const tinyobj::attrib_t &attrib = model->attrib;
            const std::vector<tinyobj::shape_t> &shapes = model->shapes;
            const std::vector<tinyobj::material_t> &materials = model->materials;

            printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
            printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
            printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
            printf("# of shapes    : %d\n", (int)shapes.size());
            printf("# of materials : %d\n", (int)materials.size());

            for (size_t v = 0; v < attrib.vertices.size() / 3; v++)
            {
                printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
                       static_cast<const double>(attrib.vertices[3 * v + 0]),
                       static_cast<const double>(attrib.vertices[3 * v + 1]),
                       static_cast<const double>(attrib.vertices[3 * v + 2]));
            }

            for (size_t v = 0; v < attrib.normals.size() / 3; v++)
            {
                printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
                       static_cast<const double>(attrib.normals[3 * v + 0]),
                       static_cast<const double>(attrib.normals[3 * v + 1]),
                       static_cast<const double>(attrib.normals[3 * v + 2]));
            }

            for (size_t v = 0; v < attrib.texcoords.size() / 2; v++)
            {
                printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
                       static_cast<const double>(attrib.texcoords[2 * v + 0]),
                       static_cast<const double>(attrib.texcoords[2 * v + 1]));
            }

            // For each shape
            for (size_t i = 0; i < shapes.size(); i++)
            {
                printf("shape[%ld].name = %s\n", static_cast<long>(i),
                       shapes[i].name.c_str());
                printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
                       static_cast<unsigned long>(shapes[i].mesh.indices.size()));

                size_t index_offset = 0;

                assert(shapes[i].mesh.num_face_vertices.size() ==
                       shapes[i].mesh.material_ids.size());

                printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
                       static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

                // For each face
                for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++)
                {
                    size_t fnum = shapes[i].mesh.num_face_vertices[f];

                    printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
                           static_cast<unsigned long>(fnum));

                    // For each vertex in the face
                    for (size_t v = 0; v < fnum; v++)
                    {
                        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
                        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
                               static_cast<long>(v), idx.vertex_index, idx.normal_index,
                               idx.texcoord_index);
                    }

                    printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
                           shapes[i].mesh.material_ids[f]);

                    index_offset += fnum;
                }

                printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
                       static_cast<unsigned long>(shapes[i].mesh.tags.size()));
                for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++)
                {
                    printf("  tag[%ld] = %s ", static_cast<long>(t),
                           shapes[i].mesh.tags[t].name.c_str());
                    printf(" ints: [");
                    for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j)
                    {
                        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
                        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1))
                        {
                            printf(", ");
                        }
                    }
                    printf("]");

                    printf(" floats: [");
                    for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j)
                    {
                        printf("%f", static_cast<const double>(
                                         shapes[i].mesh.tags[t].floatValues[j]));
                        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1))
                        {
                            printf(", ");
                        }
                    }
                    printf("]");

                    printf(" strings: [");
                    for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j)
                    {
                        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
                        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1))
                        {
                            printf(", ");
                        }
                    }
                    printf("]");
                    printf("\n");
                }
            }

            for (size_t i = 0; i < materials.size(); i++)
            {
                printf("material[%ld].name = %s\n", static_cast<long>(i),
                       materials[i].name.c_str());
                printf("  material.Ka = (%f, %f ,%f)\n",
                       static_cast<const double>(materials[i].ambient[0]),
                       static_cast<const double>(materials[i].ambient[1]),
                       static_cast<const double>(materials[i].ambient[2]));
                printf("  material.Kd = (%f, %f ,%f)\n",
                       static_cast<const double>(materials[i].diffuse[0]),
                       static_cast<const double>(materials[i].diffuse[1]),
                       static_cast<const double>(materials[i].diffuse[2]));
                printf("  material.Ks = (%f, %f ,%f)\n",
                       static_cast<const double>(materials[i].specular[0]),
                       static_cast<const double>(materials[i].specular[1]),
                       static_cast<const double>(materials[i].specular[2]));
                printf("  material.Tr = (%f, %f ,%f)\n",
                       static_cast<const double>(materials[i].transmittance[0]),
                       static_cast<const double>(materials[i].transmittance[1]),
                       static_cast<const double>(materials[i].transmittance[2]));
                printf("  material.Ke = (%f, %f ,%f)\n",
                       static_cast<const double>(materials[i].emission[0]),
                       static_cast<const double>(materials[i].emission[1]),
                       static_cast<const double>(materials[i].emission[2]));
                printf("  material.Ns = %f\n",
                       static_cast<const double>(materials[i].shininess));
                printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
                printf("  material.dissolve = %f\n",
                       static_cast<const double>(materials[i].dissolve));
                printf("  material.illum = %d\n", materials[i].illum);
                printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
                printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
                printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
                printf("  material.map_Ns = %s\n",
                       materials[i].specular_highlight_texname.c_str());
                printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
                printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
                printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
                printf("  <<PBR>>\n");
                printf("  material.Pr     = %f\n", materials[i].roughness);
                printf("  material.Pm     = %f\n", materials[i].metallic);
                printf("  material.Ps     = %f\n", materials[i].sheen);
                printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
                printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
                printf("  material.aniso  = %f\n", materials[i].anisotropy);
                printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
                printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
                printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
                printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
                printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
                printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
                std::map<std::string, std::string>::const_iterator it(
                    materials[i].unknown_parameter.begin());
                std::map<std::string, std::string>::const_iterator itEnd(
                    materials[i].unknown_parameter.end());

                for (; it != itEnd; it++)
                {
                    printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
                }
                printf("\n");
            }
        }

        // set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
        // vim: set spell spelllang=pt_br :
>>>>>>> 91a1bd1 (remoção de tree2)
