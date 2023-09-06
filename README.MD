# Fogos de Artifício

![Imagem 1](https://github.com/ofelipedidio/fcg-trabalho-final/blob/ambient-master-merge/image1.png?raw=true)
![Imagem 2](https://github.com/ofelipedidio/fcg-trabalho-final/blob/ambient-master-merge/image2.png?raw=true)


## Membros da dupla

- Felipe Souza Didio
- Rafael Lacerda

## Manual de uso

Tecla | Funcionalidade
----- | ------
Q | Alterna entre os modos de câmera LookAt e Livre
WASD | Controla o movimento da câmera livre no plano XZ
Shift | Move a câmera para baixo
Space | Move a câmera para cima
E e F | Lançam fogos em padrões diferentes
B | Faz a câmera passar pelo caminho da curva de Bézier
Esc | Fecha a janela
P | Muda a projeção para perspectiva
O | Muda a projeção para ortogonal
1, 2, 3 e 4 | Move a câmera para posições pré-definidas

### Mouse

O botão esquerdo do mouse controla a rotação da câmera. O botão direito do mouse lança um fogo de artifício no meio da tela.

## Relatório

O projeto compõe um sistema de partículas implementado para simulação de fogos de artifício, múltiplas câmeras de visualização em tempo real, cenário noturno simulando um parque de diversões para visualização de um pequeno show e explosões disparadas pelo usuário ao clicar no chão. A criação do cenário, bem como a inclusão de modelos obj complexos, texturização e configuração das câmeras é parte do trabalho de Rafael Lacerda Busatta. A criação é configuração do sistema de partículas, animação da câmera com bezier e colisões é parte do trabalho de Felipe Didio. Ambos os colegas trabalharam conjuntamente fazendo pair programming e individualmente em diferentes estágios do desenvolvimento. A unificação do código foi um trabalho conjunto e lento, devido a diversos conflitos que precisaram ser resolvidos. Por fim, decidiu-se fazer a inclusão manual das alterações no arquivo principal (main.cpp).

## Uso do ChatGPT

Não foi usado o ChatGPT para gerar código, mas foi utilizado para entendar o que algumas funções do OpenGL. O ChatGPT foi bastante útil para isso.

## Como rodar o arquivo binário

O arquivo binário deve ser executado no diretório onde ele se localiza. No diretório pai do diretório do arquivo binário, devem estar contidos os diretórios assets e data.

## Requisitos

### Interação em tempo real

Com exceção de quando muitos fogos de artifício, a aplicação roda em tempo real. Quando a computação dos quadros toma mais tempo, o cálculo de animações não é afetado.

### Lógica de controle não trivial

O programa possui lógica de controle não trivial, por exemplo, permitindo que usuários lançem fogos de artifício utilizando o mouse.

### Utilização de matrizes para as transformações geométricas vistas em aula

A aplicação exclusivamente utiliza das matrizes vistas em aula para calcular transformações geométricas.

### Interação do usuário através do teclado e mouse

A aplicação utiliza do mouse e do teclado para a interação do usuário. Um exemplo é a movimentação da câmera, que utiliza ambos.

### Malhas poligonais complexas

A aplicação implementa o carregamento de malhas poligonais complexas através de arquivos OBJ. A aplicação utiliza essa funcionalidade para carregar modelos como o modelo da roda gigante.

### Testes de intersecção entre objetos virtuais

A aplicação implementa as colisões requeridas, além do teste de intersecção de linha com plano. Apenas a intersecção de linha com plano é utilizada (para criar fogos de artifício com o botão direito do mouse).

### Modelos de iluminação de objetos geométricos

São utilizados os modelos de iluminação difuso e Blinn-Phong para todos os objetos geométricos.

### Mapeamento de texturas

A aplicação usa mais de 5 texturas diferentes. Exemplos são: para o céu, para o chão, para a torre e para a rocha.

### Curvas de Bézier

A aplicação utiliza curvas de Bézier para controlar a movimentação da câmera quando a tecla B é apertada.

### Animação de Movimento baseada no tempo

É implementada a animação baseada no tempo para a câmera e para os fógos de artifício.

## Funcionalidades Extras

É implementado um sistema de partículas para os fogos


