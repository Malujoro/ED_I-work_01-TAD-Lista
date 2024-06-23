#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> // Biblioteca para verificar pastas
#include <sys/stat.h> // Biblioteca para criar pastas
#include <sys/types.h> // Biblioteca para especificar os bits de permissão da pasta criada
#include <python3.10/Python.h> // API para utilizar o python em C
#include <locale.h> //Biblioteca para adicionar os emoji (usados nas funções menuRotate e menuTranspose)
#include <string.h> //Biblioteca para usar strdup //possívelmente temporária

#define SCRIPT 0
#define FUNCAO 1
#define ARGUMENTOS 2

int clip_limit = 40;
// TODO Criar função de se comunicar com python
// TODO O caminho será o caminho relativo até a pasta. Nome será o nome do arquivo, junto da sua extensão

//struct dos nós da lista duplamente encadeada, usada no histórico
typedef struct historico_gray
{
    ImageGray *img;
    char *nome;
    struct historico_gray *prox;
    struct historico_gray *ant;
} Historico_Gray;

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

float *alocarFloat(int tam)
{
    float *vetor = (float *) calloc(tam, sizeof(float));

    if(!vetor)
    {
        printf("Erro ao alocar float");
        exit(EXIT_FAILURE);
    }

    return vetor;
}

PyObject **alocarPython(int tam)
{
    PyObject **matriz = (PyObject **) malloc(tam * sizeof(PyObject *));

    if(matriz != NULL)
    {
        // for(int i = 0; i < tam; i++)
        // {
        //     matriz[i] = (PyObject *) malloc(sizeof(PyObject));

        //     if(!matriz[i])
        //     {
        //         printf("Erro ao alocar PyObject");
        //         exit(EXIT_FAILURE);
        //     }        
        // }
        return matriz;
    }
    printf("Erro ao alocar PyObject");
    exit(EXIT_FAILURE);
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

void limparFloat(float *vetor, int tam)
{
    for(int i = 0; i < tam; i++)
        vetor[i] = 0;
}

// Função para converter um Inteiro em String
char *intParaStr(int num)
{
    int tam, quant;
    for(tam = 1, quant = 1; tam*10 <= num; tam *= 10, quant++);

    char *result = alocarStr(quant);

    for(int i = 0; i < quant; i++)
    {
        result[i] = '0' + num / tam;
        num %= tam;
        tam /= 10;
    }

    return result;
}

char *gerarCaminho(char *pasta, char *simbolo, char *nome)
{
    int tamanho = 128;
    char *caminho = alocarStr(tamanho);

    snprintf(caminho, tamanho, "%s%s%s", pasta, simbolo, nome);

    return caminho;
}

DIR *abrirPasta(char *caminho)
{
    DIR *pasta = opendir(caminho);

    if(!pasta)
    {
        printf("Erro ao abrir pasta");
        exit(EXIT_FAILURE);
    }
    
    return pasta;
}

void criarPasta(char *caminho)
{
    if(mkdir(caminho, 0755) != 0)
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
    return 0;
}

int contarPastas(char *caminho)
{
    DIR *pasta = abrirPasta(caminho);

    struct dirent *entrada;
    int quant = -1;

    for(entrada = readdir(pasta); entrada; entrada = readdir(pasta))
    {
        if(entrada->d_type == DT_DIR)
            quant++;
    }

    closedir(pasta);
    return quant;
}

char *pastaPrincipal(char *caminho)
{
    char *num = intParaStr(contarPastas(caminho));
    char *caminho2 = gerarCaminho(caminho, "/", num);

    if(!pastaExiste(caminho2))
        criarPasta(caminho2);

    num = liberarVetor(num);
    return caminho2;
}

// Função para inicializar o interpretador python
PyObject **inicializaPython(char *funcao, char *image_path, char *output_path, int gray)
{
    // Inicializa o interpretador python
    Py_Initialize();

    // Adiciona o diretório atual ao caminho do sistema, permitindo que o script seja encontrado
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\".\")");

    PyObject **matriz = alocarPython(3);

    // "Inclui" o script cujo caminho é utils/image_utils.py
    PyObject *nomeScript = PyUnicode_DecodeFSDefault("utils.image_utils");
    matriz[SCRIPT] = PyImport_Import(nomeScript);
    Py_DECREF(nomeScript); // Libera a memória de um PyObject

    if (matriz[SCRIPT] != NULL)
    {
        // "Importa" a função passada como parâmetro
        matriz[FUNCAO] = PyObject_GetAttrString(matriz[SCRIPT], funcao);

        // Verifica se a função é "chamável"
        if (PyCallable_Check(matriz[FUNCAO]))
            return matriz;
        
        Py_DECREF(matriz[SCRIPT]);
    }
    PyErr_Print();
    exit(EXIT_FAILURE);
}

