#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> // Biblioteca para verificar pastas
#include <sys/stat.h> // Biblioteca para criar pastas
#include <sys/types.h> // Biblioteca para especificar os bits de permissão da pasta criada

// TODO Criar função de se comunicar com python
// TODO O caminho será o caminho relativo até a pasta. Nome será o nome do arquivo, junto da sua extensão

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
    snprintf(caminho, tam, "imagens/numero da execução");
    snprintf(caminho, tam, "imagens/%d", global);

    printf("%s", comando);
    return 0;
}
*/

//////////////////// Funções auxiliares ////////////////////

// Função para converter uma posição de matriz em posição de vetor
int posicaoVetor(int largura, int i, int j)
{
    return largura * i + j;
}

FILE *lerArquivo(char *caminho, char *modo)
{
    FILE *arquivo = fopen(caminho, modo);

    if(!arquivo)
    {
        prinf("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    return arquivo;
}

char *alocarStr(int tam)
{
    char *vetor = (char *) malloc(tam * sizeof(char));

    if(!vetor)
    {
        printf("Erro ao alocar string");
        exit(EXIT_FAILURE);
    }

    return vetor;
}

// Funções para alocar um vetor de pixels
PixelRGB *alocarPixelRGB(int tam)
{
    PixelRGB *vetor = (PixelRGB *) malloc(tam * sizeof(PixelRGB));

    if(!vetor)
    {
        printf("Erro ao alocar pixel RGB");
        exit(EXIT_FAILURE);
    }

    return vetor;
}

PixelGray *alocarPixelGray(int tam)
{
    PixelGray *vetor = (PixelGray *) malloc(tam * sizeof(PixelGray));

    if(!vetor)
    {
        printf("Erro ao alocar pixel Gray");
        exit(EXIT_FAILURE);
    }

    return vetor;
}


void liberarVetor(void **vetor)
{
    free(*vetor);
    *vetor = NULL;
}

char *gerarCaminho(char *pasta, char *nome)
{
    int tamanho = 128;
    char caminho = alocarStr(tamanho);

    snprintf(caminho, tamanho, "%s/%s", pasta, nome);

    return caminho;
}

void criarPasta(char *caminho)
{
    if (mkdir(caminho, 0755) != 0)
    {
        printf("Erro ao criar pasta");
        exit(EXIT_FAILURE);
    }
}

////////////// Funções de criação e liberação //////////////

// Funções para criar a variável de imagem
ImageGray *create_image_gray(int largura, int altura)
{
    ImageGray *imagem = (ImageGray *) malloc(sizeof(ImageGray));

    if(!imagem)
    {
        printf("Erro ao alocar imagem Gray");
        exit(EXIT_FAILURE);
    }

    imagem->dim.altura = altura;
    imagem->dim.largura = largura;
    imagem->pixels = alocarPixelGray(largura * altura);

    return imagem;
}

void free_image_gray(ImageGray *image)
{
    liberarVetor(&image->pixels);
    image->dim.altura = 0;
    image->dim.largura = 0;
    liberarVetor(&image);
}


ImageRGB *create_image_rgb(int largura, int altura)
{
    ImageRGB *imagem = (ImageRGB *) malloc(sizeof(ImageRGB));

    if(!imagem)
    {
        printf("Erro ao alocar imagem RGB");
        exit(EXIT_FAILURE);
    }

    imagem->dim.altura = altura;
    imagem->dim.largura = largura;
    imagem->pixels = alocarPixelRGB(largura * altura);

    return imagem;
}

void free_image_rgb(ImageRGB *image)
{
    liberarVetor(&image->pixels);
    image->dim.altura = 0;
    image->dim.largura = 0;
    liberarVetor(&image);
}


////////////// Funções para leitura e salvamento //////////////

ImageGray *lerTxtGray(char *caminho)
{
    FILE *arquivo = lerArquivo(caminho, "r");
    
    int altura, largura;
    fscanf(arquivo, "%d", &altura);
    fgetc(arquivo);
    fscanf(arquivo, "%d", &largura);
    fgetc(arquivo);

    ImageGray *imagem = create_image_gray(largura, altura);

    for(int i = 0; i < altura; i++)
    {
        for(int j = 0; j < largura; j++)
        {
            fscanf(arquivo, "%d", &imagem->pixels[posicaoVetor(largura, i, j)]);
            fgetc(arquivo);
        }
        fgetc(arquivo);
    }

    fclose(arquivo);
    return imagem;
}

ImageRGB *lerTxtRGB(char *caminho)
{

}


// Falta completar [Python]
ImageGray *lerImagemGray(char *caminho)
{
    // Utilizar a função txt from image gray

    return lerTxtGray(caminho);
}

ImageRGB *lerImagemRGB(char *caminho)
{

}


void salvarTxtGray(ImageGray *imagem, char *caminho, char *nome)
{
    DIR *pasta = opendir(caminho);

    if(pasta)
        closedir(pasta);
    else
        criarPasta(caminho);

    caminho = gerarCaminho(caminho, nome);

    FILE *arquivo = lerArquivo(caminho, "w");

    fprintf(arquivo, "%d,", imagem->dim.altura);
    fputc(arquivo, '\n');
    fprintf(arquivo, "%d,", imagem->dim.largura);
    fputc(arquivo, '\n');

    for(int i = 0; i < imagem->dim.altura; i++)
    {
        for(int j = 0; j < imagem->dim.largura; j++)
        {
            fprintf(arquivo, "%d,", imagem->pixels[posicaoVetor(imagem->dim.largura, i, j)]);
            fputc(arquivo, '\n');
        }
        fputc(arquivo, '\n');
    }

    liberarVetor(&caminho);
    fclose(arquivo);
}

// 
void salvarTxtRGB(ImageRGB *imagem)
{

}


// Falta completar [Python]
void salvarImagemGray(ImageGray *imagem, char *caminho, char *nome)
{
    salvarTxtGray(imagem, caminho, nome);

    caminho = gerarCaminho(caminho, nome);
    
    // Utilizar a função image gray from txt

    liberarVetor(&caminho);
}

// 
void salvarImagemRGB(ImageRGB *imagem)
{

}


////////////////// Funções para Operações //////////////////

// Operações para ImageGray
ImageGray *flip_vertical_gray(ImageGray *image)
{
    //TESTE 2
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