#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> // Biblioteca para verificar pastas
#include <sys/stat.h> // Biblioteca para criar pastas
#include <Python.h> // API para utilizar o python em C

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
void limparVet(int *vetor, int tam);
char *intParaStr(int num);

FILE *lerArquivo(char *caminho, char *modo);

char *alocarStr(int tam);
int *alocarInt(int tam);
PyObject **alocarPython(int tam);

// Funções para alocar um vetor de pixels
PixelRGB *alocarPixelRGB(int tam);
PixelGray *alocarPixelGray(int tam);

void *liberarVetor(void *vetor);

//////////////// Pastas ////////////////

char *gerarCaminho(char *pasta, char *simbolo, char *nome);
DIR *abrirPasta(char *caminho);
void criarPasta(char *caminho);
int pastaExiste(char *caminho);
int contarPastas(char *caminho);
char *pastaPrincipal(char *caminho);

//////////////// Python ////////////////

PyObject **inicializaPython(char *funcao, char *image_path, char *output_path, int gray);
void executaPython(PyObject **matriz);
void finalizaPython(PyObject **matriz);

void txt_from_image(char *image_path, char *output_path, int gray);
void image_from_txt(char *txt_path, char *output_path, int gray);

///////// Auxiliar Median Blur /////////

int mediana(int *vetor, int tam);

//////////// Auxiliar Clahe ////////////

int cdf(int *vetor, int pos);
int cdf_normalizado(int cdf_i, int cdf_min, int cdf_max);
void redistribuirHistograma(int *histograma, int clip_limit);
int posMinimo(int *histograma);
int posMenor(int *histograma);
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

ImageGray *lerTxtGray(char *caminho);
ImageRGB *lerTxtRGB(char *caminho);

ImageGray *lerImagemGray(char *png, char *txt);
ImageRGB *lerImagemRGB(char *png, char *txt);

void salvarTxtGray(ImageGray *imagem, char *caminho, char *txt);
void salvarTxtRGB(ImageRGB *imagem);

void salvarImagemGray(ImageGray *imagem, char *caminho, char *txt, char *png);
void salvarImagemRGB(ImageRGB *imagem);


////////////////// Funções para Operações //////////////////

// Operações para ImageGray
ImageGray *flip_vertical_gray(const ImageGray *image);
ImageGray *flip_horizontal_gray(const ImageGray *image);
ImageGray *rotate_90_gray(const ImageGray *image);
ImageGray *transpose_gray(const ImageGray *image);

// Operações para ImageRGB
ImageRGB *flip_vertical_rgb(const ImageRGB *image);
ImageRGB *flip_horizontal_rgb(const ImageRGB *image);
ImageRGB *rotate_90_rgb(const ImageRGB *image);
ImageRGB *transpose_rgb(const ImageRGB *image);


///////////// Funções de Manipulação por Pixel /////////////

// Manipulação por pixel para ImageGray
ImageGray *clahe_gray(const ImageGray *image, int tile_width, int tile_height);
ImageGray *median_blur_gray(const ImageGray *image, int kernel_size);
ImageGray *negativo_gray(const ImageGray *image);

// Manipulação por pixel para ImageRGB
ImageRGB *clahe_rgb(const ImageRGB *image, int tile_width, int tile_height);
ImageRGB *median_blur_rgb(const ImageRGB *image, int kernel_size);
ImageRGB *negativo_rgb(const ImageRGB *image);

////////////////////////////////////////////////////////////

void Geral();

#endif // IMAGE_H