// Função para finalizar o interpretador python
void executaPython(PyObject **matriz)
{
    PyObject *retorno = PyObject_CallObject(matriz[FUNCAO], matriz[ARGUMENTOS]);

    if (!retorno)
    {
        PyErr_Print();
        exit(EXIT_FAILURE);
    }

    Py_DECREF(retorno);
}

// Função para finalizar o interpretador python
void finalizaPython(PyObject **matriz)
{
    Py_DECREF(matriz[ARGUMENTOS]);
    Py_DECREF(matriz[FUNCAO]);
    Py_DECREF(matriz[SCRIPT]);
    free(matriz);

    // Finaliza o interpretador
    Py_Finalize();
}


void txt_from_image(char *image_path, char *output_path, int gray)
{
    PyObject **matriz = inicializaPython("txt_from_image_gray", image_path, output_path, gray);

    // Organiza os argumentos da função
    matriz[ARGUMENTOS] = PyTuple_Pack(3, PyUnicode_FromString(image_path), PyUnicode_FromString(output_path), PyLong_FromLong(gray));

    // Chama a função Python e obtém o resultado
    executaPython(matriz);
    finalizaPython(matriz);
}

void image_from_txt(char *txt_path, char *output_path, int gray)
{
    char *nome;

    if(gray)
        nome = "image_gray_from_txt";
    else
        nome = "image_rgb_from_txt";

    PyObject **matriz = inicializaPython(nome, txt_path, output_path, gray);

    // Organiza os argumentos da função
    matriz[ARGUMENTOS] = PyTuple_Pack(2, PyUnicode_FromString(txt_path), PyUnicode_FromString(output_path));

    // Chama a função Python e obtém o resultado
    executaPython(matriz);
    finalizaPython(matriz);
}

///////// Auxiliar Median Blur /////////

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

//////////// Auxiliar Clahe ////////////

float cdf(float *vetor, int pos)
{
    float soma = 0;
    for(int i = 0; i <= pos; i++)
        soma += vetor[i];
    return soma;
}

float cdf_normalizado(float cdf_i, float cdf_min, float cdf_max)
{
    return (cdf_i - cdf_min) / (cdf_max - cdf_min) * 255;
}

void redistribuirHistograma(float *histograma)
{
    int limite;
    float soma, somaTotal;
    do
    {
        limite = 0;
        soma = 0;
        somaTotal = 0;

        // Redistribuir os valores enquanto algum passar do limite
        // Somar os passados
        for(int i = 0; i < 256; i++)
        {
            somaTotal += histograma[i];
            if(histograma[i] > clip_limit)
            {
                limite = 1;
                soma += histograma[i] - clip_limit;
                histograma[i] = clip_limit;
            }
        }

        // Blindagem contra looping infinito (impossível de redistribuir)
        if(somaTotal >= 256 * clip_limit || soma <= 0.001)
            limite = 0;

        // Distribuir os valores igualmente
        if(limite)
        {
            for(int i = 0; i < 256; i++)
                histograma[i] += soma / 256;
        }
    }while(limite);
}

int posMinimo(float *histograma)
{
    for(int i = 0; i < 256; i++)
    {
        if(histograma[i] != 0)
            return i;
    }
    return 0;
}

