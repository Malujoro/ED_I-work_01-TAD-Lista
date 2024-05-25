#include "image.h"
#include <stdio.h>
#include <stdlib.h>

// TODO Criar função de se comunicar com python
// Criar função de Ler imagem (Txt) (RGB e Gray)
// Criar função de Ler imagem (png) (RGB e Gray)
// TODO Criar função de Salvar imagem (Txt e Gerar imagem) (RGB e Gray)

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
    return largura * i + j;
*/

//////////////////// Funções auxiliares ////////////////////

// Função para converter uma posição de matriz em posição de vetor
int posicaoVetor(int largura, int i, int j)
{

}


// Funções para alocar um vetor de pixels
PixelRGB *alocarPixelRGB(int tam)
{

}

PixelGray *alocarPixelGray(int tam)
{

}


////////////// Funções de criação e liberação //////////////
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


////////////// Funções para leitura e salvamento //////////////

ImageGray *LerImagemGray(char *caminho)
{

}

ImageRGB *LerImagemRGB(char *caminho)
{

}


void *SalvarTxtGray(ImageGray *imagem)
{

}

void *SalvarTxtRGB(ImageRGB *imagem)
{

}


void *SalvarImagemGray(ImageGray *imagem)
{

}

void *SalvarImagemRGB(ImageRGB *imagem)
{

}


////////////////// Funções para Operações //////////////////

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


///////////// Funções de Manipulação por Pixel /////////////

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

////////////////////////////////////////////////////////////