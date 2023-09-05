namespace collision
{
  // Ponto ou vertor no espaço tridimensional
  struct Point
  {
    float x, y, z;
  };

  // Cubo para intersecção
  struct Cube
  {
    // Posição do vértice com menores x, y, e z
    Point positionMin;
    // Posição do vértice com maiores x, y, e z
    Point positionMax;
  };

  // TODO: documentation
  struct Plane
  {
    Point position;
    Point normal;
  };

  // TODO: documentation
  struct Sphere
  {
    Point position;
    float r;
  };

  // TODO: documentation
  struct Ray
  {
    Point startPosition;
    Point direction;

    Point at(float t);
  };

  // Verifica se os cubos se intersectam
  bool check(Cube &cube1, Cube &cube2);

  // Verifica se o cubo e o plano se intersectam
  bool check(Cube &cube, Plane &plane);

  // Verifica se a esfera e o ponto se intersectam
  bool check(Sphere &sphere, Point &point);

  // Calcula quando o raio intersecciona o plano
  // - Caso o raio interseccione o plano, o retorno é positiovo
  // - Caso o plano esteja atrás do raio, o retorna é negativo
  // - Caso o raio seja paralelo ao plano, o retorno é o infinito negativo de float
  float collide(Plane &plane, Ray &ray);
}