int posMaximo(float *histograma)
{
    for(int i = 255; i >= 0; i--)
    {
        if(histograma[i] != 0)
            return i;
    }
    return 255;
}

void suavizaLinhaGray(ImageGray *image, int height)
{
    float media, pixel1, pixel2;
    int aux1, aux2;
    int quant = image->dim.altura / height;
    
    // Se o tamanho das caixas for exato, a quantidade de bordas pode decrementar 1
    if(image->dim.altura % height == 0)
        quant--;

    aux1 = height - 1;
    aux2 = height;

    for(int i = 0; i < quant; i++)
    {
        for(int j = 0; j < image->dim.largura; j++)
        {
            pixel1 = image->pixels[posicaoVetor(image->dim.largura, aux1, j)].value;
            pixel2 = image->pixels[posicaoVetor(image->dim.largura, aux2, j)].value;
            media = (pixel1 + pixel2) / 2;
            image->pixels[posicaoVetor(image->dim.largura, aux1, j)].value = media;
            image->pixels[posicaoVetor(image->dim.largura, aux2, j)].value = media;
        }
        aux1 += height;
        aux2 += height;
    }
}

void suavizaColunaGray(ImageGray *image, int width)
{
    float media, pixel1, pixel2;
    int aux1, aux2;
    int quant = image->dim.largura / width;
    
    // Se o tamanho das caixas for exato, a quantidade de bordas pode decrementar 1
    if(image->dim.largura % width == 0)
        quant--;

    aux1 = width - 1;
    aux2 = width;

    for(int i = 0; i < quant; i++)
    {
        for(int j = 0; j < image->dim.altura; j++)
        {
            pixel1 = image->pixels[posicaoVetor(image->dim.largura, j, aux1)].value;
            pixel2 = image->pixels[posicaoVetor(image->dim.largura, j, aux2)].value;
            media = (pixel1 + pixel2) / 2;
            image->pixels[posicaoVetor(image->dim.largura, j, aux1)].value = media;
            image->pixels[posicaoVetor(image->dim.largura, j, aux2)].value = media;
        }
        aux1 += width;
        aux2 += width;
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
ImageGray *lerImagemGray(char *png, char *txt)
{
    txt_from_image(png, txt, 1);
    return lerTxtGray(txt);
}

// TODO RGB futuro
// ImageRGB *lerImagemRGB(char *caminho)
// {

// }


void salvarTxtGray(ImageGray *imagem, char *caminho, char *txt)
{       
    if(!pastaExiste(caminho))
        criarPasta(caminho);

    FILE *arquivo = lerArquivo(txt, "w");

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

    fclose(arquivo);
}

// TODO RGB futuro
// void salvarTxtRGB(ImageRGB *imagem)
// {

// }


// TODO Falta completar [Python]
void salvarImagemGray(ImageGray *imagem, char *caminho, char *txt, char *png)
{
    if(!pastaExiste(caminho))
        criarPasta(caminho);

    salvarTxtGray(imagem, caminho, txt);

    image_from_txt(txt, png, 1);
}

// TODO RGB futuro
// void salvarImagemRGB(ImageRGB *imagem)
// {

// }


////////////////// Funções para Operações //////////////////

// // Operações para ImageGray

ImageGray *flip_vertical_gray(const ImageGray *image)
{
    ImageGray *imageFlipV = create_image_gray(image->dim.largura, image->dim.altura);

    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++){
            imageFlipV->pixels[posicaoVetor(image->dim.largura, i, j)] = image->pixels[posicaoVetor(image->dim.largura, (image->dim.altura - i - 1), j)];
        }
    }

    return imageFlipV;
}

ImageGray *flip_horizontal_gray(const ImageGray *image)
{
    ImageGray *imageFlipH = create_image_gray(image->dim.largura, image->dim.altura);

    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++){
            imageFlipH->pixels[posicaoVetor(image->dim.largura, i, j)] = image->pixels[(image->dim.largura * i) + (image->dim.largura - j - 1)];
        }
    }

    return imageFlipH;
}


