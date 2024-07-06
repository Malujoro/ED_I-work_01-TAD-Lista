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

//////////////// Python ////////////////

void txt_from_image(char *image_path, char *output_path, int gray);
void image_from_txt(char *txt_path, char *output_path, int gray);

////////////// Funções de criação e liberação //////////////

ImageGray *create_image_gray(int largura, int altura);
void free_image_gray(ImageGray *image);

ImageRGB *create_image_rgb(int largura, int altura);
void free_image_rgb(ImageRGB *image);

////////////// Funções para leitura e salvamento //////////////

void salvarTxtGray(ImageGray *imagem, char *caminho, char *txt);
void salvarTxtRGB(ImageRGB *imagem, char *caminho, char *txt);

void salvarImagemGray(ImageGray *imagem, char *caminho, char *txt, char *png);
void salvarImagemRGB(ImageRGB *imagem, char *caminho, char *txt, char *png);

////////////////// Funções para Operações //////////////////

// Operações para ImageGray
ImageGray *flip_vertical_gray(const ImageGray *image);
ImageGray *flip_horizontal_gray(const ImageGray *image);
ImageGray *rotate_90_gray(const ImageGray *image);
ImageGray *rotate_90_anti_gray(const ImageGray *image);
ImageGray *transpose_gray(const ImageGray *image);
ImageGray *transpose2_gray(const ImageGray *image);

// Operações para ImageRGB
ImageRGB *flip_vertical_rgb(const ImageRGB *image);
ImageRGB *flip_horizontal_rgb(const ImageRGB *image);
ImageRGB *rotate_90_rgb(const ImageRGB *image);
ImageRGB *rotate_90_anti_rgb(const ImageRGB *image);
ImageRGB *transpose_rgb(const ImageRGB *image);
ImageRGB *transpose2_rgb(const ImageRGB *image);


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

int iniciar(int argc, char **argv);

#endif // IMAGE_H
