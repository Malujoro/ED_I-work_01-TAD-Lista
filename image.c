#include "image.h"
#include <stdio.h>
#include <stdlib.h>

// TODO Criar função de cálculo de posição
// TODO Criar função de se comunicar com python
// TODO Criar função de Salvar imagem (RGB e Gray)

/* FUNÇÃO "Teste" da comunicação com python
int main()
{
    int tam = 128;
    char comando[tam];

    int tipo, cor;
    tipo = 1;
    cor = 1;
    char origem[] = "imagem.txt";
    char saida[] = "pasta/saida.png";

    snprintf(comando, tam, "python3 utils/image_utils.py %d %d %s %s", tipo, cor, origem, saida);

    printf("%s", comando);
    return 0;
}
*/


// Funções de criação e liberação
ImageGray *create_image_gray(int largura, int altura)
{

}

void free_image_gray(ImageGray *image)
{

}


ImageRGB *create_image_rgb(int largura, int altura)
{

}

void free_image_rgb(ImageRGB *image)
{

}


// Operações para ImageGray
ImageGray *flip_vertical_gray(ImageGray *image)
{

}

ImageGray *flip_horizontal_gray(ImageGray *image)
{

}

ImageGray *transpose_gray(const ImageGray *image)
{

}


// Operações para ImageRGB
ImageRGB *flip_vertical_rgb(const ImageRGB *image)
{

}

ImageRGB *flip_horizontal_rgb(const ImageRGB *image)
{

}

ImageRGB *transpose_rgb(const ImageRGB *image)
{

}


// Manipulação por pixel para ImageGray
ImageGray *clahe_gray(const ImageGray *image, int tile_width, int tile_height)
{

}

ImageGray *median_blur_gray(const ImageGray *image, int kernel_size)
{

}


// Manipulação por pixel para ImageRGB
ImageRGB *clahe_rgb(const ImageRGB *image, int tile_width, int tile_height)
{

}

ImageRGB *median_blur_rgb(const ImageRGB *image, int kernel_size)
{
 
    
}