int menuRotate(){
    int op;
    setlocale(LC_ALL,"");

    printf("Menu de opções de Rotate:\n");
    do{
        printf("1- Rotacionar no sentido horário 🔁\n");  //\U0001F504
        printf("2- Rotacionar no sentido anti-horário 🔄\n");  //\U0001F504
        printf("Escolha: ");
        if(scanf("%d", &op) != 1 || (op != 1 && op != 2)){
            while(getchar() != '\n');
            printf("Entrada inválida. Por favor, escolha 1 ou 2.\n");
        }
        else
            break;
    } while(1);

    return op;  
}

ImageGray *rotate_90_gray(const ImageGray *image)
{
    ImageGray *imageRotate = create_image_gray(image->dim.altura, image->dim.largura);

    int op = menuRotate();

    switch(op){
        case 1:
            //Rotacionar no sentido horário:
            for(int i = 0; i < image->dim.altura; i++){
                for(int j = 0; j < image->dim.largura; j++)
                    imageRotate->pixels[posicaoVetor(imageRotate->dim.largura, j, (imageRotate->dim.largura - 1 - i))] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
            }
            break;
        case 2:
            //Rotacionar no sentido anti horário
            for(int i = 0; i < image->dim.altura; i++){
                for(int j = 0; j < image->dim.largura; j++)
                    imageRotate->pixels[posicaoVetor(imageRotate->dim.largura, imageRotate->dim.altura, i) - imageRotate->dim.largura * (j + 1)] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
            }
            break;
    }

    return imageRotate;
}


int menuTranspose(){
    setlocale(LC_ALL,"");

    int op;

    printf("Menu de opções de transpose:\n");
    do{
        printf("1- Transpose ↗️\n");  //\u2197
        printf("2- Transpose ↘️\n");  //\u2198
        printf("Escolha: ");
        if(scanf("%d", &op) != 1 || (op != 1 && op != 2)){
            while (getchar() != '\n');
            printf("Entrada inválida. Por favor, escolha 1 ou 2.\n");
        }
        else
            break;
    } while (1);

    return op;
}

ImageGray *transpose_gray(const ImageGray *image)
{
    ImageGray *imageTranspose = create_image_gray(image->dim.altura, image->dim.largura);

    int op = menuTranspose();

    switch (op){
        case 1:
            //Transpose - inverte diagonais direita superior e esqueda inferior
            for(int i = 0; i < image->dim.altura; i++){
            for(int j = 0; j < image->dim.largura; j++)
                imageTranspose->pixels[posicaoVetor(image->dim.altura, j, i)] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
            }
            break;
        case 2:
            //Transpose - inverte diagonais esquerda superior e direita inferior
            for(int i = 0; i < image->dim.altura; i++){
                for(int j = 0; j < image->dim.largura; j++)
                    imageTranspose->pixels[posicaoVetor(image->dim.altura, (image->dim.largura - j - 1), (image->dim.altura - i - 1))] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
            }
            break;
    }
    
    return imageTranspose;
}


// // Operações para ImageRGB
// ImageRGB *flip_vertical_rgb(const ImageRGB *image)
// {

// }

// ImageRGB *flip_horizontal_rgb(const ImageRGB *image)
// {

// }

// ImageRGB *rotate_90_rgb(const ImageRGB *image)
// {

// }

// ImageRGB *transpose_rgb(const ImageRGB *image)
// {

// }


///////////// Funções de Manipulação por Pixel /////////////

