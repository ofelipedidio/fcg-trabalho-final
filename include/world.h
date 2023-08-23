#pragma once

#include <algorithm>
#include <vector>

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

// Headers das bibliotecas OpenGL
#include "glad/glad.h"  // Criação de contexto OpenGL 3.3
#include "GLFW/glfw3.h" // Criação de janelas do sistema operacional

#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "utils.h"
#include "matrices.h"
#include "renderer.h"
#include "object.h"
#include <tiny_obj_loader.h>

namespace World
{
  struct SceneObject
  {
    std::string name;              // Nome do objeto
    size_t first_index;            // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t num_indices;            // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum rendering_mode;         // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
  };
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

  std::map<std::string, SceneObject> g_VirtualScene;
  void BuildTrianglesAndAddToVirtualScene(ObjModel *); // Constrói representação de um ObjModel como malha de triângulos para renderização
  void ComputeNormals(ObjModel *model);                // Computa normais de um ObjModel, caso não existam.
  void PrintObjModelInfo(ObjModel *);                  // Função para debugging
  void loadModel();
  void drawModel();
}