#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct dimensoes {
    int altura, largura;
} Dimensoes;

typedef struct pixelRGB {
    int red, blue, green;
} PixelRGB;

typedef struct pixelGray {
    int value;
} PixelGray;

typedef struct imageGray {
    Dimensoes dim;
    PixelGray *pixels;
} ImageGray;

typedef struct imageRGB {
    Dimensoes dim;
    PixelRGB *pixels;
} ImageRGB;

//////////////////// Funções auxiliares ////////////////////

// Função para converter uma posição de matriz em posição de vetor
int posicaoVetor(int largura, int i, int j);

FILE *lerArquivo(char *caminho, char *modo);

char *alocarStr(int tam);
int *alocarInt(int tam);
float *alocarFloat(int tam);

// Funções para alocar um vetor de pixels
PixelRGB *alocarPixelRGB(int tam);
PixelGray *alocarPixelGray(int tam);

void *liberarVetor(void *vetor);
void limparFloat(float *vetor, int tam);

char *intParaStr(int num);

char *gerarCaminho(char *pasta, char *nome, char *tipo);

void criarPasta(char *caminho);
int pastaExiste(char *caminho);
int contarPastas(char *caminho);

void python(char *origem, char *tipo, char *cor, char *pasta, char *nome, char *extensao);

///////// Auxiliar Median Blur /////////

int mediana(int *vetor, int tam);

//////////// Auxiliar Clahe ////////////

float cdf(float *vetor, int pos);
float cdf_normalizado(float cdf_i, float cdf_min, float cdf_max);
void redistribuirHistograma(float *histograma);
int posMinimo(float *histograma);
int posMaximo(float *histograma);
void suavizaLinhaGray(ImageGray *image, int height);
void suavizaColunaGray(ImageGray *image, int width);

////////////// Funções de criação e liberação //////////////

ImageGray *create_image_gray(int largura, int altura);
void free_image_gray(ImageGray *image);

ImageRGB *create_image_rgb(int largura, int altura);
void free_image_rgb(ImageRGB *image);

ImageGray *copiarImagemGray(const ImageGray *image);
ImageRGB *copiarImagemRGB(const ImageRGB *image);

////////////// Funções para leitura e salvamento //////////////

ImageGray *lerTxtGray(char *pasta, char *nome);
ImageRGB *lerTxtRGB(char *pasta, char *nome);

ImageGray *lerImagemGray(char *origem, char *pasta, char *nome);
ImageRGB *lerImagemRGB(char *pasta, char *nome);

void salvarTxtGray(ImageGray *imagem, char *caminho, char *nome);
void salvarTxtRGB(ImageRGB *imagem);

void salvarImagemGray(ImageGray *imagem, char *caminho, char *nome);
void salvarImagemRGB(ImageRGB *imagem);


////////////////// Funções para Operações //////////////////

// Operações para ImageGray
ImageGray *flip_vertical_gray(ImageGray *image);
ImageGray *flip_horizontal_gray(ImageGray *image);
ImageGray *transpose_gray(const ImageGray *image);

// Operações para ImageRGB
ImageRGB *flip_vertical_rgb(const ImageRGB *image);
ImageRGB *flip_horizontal_rgb(const ImageRGB *image);
ImageRGB *transpose_rgb(const ImageRGB *image);


///////////// Funções de Manipulação por Pixel /////////////

// Manipulação por pixel para ImageGray
ImageGray *clahe_gray(const ImageGray *image, int tile_width, int tile_height);
ImageGray *median_blur_gray(const ImageGray *image, int kernel_size);

// Manipulação por pixel para ImageRGB
ImageRGB *clahe_rgb(const ImageRGB *image, int tile_width, int tile_height);
ImageRGB *median_blur_rgb(const ImageRGB *image, int kernel_size);

////////////////////////////////////////////////////////////

#endif // IMAGE_H