// Manipulação por pixel para ImageGray
ImageGray *clahe_gray(const ImageGray *image, int tile_width, int tile_height)
{
    int caixaX, caixaY;

    caixaX = image->dim.largura / tile_width;
    if(image->dim.largura % tile_width != 0)
        caixaX++;

    caixaY = image->dim.altura / tile_height;
    if(image->dim.altura % tile_height != 0)
        caixaY++;

    ImageGray *resultado = create_image_gray(image->dim.largura, image->dim.altura);
    int tamVet = tile_height * tile_width;
    float *vetor = alocarFloat(tamVet);
    float *histograma = alocarFloat(256);
    for(int a = 0; a < caixaY; a++)
    {
        for(int b = 0, tam = 0; b < caixaX; b++, tam = 0)
        {
            // Monta o Tile
            for(int i = 0, posI = a * tile_height; i < tile_height && posI < image->dim.altura; i++, posI++)
            {
                for(int j = 0, posJ = b * tile_width; j < tile_width && posJ < image->dim.largura; j++, posJ++, tam++)
                    vetor[tam] = image->pixels[posicaoVetor(image->dim.largura, posI, posJ)].value;
            }
            
            // Monta o Histograma
            for(int i = 0; i < tam; i++)
                histograma[(int) vetor[i]]++;

            redistribuirHistograma(histograma);

            // Encontrar mínimo e máximo
            int minimo = cdf(histograma, posMinimo(histograma));
            int maximo = cdf(histograma, posMaximo(histograma));

            // Remapeia todo o bloco
            if(maximo != minimo)
            {
                for(int i = 0, tam = 0, posI = a * tile_height; i < tile_height && posI < image->dim.altura; i++, posI++)
                {
                    for(int j = 0, posJ = b * tile_width; j < tile_width && posJ < image->dim.largura; j++, posJ++, tam++)
                        resultado->pixels[posicaoVetor(image->dim.largura, posI, posJ)].value = cdf_normalizado(cdf(histograma, vetor[tam]), minimo, maximo);
                }
            }

            limparFloat(vetor, tam);
            limparFloat(histograma, 256);
        }
    }
    histograma = liberarVetor(histograma);
    vetor = liberarVetor(vetor);

    suavizaColunaGray(resultado, tile_width);
    suavizaLinhaGray(resultado, tile_height);

    return resultado;
}

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

ImageGray *negativo_gray(const ImageGray *image)
{
    ImageGray *result = create_image_gray(image->dim.largura, image->dim.altura);

    for(int i = 0; i < result->dim.altura * result->dim.largura; i++)
        result->pixels[i].value = 255 - image->pixels[i].value;
    
    return result;
}

// // Manipulação por pixel para ImageRGB
// ImageRGB *clahe_rgb(const ImageRGB *image, int tile_width, int tile_height)
// {

// }

// ImageRGB *median_blur_rgb(const ImageRGB *image, int kernel_size)
// {

// }

////////////////////////////////////////////////////////////

//////////////////   Funções para as operações do Histórico   //////////////////// 
Historico_Gray *criar_lista(){
    return NULL;
}

//cria um novo elemento para o historico
Historico_Gray *criar_No(){
    Historico_Gray *no = (Historico_Gray *) malloc(sizeof(Historico_Gray));
    if(!no){
        printf("Erro ao alocar o novo nó.");
        exit(EXIT_FAILURE);
    }

    no->prox = NULL;
    no->ant = NULL;

    return no;
}

//adiciona a ultima edição ao final do historico
Historico_Gray *add_historico(ImageGray *image, Historico_Gray *l, const char *nome){
    Historico_Gray *novo = criar_No();
    novo->img = image;
    novo->nome = strdup(nome);  //cria uma cópia de nome

    if(l == NULL)
        return novo;
    
    Historico_Gray *aux = l;
    while(aux->prox != NULL)
        aux = aux->prox;

    aux->prox = novo;
    novo->ant = aux;

    return novo;
}

//percorre para a proxima edição
Historico_Gray *next_Image(Historico_Gray *atual){
    if(atual != NULL && atual->prox != NULL){
        atual = atual->prox;
        printf("Imagem atual:\n");
        printf("\"%s\"\n\n", atual->nome);
    }
    else{
        printf("Você chegou a ultima imagem.\n");
        printf("\"%s\"\n", atual->nome);
    }
    return atual;
}

