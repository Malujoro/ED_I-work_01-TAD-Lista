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
        printf("Erro ao abrir o arquivo");
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

int *alocarInt(int tam)
{
    int *vetor = (int *) calloc(tam, sizeof(int));

    if(!vetor)
    {
        printf("Erro ao alocar inteiro");
        exit(EXIT_FAILURE);
    }

    return vetor;
}

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
    char *caminho = alocarStr(tamanho);

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

int mediana2(int *vetor, int tam)
{
    int aux, menor, pos;
    for(int i = 0; i <= tam/2; i++)
    {
        menor = vetor[i];
        pos = i;
        for(int j = i+1; j < tam; j++)
        {
            if(vetor[j] < menor)
            {
                menor = vetor[j];
                pos = j;
            }
        }
        aux = vetor[pos];
        vetor[pos] = vetor[i];
        vetor[i] = aux;
    }
    return vetor[tam/2];
}

int mediana(int *vetor, int tam)
{
    int *gray = alocarInt(256);

    for(int i = 0; i < tam; i++)
        gray[vetor[i]]++;

    // 5 * 5 = 25
    // 25 / 2 = 12
    for(int i = 0, valor = 0; i < 256; i++)
    {
        valor += gray[i];
        if(valor >= tam/2)
            return i;
    }

    liberarVetor(&gray);
    return 0;
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
            fscanf(arquivo, "%d", &imagem->pixels[posicaoVetor(largura, i, j)].value);
            fgetc(arquivo);
        }
        fgetc(arquivo);
    }

    fclose(arquivo);
    return imagem;
}

// TODO RGB futuro
// ImageRGB *lerTxtRGB(char *caminho)
// {

// }


// TODO Falta completar [Python]
ImageGray *lerImagemGray(char *caminho)
{
    // Utilizar a função txt from image gray

    return lerTxtGray(caminho);
}

// TODO RGB futuro
// ImageRGB *lerImagemRGB(char *caminho)
// {

// }


void salvarTxtGray(ImageGray *imagem, char *caminho, char *nome)
{
    DIR *pasta = opendir(caminho);

    if(pasta)
        closedir(pasta);
    else
        criarPasta(caminho);

    caminho = gerarCaminho(caminho, nome);

    FILE *arquivo = lerArquivo(caminho, "w");

    fprintf(arquivo, "%d", imagem->dim.altura);
    fputc('\n', arquivo);
    fprintf(arquivo, "%d", imagem->dim.largura);
    fputc('\n', arquivo);

    for(int i = 0; i < imagem->dim.altura; i++)
    {
        for(int j = 0; j < imagem->dim.largura; j++)
        {
            fprintf(arquivo, "%d,", imagem->pixels[posicaoVetor(imagem->dim.largura, i, j)].value);
        }
        fputc('\n', arquivo);
    }

    liberarVetor(&caminho);
    fclose(arquivo);
}

// TODO RGB futuro
// void salvarTxtRGB(ImageRGB *imagem)
// {

// }


// TODO Falta completar [Python]
void salvarImagemGray(ImageGray *imagem, char *caminho, char *nome)
{
    salvarTxtGray(imagem, caminho, nome);

    caminho = gerarCaminho(caminho, nome);
    
    // Utilizar a função image gray from txt

    liberarVetor(&caminho);
}

// TODO RGB futuro
// void salvarImagemRGB(ImageRGB *imagem)
// {

// }


////////////////// Funções para Operações //////////////////

// // Operações para ImageGray
// ImageGray *flip_vertical_gray(ImageGray *image)
// {

// }

// ImageGray *flip_horizontal_gray(ImageGray *image)
// {

// }

// ImageGray *transpose_gray(const ImageGray *image)
// {

// }


// // Operações para ImageRGB
// ImageRGB *flip_vertical_rgb(const ImageRGB *image)
// {

// }

// ImageRGB *flip_horizontal_rgb(const ImageRGB *image)
// {

// }

// ImageRGB *transpose_rgb(const ImageRGB *image)
// {

// }


///////////// Funções de Manipulação por Pixel /////////////

// Manipulação por pixel para ImageGray
// ImageGray *clahe_gray(const ImageGray *image, int tile_width, int tile_height)
// {
    // int quantY = image->dim.altura / kernel_size;
    // int quantX = image->dim.largura / kernel_size;
// }

ImageGray *median_blur_gray(const ImageGray *image, int kernel_size)
{
    if(kernel_size % 2 == 0)
    {
        printf("Kernel size deve ser ímpar");
        return NULL;
    }

    ImageGray *blur = create_image_gray(image->dim.largura, image->dim.altura);
    for(int i = 0; i < image->dim.largura * image->dim.altura; i++)
        blur->pixels[i].value = image->pixels[i].value;

    int tam = kernel_size / 2, quant, meio;
    int quantY = image->dim.altura - (tam * 2);
    int quantX = image->dim.largura - (tam * 2);
    int *vetor = alocarInt(kernel_size * kernel_size);
    printf("\n[%d]\n", quantY * quantX);
    for(int i = 0; i < quantY; i++)
    {
        for(int j = 0; j < quantX; j++)
        {
            quant = 0;
            meio = posicaoVetor(image->dim.largura, tam + i, tam + j);
            for(int i2 = 0; i2 < kernel_size; i2++)
            {
                for(int j2 = 0; j2 < kernel_size; j2++)
                {
                    vetor[quant] = image->pixels[posicaoVetor(image->dim.largura, i2 + i, j2 + j)].value;
                    quant++;
                }
            }
            blur->pixels[meio].value = mediana(vetor, quant);
        }
    }

    return blur;
}


// // Manipulação por pixel para ImageRGB
// ImageRGB *clahe_rgb(const ImageRGB *image, int tile_width, int tile_height)
// {

// }

// ImageRGB *median_blur_rgb(const ImageRGB *image, int kernel_size)
// {

// }

////////////////////////////////////////////////////////////