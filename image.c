#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> // Biblioteca para verificar pastas
#include <sys/stat.h> // Biblioteca para criar pastas
#include <sys/types.h> // Biblioteca para especificar os bits de permissão da pasta criada

// TODO Criar função de se comunicar com python
// TODO O caminho será o caminho relativo até a pasta. Nome será o nome do arquivo, junto da sua extensão

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

void *liberarVetor(void *vetor)
{
    free(vetor);
    return NULL;
}

// Função para converter um Inteiro em String
char *intParaStr(int num)
{
    int tam, quant;
    for(tam = 1, quant = 1; tam*10 <= num; tam *= 10, quant++);
    printf("tam = [%d] | quant = [%d]\n", tam, quant);

    char *result = alocarStr(quant);

    for(int i = 0; i < quant; i++)
    {
        result[i] = '0' + num / tam;
        num %= tam;
        tam /= 10;
    }

    return result;
}

char *gerarCaminho(char *pasta, char *nome, char *tipo)
{
    int tamanho = 128;
    char *caminho = alocarStr(tamanho);

    snprintf(caminho, tamanho, "%s%s%s", pasta, tipo, nome);

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

int pastaExiste(char *caminho)
{
    DIR *pasta = opendir(caminho);

    if(pasta)
    {
        closedir(pasta);
        return 1;
    }

    criarPasta(caminho);
    return 0;
}

int contarPastas(char *caminho)
{
    DIR *pasta = opendir(caminho);

    if(!pasta)
    {
        printf("Erro ao abrir pasta");
        exit(EXIT_FAILURE);
    }

    struct dirent *entrada = readdir(pasta);
    int quant = -1;

    while(entrada)
    {
        quant++;
        entrada = readdir(pasta);
    }

    closedir(pasta);
    return quant;
}

int mediana(int *vetor, int tam)
{
    int *gray = alocarInt(256);

    for(int i = 0; i < tam; i++)
        gray[vetor[i]]++;

    for(int i = 0, valor = 0; i < 256; i++)
    {
        valor += gray[i];
        if(valor >= tam/2)
        {
            gray = liberarVetor(gray);
            return i;
        }
    }

    gray = liberarVetor(gray);
    return 0;
}

// TODO [arrumar função!!]
void python(char *origem, char *tipo, char *cor, char *pasta, char *nome, char *extensao)
{
    int tam = 256, quant = contarPastas(pasta);
    char comando[tam];

    if(tipo[0] == 't')
        quant--;
    
    char *num = intParaStr(quant);
    char *caminho = gerarCaminho(pasta, num, "/");
    pastaExiste(caminho);
    num = liberarVetor(num);

    snprintf(comando, tam, "python3 utils/image_utils.py %s %s %s %s/%s.%s", tipo, cor, origem, caminho, nome, extensao);

    system(comando);
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
    image->pixels = liberarVetor(image->pixels);
    image->dim.altura = 0;
    image->dim.largura = 0;
    image = liberarVetor(image);
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
    image->pixels = liberarVetor(image->pixels);
    image->dim.altura = 0;
    image->dim.largura = 0;
    image = liberarVetor(image);
}


ImageGray *copiarImagemGray(const ImageGray *image)
{
    ImageGray *copia = create_image_gray(image->dim.largura, image->dim.altura);

    for(int i = 0; i < image->dim.largura * image->dim.altura; i++)
        copia->pixels[i].value = image->pixels[i].value;
        
    return copia;
}

ImageRGB *copiarImagemRGB(const ImageRGB *image)
{
    ImageRGB *copia = create_image_rgb(image->dim.largura, image->dim.altura);

    for(int i = 0; i < image->dim.largura * image->dim.altura; i++)
    {
        copia->pixels[i].red = image->pixels[i].red;
        copia->pixels[i].green = image->pixels[i].green;
        copia->pixels[i].blue = image->pixels[i].blue;
    }
        
    return copia;
}

////////////// Funções para leitura e salvamento //////////////

ImageGray *lerTxtGray(char *pasta, char *nome)
{
    char *caminho = gerarCaminho(pasta, nome, "/");
    caminho = gerarCaminho(caminho, "txt", ".");
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
ImageGray *lerImagemGray(char *origem, char *pasta, char *nome)
{
    // Utilizar a função txt from image gray
    python(origem, "png", "gray", pasta, nome, "txt");

    return lerTxtGray(pasta, nome);
}

// TODO RGB futuro
// ImageRGB *lerImagemRGB(char *caminho)
// {

// }


void salvarTxtGray(ImageGray *imagem, char *caminho, char *nome)
{
    pastaExiste(caminho);
    caminho = gerarCaminho(caminho, nome, "/");
    caminho = gerarCaminho(caminho, "txt", ".");
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

    // caminho = liberarVetor(caminho);
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

    // python(caminhoAtual, "txt", "gray", caminho, nome, "png");

    // caminho = liberarVetor(caminho);
}

// TODO RGB futuro
// void salvarImagemRGB(ImageRGB *imagem)
// {

// }


////////////////// Funções para Operações //////////////////

// // Operações para ImageGray
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

    ImageGray *blur = copiarImagemGray(image);

    int tam = kernel_size / 2, meio, quant;
    int *vetor = alocarInt(kernel_size * kernel_size);
    for(int i = 0; i < image->dim.altura; i++)
    {
        for(int j = 0; j < image->dim.largura; j++)
        {
            quant = 0;
            for(int i2 = 0, posY = i - tam; i2 < kernel_size; i2++, posY++)
            {
                if(posY < 0)
                    posY += 512;
                else if(posY >= 512)
                    posY -= 512;

                for(int j2 = 0, posX = j - tam; j2 < kernel_size; j2++, posX++, quant++)
                {
                    if(posX < 0)
                        posX += 512;
                    else if(posX >= 512)
                        posX -= 512;
                        
                    vetor[quant] = image->pixels[posicaoVetor(image->dim.largura, posY, posX)].value;
                }
            }
            meio = posicaoVetor(image->dim.largura, i, j);
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