//percorre para a edição anterior
Historico_Gray *prev_Image(Historico_Gray *atual){
    if(atual && atual->ant != NULL){
        atual = atual->ant;
        printf("Imagem atual:\n");
        printf("\"%s\"\n\n", atual->nome);
    }
    else{
        printf("Você chegou a ultima imagem.\n");
        printf("\"%s\"\n", atual->nome);
    }
    return atual;
}

//libera a memória do historico de imagens
void free_Historico(Historico_Gray *l){
    Historico_Gray *aux;

    while(l != NULL){
        aux = l;
        l = l->prox;
        free_image_gray(aux->img);
        aux->nome = liberarVetor(aux->nome);
        free(aux);
    }
}
///////////////////////////////////////////////////////////////

////////////////////// SALVAR IMAGEM //////////////////////

void salvar(ImageGray *image, char *pasta, char *nome){
    char *caminho = gerarCaminho(pasta, "/", nome);
    char *txt = gerarCaminho(caminho, ".", "txt");
    char *png = gerarCaminho(caminho, ".", "png");

    salvarImagemGray(image, pasta, txt, png);
    
    system("clear");
    printf("Imagem salva com sucesso...\nPressione qualquer tecla para continuar...\n");
    while (getchar() != '\n');
    getchar();
    system("clear");

    caminho = liberarVetor(caminho);
    txt = liberarVetor(txt);
    png = liberarVetor(png);
}

void salvarTudo(Historico_Gray *l, char *pasta){
    Historico_Gray *aux = l;

    while(aux->ant->ant != NULL)
        aux = aux->ant;

    while(aux != NULL){
        printf("Salvando imagem \"%s\"...\n\n", aux->nome);
        salvar(aux->img, pasta, aux->nome);
        aux = aux->prox;
    }

    system("clear");
    printf("Todas as imagens foram salvas com sucesso...\nPressione qualquer tecla para continuar...\n");
    while (getchar() != '\n');
    getchar();
    system("clear");
}

///////////////////////////////////////////////////////////////

//////////////////////////    MENUS    ////////////////////////

int menuEdicoes(){
    int op;

    do{
        printf("=----- Edições -----=\n");
        printf("1- Flip Vertical\n");
        printf("2- Flip Horizontal\n");
        printf("3- Rotacionar 90 graus\n");
        printf("4- Transpose\n");
        printf("5- Clahe\n");
        printf("6- Blur\n");
        printf("7- Negativo\n");
        printf("0- Sair\n");
        printf("Escolha: ");
            if(scanf("%d", &op) != 1 || (op < 0 || op > 7)){
                while (getchar() != '\n');
                printf("Entrada inválida. Por favor, digite uma opção válida.\n");
            }
            else
                break;
    } while (1);

    return op;
}

Historico_Gray *edicoes(Historico_Gray *l){
    int op;
    ImageGray *editedImage = NULL;
    char *nome = NULL;
    char *sufixo = NULL;
    Historico_Gray *aux = l;

    do{
        op = menuEdicoes();
        system("clear");

        switch(op){
            case 1:
                editedImage = flip_vertical_gray(aux->img);
                nome = strdup("flipV");
                break;

            case 2:
                editedImage = flip_horizontal_gray(aux->img);
                nome = strdup("flipH");
                break;

            case 3:
                editedImage = rotate_90_gray(aux->img);
                nome = strdup("rot90");
                break;

            case 4: 
                editedImage = transpose_gray(aux->img);
                nome = strdup("transp");
                break;

            case 5:
                editedImage = clahe_gray(aux->img, 64, 128);
                nome = strdup("clahe");
                break;

            case 6: 
                editedImage = median_blur_gray(aux->img, 7);
                nome = strdup("blur");
                break;

            case 7: 
                editedImage = negativo_gray(aux->img);
                nome = strdup("negativ");
                break;

            case 0:
                printf("Saindo das edições...\nPressione qualquer tecla para continuar...\n");
                while (getchar() != '\n');
                getchar();
                system("clear");
                break;

            default:
                printf("Opção inválida!\n");
                continue;
        }
        if(editedImage != NULL){
            sufixo = nome;
            nome = alocarStr(strlen(aux->nome) + strlen(nome) + 2);
            sprintf(nome, "%s_%s", aux->nome, sufixo);

            aux = add_historico(editedImage, l, nome);
            editedImage = NULL;

            sufixo = liberarVetor(sufixo);
            nome = liberarVetor(nome);
            printf("Nova imagem: \"%s\"\n\n", aux->nome);
        }

    } while(op != 0);
    return aux;
}

int menuHist(){
    int op;

    do{
        printf("Historico\n");
        printf("1- Proxima Imagem\n");
        printf("2- Imagem anterior\n");
        printf("0- Sair\n");
        printf("Escolha: ");
            if(scanf("%d", &op) != 1 || (op < 0 || op > 2)){
                while (getchar() != '\n');
                printf("Entrada inválida. Por favor, digite uma opção válida.\n");
            }
            else
                break;
    } while (1);

    return op;
}

Historico_Gray *historico(Historico_Gray *atual){
    int op;

    printf("Imagem atual: \"%s\"\n", atual->nome);

    do{
        op = menuHist();
        system("clear");

        switch (op) {
            case 1:
                atual = next_Image(atual);
                break;

            case 2:
                atual = prev_Image(atual);
                break;

            case 0:
                printf("Saindo do Histórico...\nPressione qualquer tecla para continuar...\n");
                while (getchar() != '\n');
                getchar();
                system("clear");
                break;
        }
    }while(op != 0);

    return atual;
}


int menuGray(){
    int op;

    do{
        printf("=----- MENU -----=\n");
        printf("1- Editar imagem\n");
        printf("2- Ver historico de edições\n");
        printf("3- Salvar imagem\n");
        printf("4- Salvar tudo\n");
        printf("0- Encerrar programa\n");
        printf("Escolha: ");
            if(scanf("%d", &op) != 1 || (op < 0 || op > 4)){
                while (getchar() != '\n');
                printf("Entrada inválida. Por favor, digite uma opção válida.\n");
            }
            else
                break;
    } while (1);

    return op;
}

void Executar_Gray(){
    int op;
    char ch;

    char *caminhoOriginal = "imagens";
    char *txtOriginal = gerarCaminho(caminhoOriginal, "/", "lena.txt");
    char *imagemOriginal = "utils/lena.png";
    char *pasta = pastaPrincipal(caminhoOriginal);
 
    ImageGray *image = lerImagemGray(imagemOriginal, txtOriginal);

    char *nome = strdup("lena");

    Historico_Gray *history = criar_lista();
    history = add_historico(image, history, nome); //adiciona a imagem original na cabeça da lista

    printf("Imagem: \"%s\" foi adicionada\n\n", history->nome);

    nome = liberarVetor(nome);

    do{
        printf("Imagem atual: %s\n\n", history->nome);
        op = menuGray();
        system("clear");

        switch (op){
            case 1:
                if(history->prox != NULL){
                    printf("ATENÇÃO!\nA imagem atual não é a ultima imagem editada.\nSe você decidir editar a imagem atual, todas as edições posteriores realizadas anteriormente serão perdidas.\nAvançar? (S/N):");
                    scanf(" %c", &ch);
                    if(ch == 'S' || ch == 's'){
                        system("clear");
                        free_Historico(history->prox);
                        history->prox = NULL;

                        history = edicoes(history);
                        break;
                    }
                    else
                        break;
                }
                else{
                    history = edicoes(history);
                    break;
                }

            case 2:
                history = historico(history);
                break;

            case 3: 
                printf("Salvando imagem \"%s\"...\n\n", history->nome);
                salvar(history->img, pasta, history->nome);
                break;

            case 4:
                salvarTudo(history, pasta);
                break;

            case 0:
                printf("Encerrando programa...\n");
                txtOriginal = liberarVetor(txtOriginal);
                pasta = liberarVetor(pasta);
                break;
        }
    }while(op != 0);
}
