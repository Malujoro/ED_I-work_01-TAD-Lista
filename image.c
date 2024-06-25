#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> // Biblioteca para verificar pastas
#include <sys/stat.h> // Biblioteca para criar pastas
#include <sys/types.h> // Biblioteca para especificar os bits de permissão da pasta criada
#include <Python.h> // API para utilizar o python em C
#include <locale.h> //Biblioteca para adicionar os emoji (usados nos botões de rotate e transpose)
#include <string.h> //Biblioteca para usar strdup //possívelmente temporária
#include <gtk-4.0/gtk/gtk.h> // Biblioteca para interface gráfica
#include <ctype.h> // Biblioteca para isdigit()
#include <time.h> // Biblioteca para números aleatórios

#define SCRIPT 0
#define FUNCAO 1
#define ARGUMENTOS 2

#define RGB 0
#define GRAY 1
#define TRANSPOSE1 2
#define TRANSPOSE2 3
#define ROTATE_90 4
#define ROTATE_90_ANTI 5
#define VERTICAL 6
#define HORIZONTAL 7
#define BLUR 8
#define CLAHE 9
#define NEGATIVO 10
#define ALEATORIO 11
#define ANTERIOR 12
#define PROXIMO 13
#define SALVAR 14
#define SAIR 15

int funcoes[] = {RGB, GRAY, TRANSPOSE1, TRANSPOSE2, ROTATE_90, ROTATE_90_ANTI, VERTICAL, HORIZONTAL, BLUR, CLAHE, NEGATIVO, ALEATORIO, ANTERIOR, PROXIMO, SALVAR, SAIR};

int tileBlurGlobal = 9;
int alturaClaheGlobal = 64;
int larguraClaheGlobal = 64;
int clipLimitGlobal = 40;
int randomGlobal = 3;

char *pastaOriginal;
char *txtOriginal;

GtkWidget *grid;

//struct dos nós da lista duplamente encadeada, usada no histórico
typedef struct historico_gray
{
    ImageGray *img;
    char *nome;
    char *png;
    struct historico_gray *prox;
    struct historico_gray *ant;
} Historico_Gray;

typedef struct historico_rgb
{
    ImageRGB *img;
    char *nome;
    char *png;
    struct historico_rgb *prox;
    struct historico_rgb *ant;
} Historico_RGB;

Historico_Gray *historyGray;
Historico_RGB *historyRGB;

//////////////////// Funções auxiliares ////////////////////

// Função para converter uma posição de matriz em posição de vetor
int posicaoVetor(int largura, int i, int j)
{
    return largura * i + j;
}

void limparVet(int *vetor, int tam)
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
    result[quant] = '\0';
    return result;
}

int validarNumero(const gchar *str)
{
    for(int i = 0; str[i] != '\0'; i++)
    {
        if (!isdigit(str[i]))
            return 0;
    }
    return 1;
}

char *nomeCaminho(char *imagem)
{
    int posPonto = strlen(imagem), posBarra = -1;

    for(int i = 0; imagem[i] != '\0'; i++)
    {
        if(imagem[i] == '.')
            posPonto = i;
        else if(imagem[i] == '/')
            posBarra = i;
    }

    int tam = posPonto - posBarra;
    char *nome = alocarStr(tam);

    for(int i = posBarra+1, j = 0; i < posPonto && j < tam; i++, j++)
        nome[j] = imagem[i];
    nome[tam-1] = '\0';

    return nome;
}
/////////////// Alocação ///////////////

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

int ***alocarMatrizInt3(int lin, int col, int prof)
{
    int ***matriz = (int ***) malloc(lin * sizeof(int **));

    if(!matriz)
    {
        printf("Erro ao alocar matriz");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < lin; i++)
    {
        matriz[i] = (int **) malloc(col * sizeof(int *));

        if(!matriz[i])
        {
            printf("Erro ao alocar matriz");
            exit(EXIT_FAILURE);
        }

        for(int j = 0; j < col; j++)
            matriz[i][j] = alocarInt(prof);

    }

    return matriz;
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
        return matriz;

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

void *liberarMatrizInt3(int ***matriz, int lin, int col)
{
    for(int i = 0; i < lin; i++)
    {
        for(int j = 0; j < col; j++)
        {
            free(matriz[i][j]);
            matriz[i][j] = NULL;
        }
        free(matriz[i]);
        matriz[i] = NULL;
    }
    free(matriz);
    return NULL;
}

//////////////// Pastas ////////////////

char *gerarCaminho(char *pasta, char *simbolo, char *nome)
{
    int tamanho = 256;
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
    if(!pastaExiste(caminho))
        criarPasta(caminho);

    char *num = intParaStr(contarPastas(caminho));
    char *caminho2 = gerarCaminho(caminho, "/", num);

    if(!pastaExiste(caminho2))
        criarPasta(caminho2);

    num = liberarVetor(num);
    return caminho2;
}

//////////////// Python ////////////////

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

int calculaCaixas(int eixo, int tile_eixo)
{
    int caixa = eixo / tile_eixo;

    if(eixo % tile_eixo != 0)
        caixa++;

    return caixa;
}

int cdf(int *vetor, int pos)
{
    int soma = 0;
    for(int i = 0; i <= pos; i++)
        soma += vetor[i];
    return soma;
}

int cdf_normalizado(int cdf_i, int cdf_min, int cdf_max)
{
    return ((float) (cdf_i - cdf_min)) / (cdf_max - cdf_min) * 255;
}

int normaliza_histograma(int *histograma, int *resultado)
{
    int minimo = cdf(histograma, posMinimo(histograma));
    int maximo = cdf(histograma, 255);

    if(maximo != minimo)
    {
        for(int i = 0, ac = 0; i < 256; i++)
        {
            ac += histograma[i];
            resultado[i] = cdf_normalizado(ac, minimo, maximo);
        }
        return 1;
    }
    return 0;
}

void redistribuirHistograma(int *histograma, int clip_limit)
{
    int *aux = alocarInt(256);
    int limite, quant = 256;
    float soma = 0;
    do
    {
        limite = 0;        

        // Redistribuir os valores enquanto algum passar do limite
        // Somar os passados
        for(int i = 0; i < 256; i++)
        {
            if((!aux[i]) && (histograma[i] > clip_limit))
            {
                aux[i]++;
                quant--;
                limite = 1;
                soma += histograma[i] - clip_limit;
                histograma[i] = clip_limit;
            }
        }

        // Blindagem contra looping infinito (impossível de redistribuir)
        if(quant == 0)
            break;

        // Distribuir os valores igualmente
        // Efetua a redistribuição se for possível adicionar no mínimo 1 pixel em cada coluna
        if(soma >= 256)
        {
            for(int i = 0; i < 256; i++)
            {
                if(!aux[i])
                    histograma[i] += soma / quant;
            }
            // Vai pegar "todos os restos" decimais que não entrarão no histograma
            // (divisão decimal - divisão inteira) * quantidade de colunas "utilizadas"
            soma = ((soma / quant) - ((int) soma / quant)) * quant;
        }
        // Caso possua menos que 256 pixels, eles serão adicionados na menor coluna
        else
        {
            histograma[posMenor(histograma)] += soma;
            soma = 0;
        }
    }while(limite);
    aux = liberarVetor(aux);
}

int posMinimo(int *histograma)
{
    for(int i = 0; i < 256; i++)
    {
        if(histograma[i] != 0)
            return i;
    }
    return 0;
}

int posMenor(int *histograma)
{
    int posicao = 0;
    for(int i = 1; i < 256; i++)
    {
        if(histograma[i] < histograma[posicao])
            posicao = i;
    }
    return posicao;
}

float interpolacaoBilinear(float x, float y, int ponto[][2])
{
    return (1 - y) * (1 - x) * ponto[0][0]
    + (1 - y) * x * ponto[0][1]
    + y * (1 - x) * ponto[1][0]
    + y * x * ponto[1][1];
}

void suavizaGray(ImageGray *image, int ***histogramas, int width, int height)
{
    int ponto[2][2];
    float x, y;
    int caixaI, caixaJ, caixaI2, caixaJ2, valor;

    for(int i = 0, i2 = height; i < image->dim.altura; i++, i2++)
    {
        if(i2 >= image->dim.altura)
            i2 = i;
            
        caixaI = i / height;
        caixaI2 = i2 / height;

        for(int j = 0, j2 = width; j < image->dim.largura; j++, j2++)
        {
            if(j2 >= image->dim.largura)
                j2 = j;

            caixaJ = j / width;
            caixaJ2 = j2 / width;

            valor = image->pixels[posicaoVetor(image->dim.largura, i, j)].value;

            ponto[0][0] = histogramas[caixaI][caixaJ][valor];
            ponto[0][1] = histogramas[caixaI][caixaJ2][valor];
            ponto[1][0] = histogramas[caixaI2][caixaJ][valor];
            ponto[1][1] = histogramas[caixaI2][caixaJ2][valor];
            
            y = ((float) (i % height) / height);
            x = ((float) (j % width) / width);

            image->pixels[posicaoVetor(image->dim.largura, i, j)].value = interpolacaoBilinear(x, y, ponto);
        }
    }
}

void suavizaRGB(ImageRGB *image, int ***histogramasRed, int ***histogramasGreen, int ***histogramasBlue, int width, int height)
{
    int ponto[2][2];
    float x, y;
    int caixaI, caixaJ, caixaI2, caixaJ2, valor;

    for(int i = 0, i2 = height; i < image->dim.altura; i++, i2++)
    {
        if(i2 >= image->dim.altura)
            i2 = i;
            
        caixaI = i / height;
        caixaI2 = i2 / height;

        for(int j = 0, j2 = width; j < image->dim.largura; j++, j2++)
        {
            if(j2 >= image->dim.largura)
                j2 = j;

            caixaJ = j / width;
            caixaJ2 = j2 / width;

            y = ((float) (i % height) / height);
            x = ((float) (j % width) / width);

            valor = image->pixels[posicaoVetor(image->dim.largura, i, j)].red;
            ponto[0][0] = histogramasRed[caixaI][caixaJ][valor];
            ponto[0][1] = histogramasRed[caixaI][caixaJ2][valor];
            ponto[1][0] = histogramasRed[caixaI2][caixaJ][valor];
            ponto[1][1] = histogramasRed[caixaI2][caixaJ2][valor];
            image->pixels[posicaoVetor(image->dim.largura, i, j)].red = interpolacaoBilinear(x, y, ponto);

            valor = image->pixels[posicaoVetor(image->dim.largura, i, j)].green;
            ponto[0][0] = histogramasGreen[caixaI][caixaJ][valor];
            ponto[0][1] = histogramasGreen[caixaI][caixaJ2][valor];
            ponto[1][0] = histogramasGreen[caixaI2][caixaJ][valor];
            ponto[1][1] = histogramasGreen[caixaI2][caixaJ2][valor];
            image->pixels[posicaoVetor(image->dim.largura, i, j)].green = interpolacaoBilinear(x, y, ponto);

            valor = image->pixels[posicaoVetor(image->dim.largura, i, j)].blue;
            ponto[0][0] = histogramasBlue[caixaI][caixaJ][valor];
            ponto[0][1] = histogramasBlue[caixaI][caixaJ2][valor];
            ponto[1][0] = histogramasBlue[caixaI2][caixaJ][valor];
            ponto[1][1] = histogramasBlue[caixaI2][caixaJ2][valor];
            image->pixels[posicaoVetor(image->dim.largura, i, j)].blue = interpolacaoBilinear(x, y, ponto);
        }
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

ImageRGB *lerTxtRGB(char *caminho)
{
    FILE *arquivo = lerArquivo(caminho, "r");
    
    int altura, largura;
    fscanf(arquivo, "%d", &altura);
    fgetc(arquivo);
    fscanf(arquivo, "%d", &largura);
    fgetc(arquivo);

    ImageRGB *imagem = create_image_rgb(largura, altura);

    for(int i = 0; i < altura; i++)
    {
        for(int j = 0; j < largura; j++)
        {
            fscanf(arquivo, "%d %d %d", &imagem->pixels[posicaoVetor(largura, i, j)].red, &imagem->pixels[posicaoVetor(largura, i, j)].green, &imagem->pixels[posicaoVetor(largura, i, j)].blue);
            fgetc(arquivo);
        }
        fgetc(arquivo);
    }
    fclose(arquivo);
    return imagem;
}


ImageGray *lerImagemGray(char *png, char *txt)
{
    txt_from_image(png, txt, 1);
    return lerTxtGray(txt);
}

ImageRGB *lerImagemRGB(char *png, char *txt)
{
    txt_from_image(png, txt, 0);
    return lerTxtRGB(txt);
}


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

void salvarTxtRGB(ImageRGB *imagem, char *caminho, char *txt)
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
            fprintf(arquivo, "%d %d %d,", imagem->pixels[posicaoVetor(imagem->dim.largura, i, j)].red, imagem->pixels[posicaoVetor(imagem->dim.largura, i, j)].green, imagem->pixels[posicaoVetor(imagem->dim.largura, i, j)].blue);
        }
        fputc('\n', arquivo);
    }

    fclose(arquivo);
}


void salvarImagemGray(ImageGray *imagem, char *caminho, char *txt, char *png)
{
    if(!pastaExiste(caminho))
        criarPasta(caminho);

    salvarTxtGray(imagem, caminho, txt);

    image_from_txt(txt, png, 1);
}

void salvarImagemRGB(ImageRGB *imagem, char *caminho, char *txt, char *png)
{
    if(!pastaExiste(caminho))
        criarPasta(caminho);

    salvarTxtRGB(imagem, caminho, txt);

    image_from_txt(txt, png, 0);
}

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


ImageGray *rotate_90_gray(const ImageGray *image)
{
    ImageGray *imageRotate = create_image_gray(image->dim.altura, image->dim.largura);

    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++)
            imageRotate->pixels[posicaoVetor(imageRotate->dim.largura, j, (imageRotate->dim.largura - 1 - i))] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
    }

    return imageRotate;
}

ImageGray *rotate_90_anti_gray(const ImageGray *image)
{
    ImageGray *imageRotate = create_image_gray(image->dim.altura, image->dim.largura);

    //Rotacionar no sentido anti horário
    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++)
            imageRotate->pixels[posicaoVetor(imageRotate->dim.largura, imageRotate->dim.altura, i) - imageRotate->dim.largura * (j + 1)] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
    }

    return imageRotate;
}


ImageGray *transpose_gray(const ImageGray *image)
{
    ImageGray *imageTranspose = create_image_gray(image->dim.altura, image->dim.largura);

    //Transpose - inverte diagonais direita superior e esqueda inferior
    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++)
            imageTranspose->pixels[posicaoVetor(image->dim.altura, j, i)] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
    }
    
    return imageTranspose;
}

ImageGray *transpose2_gray(const ImageGray *image)
{
    ImageGray *imageTranspose = create_image_gray(image->dim.altura, image->dim.largura);

    //Transpose - inverte diagonais esquerda superior e direita inferior
    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++)
            imageTranspose->pixels[posicaoVetor(image->dim.altura, (image->dim.largura - j - 1), (image->dim.altura - i - 1))] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
    }
    
    return imageTranspose;
}



ImageRGB *flip_vertical_rgb(const ImageRGB *image)
{
    ImageRGB *imageFlipV = create_image_rgb(image->dim.largura, image->dim.altura);

    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++){
            imageFlipV->pixels[posicaoVetor(image->dim.largura, i, j)] = image->pixels[posicaoVetor(image->dim.largura, (image->dim.altura - i - 1), j)];
        }
    }

    return imageFlipV;
}

ImageRGB *flip_horizontal_rgb(const ImageRGB *image)
{
    ImageRGB *imageFlipH = create_image_rgb(image->dim.largura, image->dim.altura);

    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++){
            imageFlipH->pixels[posicaoVetor(image->dim.largura, i, j)] = image->pixels[(image->dim.largura * i) + (image->dim.largura - j - 1)];
        }
    }

    return imageFlipH;
}


ImageRGB *rotate_90_rgb(const ImageRGB *image)
{
    ImageRGB *imageRotate = create_image_rgb(image->dim.altura, image->dim.largura);

    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++)
            imageRotate->pixels[posicaoVetor(imageRotate->dim.largura, j, (imageRotate->dim.largura - 1 - i))] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
    }

    return imageRotate;
}

ImageRGB *rotate_90_anti_rgb(const ImageRGB *image)
{
    ImageRGB *imageRotate = create_image_rgb(image->dim.altura, image->dim.largura);

    //Rotacionar no sentido anti horário
    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++)
            imageRotate->pixels[posicaoVetor(imageRotate->dim.largura, imageRotate->dim.altura, i) - imageRotate->dim.largura * (j + 1)] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
    }

    return imageRotate;
}


ImageRGB *transpose_rgb(const ImageRGB *image)
{
    ImageRGB *imageTranspose = create_image_rgb(image->dim.altura, image->dim.largura);

    //Transpose - inverte diagonais direita superior e esqueda inferior
    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++)
            imageTranspose->pixels[posicaoVetor(image->dim.altura, j, i)] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
    }
    
    return imageTranspose;
}

ImageRGB *transpose2_rgb(const ImageRGB *image)
{
    ImageRGB *imageTranspose = create_image_rgb(image->dim.altura, image->dim.largura);

    //Transpose - inverte diagonais esquerda superior e direita inferior
    for(int i = 0; i < image->dim.altura; i++){
        for(int j = 0; j < image->dim.largura; j++)
            imageTranspose->pixels[posicaoVetor(image->dim.altura, (image->dim.largura - j - 1), (image->dim.altura - i - 1))] = image->pixels[posicaoVetor(image->dim.largura, i, j)];
    }
    
    return imageTranspose;
}

///////////// Funções de Manipulação por Pixel /////////////

// Manipulação por pixel para ImageGray
ImageGray *clahe_gray(const ImageGray *image, int tile_width, int tile_height)
{
    int caixaX, caixaY;

    caixaX = calculaCaixas(image->dim.largura, tile_width);
    caixaY = calculaCaixas(image->dim.altura, tile_height);

    // ImageGray *resultado = create_image_gray(image->dim.largura, image->dim.altura);
    ImageGray *resultado = copiarImagemGray(image);
    int tamVet = tile_height * tile_width;
    int *vetor = alocarInt(tamVet);
    int ***histogramas = alocarMatrizInt3(caixaY, caixaX, 256);
    int *histograma = alocarInt(256);
    float clip_limit = clipLimitGlobal;
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
                histograma[vetor[i]]++;

            redistribuirHistograma(histograma, clip_limit);
            normaliza_histograma(histograma, histogramas[a][b]);

            limparVet(vetor, tam);
            limparVet(histograma, 256);
        }
    }
    histograma = liberarVetor(histograma);
    vetor = liberarVetor(vetor);

    suavizaGray(resultado, histogramas, tile_width, tile_height);
    histogramas = liberarMatrizInt3(histogramas, caixaY, caixaX);

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
    vetor = liberarVetor(vetor);
    return blur;
}

ImageGray *negativo_gray(const ImageGray *image)
{
    ImageGray *result = create_image_gray(image->dim.largura, image->dim.altura);

    for(int i = 0; i < result->dim.altura * result->dim.largura; i++)
        result->pixels[i].value = 255 - image->pixels[i].value;
    
    return result;
}


ImageRGB *clahe_rgb(const ImageRGB *image, int tile_width, int tile_height)
{
    int caixaX, caixaY;

    caixaX = calculaCaixas(image->dim.largura, tile_width);
    caixaY = calculaCaixas(image->dim.altura, tile_height);

    // ImageGray *resultado = create_image_gray(image->dim.largura, image->dim.altura);
    ImageRGB *resultado = copiarImagemRGB(image);
    int tamVet = tile_height * tile_width;

    int *vetorRed = alocarInt(tamVet);
    int *vetorGreen = alocarInt(tamVet);
    int *vetorBlue = alocarInt(tamVet);

    int ***histogramasRed = alocarMatrizInt3(caixaY, caixaX, 256);
    int ***histogramasGreen = alocarMatrizInt3(caixaY, caixaX, 256);
    int ***histogramasBlue = alocarMatrizInt3(caixaY, caixaX, 256);

    int *histogramaRed = alocarInt(256);
    int *histogramaGreen = alocarInt(256);
    int *histogramaBlue = alocarInt(256);

    float clip_limit = clipLimitGlobal;
    for(int a = 0; a < caixaY; a++)
    {
        for(int b = 0, tam = 0; b < caixaX; b++, tam = 0)
        {
            // Monta o Tile
            for(int i = 0, posI = a * tile_height; i < tile_height && posI < image->dim.altura; i++, posI++)
            {
                for(int j = 0, posJ = b * tile_width; j < tile_width && posJ < image->dim.largura; j++, posJ++, tam++)
                {
                    vetorRed[tam] = image->pixels[posicaoVetor(image->dim.largura, posI, posJ)].red;
                    vetorGreen[tam] = image->pixels[posicaoVetor(image->dim.largura, posI, posJ)].green;
                    vetorBlue[tam] = image->pixels[posicaoVetor(image->dim.largura, posI, posJ)].blue;
                }
            }
            
            // Monta o Histograma
            for(int i = 0; i < tam; i++)
            {
                histogramaRed[vetorRed[i]]++;
                histogramaGreen[vetorGreen[i]]++;
                histogramaBlue[vetorBlue[i]]++;
            }

            redistribuirHistograma(histogramaRed, clip_limit);
            redistribuirHistograma(histogramaGreen, clip_limit);
            redistribuirHistograma(histogramaBlue, clip_limit);

            normaliza_histograma(histogramaRed, histogramasRed[a][b]);
            normaliza_histograma(histogramaGreen, histogramasGreen[a][b]);
            normaliza_histograma(histogramaBlue, histogramasBlue[a][b]);

            limparVet(vetorRed, tam);
            limparVet(vetorGreen, tam);
            limparVet(vetorBlue, tam);

            limparVet(histogramaRed, 256);
            limparVet(histogramaGreen, 256);
            limparVet(histogramaBlue, 256);
        }
    }
    histogramaRed = liberarVetor(histogramaRed);
    histogramaGreen = liberarVetor(histogramaGreen);
    histogramaBlue = liberarVetor(histogramaBlue);

    vetorRed = liberarVetor(vetorRed);
    vetorGreen = liberarVetor(vetorGreen);
    vetorBlue = liberarVetor(vetorBlue);

    suavizaRGB(resultado, histogramasRed, histogramasGreen, histogramasBlue, tile_width, tile_height);
    
    histogramasRed = liberarMatrizInt3(histogramasRed, caixaY, caixaX);
    histogramasGreen = liberarMatrizInt3(histogramasGreen, caixaY, caixaX);
    histogramasBlue = liberarMatrizInt3(histogramasBlue, caixaY, caixaX);

    return resultado;
}

ImageRGB *median_blur_rgb(const ImageRGB *image, int kernel_size)
{
    if(kernel_size % 2 == 0)
    {
        printf("Kernel size deve ser ímpar");
        return NULL;
    }

    ImageRGB *blur = copiarImagemRGB(image);

    int tam = kernel_size / 2, meio, quant;
    int *vetorRed = alocarInt(kernel_size * kernel_size);
    int *vetorGreen = alocarInt(kernel_size * kernel_size);
    int *vetorBlue = alocarInt(kernel_size * kernel_size);
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
                        
                    vetorRed[quant] = image->pixels[posicaoVetor(image->dim.largura, posY, posX)].red;
                    vetorGreen[quant] = image->pixels[posicaoVetor(image->dim.largura, posY, posX)].green;
                    vetorBlue[quant] = image->pixels[posicaoVetor(image->dim.largura, posY, posX)].blue;
                }
            }
            meio = posicaoVetor(image->dim.largura, i, j);
            blur->pixels[meio].red = mediana(vetorRed, quant);
            blur->pixels[meio].green = mediana(vetorGreen, quant);
            blur->pixels[meio].blue = mediana(vetorBlue, quant);
        }
    }
    vetorRed = liberarVetor(vetorRed);
    vetorGreen = liberarVetor(vetorGreen);
    vetorBlue = liberarVetor(vetorBlue);
    return blur;
}

ImageRGB *negativo_rgb(const ImageRGB *image)
{
    ImageRGB *result = create_image_rgb(image->dim.largura, image->dim.altura);

    for(int i = 0; i < result->dim.altura * result->dim.largura; i++)
    {
        result->pixels[i].red = 255 - image->pixels[i].red;
        result->pixels[i].green = 255 - image->pixels[i].green;
        result->pixels[i].blue = 255 - image->pixels[i].blue;
    }
    
    return result;
}

////////////////////////////////////////////////////////////

void exibirImagemGray(Historico_Gray *atual)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(atual->png, NULL);
    
    if (!pixbuf)
    {
        g_print("Erro ao carregar a imagem\n");
        exit(EXIT_FAILURE);
    }

    GtkWidget *resultado = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);

    gtk_widget_set_size_request(resultado, atual->img->dim.largura, atual->img->dim.altura);
    gtk_grid_attach(GTK_GRID(grid), resultado, 16, 0, 16, 16);
}

void exibirImagemRGB(Historico_RGB *atual)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(atual->png, NULL);
    
    if (!pixbuf)
    {
        g_print("Erro ao carregar a imagem\n");
        exit(EXIT_FAILURE);
    }

    GtkWidget *resultado = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);

    gtk_widget_set_size_request(resultado, atual->img->dim.largura, atual->img->dim.altura);
    gtk_grid_attach(GTK_GRID(grid), resultado, 16, 0, 16, 16);
}

//////////////////   Funções para as operações do Histórico   //////////////////// 

Historico_Gray *criar_lista_gray(){
    return NULL;
}

//cria um novo elemento para o historico
Historico_Gray *criar_No_gray(){
    Historico_Gray *no = (Historico_Gray *) malloc(sizeof(Historico_Gray));
    if(!no){
        printf("Erro ao alocar o novo nó.");
        exit(EXIT_FAILURE);
    }

    no->png = NULL;
    no->prox = NULL;
    no->ant = NULL;

    return no;
}

//adiciona a ultima edição ao final do historico
Historico_Gray *add_historico_gray(ImageGray *image, Historico_Gray *l, const char *nome){
    Historico_Gray *novo = criar_No_gray();
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
Historico_Gray *next_Image_gray(Historico_Gray *atual){
    if(atual != NULL)
    {
        if(atual->prox != NULL){
            atual = atual->prox;
            printf("Imagem atual:\n");
            printf("\"%s\"\n\n", atual->nome);
        }
        else{
            printf("Você chegou a ultima imagem.\n");
            printf("\"%s\"\n", atual->nome);
        }
        exibirImagemGray(atual);
    }
    return atual;
}

//percorre para a edição anterior
Historico_Gray *prev_Image_gray(Historico_Gray *atual){
    if(atual != NULL)
    {
        if(atual->ant != NULL){
            atual = atual->ant;
            printf("Imagem atual:\n");
            printf("\"%s\"\n\n", atual->nome);
        }
        else{
            printf("Você chegou a primeira imagem.\n");
            printf("\"%s\"\n", atual->nome);
        }
        exibirImagemGray(atual);
    }
    return atual;
}

//libera a memória do historico de imagens
void free_Historico_gray(Historico_Gray *l){
    Historico_Gray *aux;

    while(l != NULL){
        aux = l;
        l = l->prox;
        free_image_gray(aux->img);
        aux->nome = liberarVetor(aux->nome);
        aux->png = liberarVetor(aux->png);
        free(aux);
    }
}


Historico_RGB *criar_lista_rgb(){
    return NULL;
}

//cria um novo elemento para o historico
Historico_RGB *criar_No_rgb(){
    Historico_RGB *no = (Historico_RGB *) malloc(sizeof(Historico_RGB));
    if(!no){
        printf("Erro ao alocar o novo nó.");
        exit(EXIT_FAILURE);
    }

    no->png = NULL;
    no->prox = NULL;
    no->ant = NULL;

    return no;
}

//adiciona a ultima edição ao final do historico
Historico_RGB *add_historico_rgb(ImageRGB *image, Historico_RGB *l, const char *nome){
    Historico_RGB *novo = criar_No_rgb();
    novo->img = image;
    novo->nome = strdup(nome);  //cria uma cópia de nome

    if(l == NULL)
        return novo;
    
    Historico_RGB *aux = l;
    while(aux->prox != NULL)
        aux = aux->prox;

    aux->prox = novo;
    novo->ant = aux;

    return novo;
}

//percorre para a proxima edição
Historico_RGB *next_Image_rgb(Historico_RGB *atual){
    if(atual != NULL)
    {
        if(atual->prox != NULL){
            atual = atual->prox;
            printf("Imagem atual:\n");
            printf("\"%s\"\n\n", atual->nome);
        }
        else{
            printf("Você chegou a ultima imagem.\n");
            printf("\"%s\"\n", atual->nome);
        }
        exibirImagemRGB(atual);
    }
    return atual;
}

//percorre para a edição anterior
Historico_RGB *prev_Image_rgb(Historico_RGB *atual){
    if(atual != NULL)
    {
        if(atual->ant != NULL){
            atual = atual->ant;
            printf("Imagem atual:\n");
            printf("\"%s\"\n\n", atual->nome);
        }
        else{
            printf("Você chegou a primeira imagem.\n");
            printf("\"%s\"\n", atual->nome);
        }
        exibirImagemRGB(atual);
    }
    return atual;
}

//libera a memória do historico de imagens
void free_Historico_rgb(Historico_RGB *l){
    Historico_RGB *aux;

    while(l != NULL){
        aux = l;
        l = l->prox;
        free_image_rgb(aux->img);
        aux->nome = liberarVetor(aux->nome);
        aux->png = liberarVetor(aux->png);
        free(aux);
    }
}

///////////////////////////////////////////////////////////////

////////////////////// SALVAR IMAGEM //////////////////////

void salvar_gray(ImageGray *image, char *pasta, char *nome){
    char *caminho = gerarCaminho(pasta, "/", nome);
    char *txt = gerarCaminho(caminho, ".", "txt");
    char *png = gerarCaminho(caminho, ".", "png");

    salvarImagemGray(image, pasta, txt, png);
    historyGray->png = png;
    exibirImagemGray(historyGray);
    
    g_print("Imagem salva com sucesso...\n");

    caminho = liberarVetor(caminho);
    txt = liberarVetor(txt);
}

void salvarTudo_gray(Historico_Gray *l, char *pasta){
    Historico_Gray *aux = l;

    while(aux->ant->ant != NULL)
        aux = aux->ant;

    while(aux != NULL){
        g_print("Salvando imagem \"%s\"...\n\n", aux->nome);
        salvar_gray(aux->img, pasta, aux->nome);
        aux = aux->prox;
    }

    g_print("Todas as imagens foram salvas com sucesso...\n");
}


void salvar_rgb(ImageRGB *image, char *pasta, char *nome){
    char *caminho = gerarCaminho(pasta, "/", nome);
    char *txt = gerarCaminho(caminho, ".", "txt");
    char *png = gerarCaminho(caminho, ".", "png");

    salvarImagemRGB(image, pasta, txt, png);
    historyRGB->png = png;
    exibirImagemRGB(historyRGB);
    
    g_print("Imagem salva com sucesso...\n");

    caminho = liberarVetor(caminho);
    txt = liberarVetor(txt);
}

void salvarTudo_rgb(Historico_RGB *l, char *pasta){
    Historico_RGB *aux = l;

    while(aux->ant->ant != NULL)
        aux = aux->ant;

    while(aux != NULL){
        g_print("Salvando imagem \"%s\"...\n\n", aux->nome);
        salvar_rgb(aux->img, pasta, aux->nome);
        aux = aux->prox;
    }

    g_print("Todas as imagens foram salvas com sucesso...\n");
}

///////////////////////////////////////////////////////////////

//////////////////////////    MENUS    ////////////////////////

Historico_Gray *edicoesGray(Historico_Gray *l, int op){
    ImageGray *editedImage = NULL;
    char *nome = NULL;
    char *sufixo = NULL;
    Historico_Gray *aux = l;

    switch(op){
        case VERTICAL:
            editedImage = flip_vertical_gray(aux->img);
            nome = strdup("flipV");
            break;

        case HORIZONTAL:
            editedImage = flip_horizontal_gray(aux->img);
            nome = strdup("flipH");
            break;

        case ROTATE_90:
            editedImage = rotate_90_gray(aux->img);
            nome = strdup("rot90");
            break;
        
        case ROTATE_90_ANTI:
            editedImage = rotate_90_anti_gray(aux->img);
            nome = strdup("rot90ant");
            break;

        case TRANSPOSE1:
            editedImage = transpose_gray(aux->img);
            nome = strdup("transp1");
            break;

        case TRANSPOSE2:
            editedImage = transpose2_gray(aux->img);
            nome = strdup("transp2");
            break;

        case CLAHE:
            editedImage = clahe_gray(aux->img, larguraClaheGlobal, alturaClaheGlobal);
            nome = strdup("clahe");
            break;

        case BLUR:
            editedImage = median_blur_gray(aux->img, tileBlurGlobal);
            nome = strdup("blur");
            break;

        case NEGATIVO:
            editedImage = negativo_gray(aux->img);
            nome = strdup("negativ");
            break;
    }

    if(editedImage != NULL){
        sufixo = nome;
        nome = alocarStr(strlen(aux->nome) + strlen(nome) + 2);
        sprintf(nome, "%s_%s", aux->nome, sufixo);

        aux = add_historico_gray(editedImage, l, nome);
        editedImage = NULL;

        sufixo = liberarVetor(sufixo);
        nome = liberarVetor(nome);
        g_print("\nNova imagem: \"%s\"\n", aux->nome);
    }

    return aux;
}

Historico_RGB *edicoesRGB(Historico_RGB *l, int op){
    ImageRGB *editedImage = NULL;
    char *nome = NULL;
    char *sufixo = NULL;
    Historico_RGB *aux = l;

    switch(op){
        case VERTICAL:
            editedImage = flip_vertical_rgb(aux->img);
            nome = strdup("flipV");
            break;

        case HORIZONTAL:
            editedImage = flip_horizontal_rgb(aux->img);
            nome = strdup("flipH");
            break;

        case ROTATE_90:
            editedImage = rotate_90_rgb(aux->img);
            nome = strdup("rot90");
            break;
        
        case ROTATE_90_ANTI:
            editedImage = rotate_90_anti_rgb(aux->img);
            nome = strdup("rot90ant");
            break;

        case TRANSPOSE1:
            editedImage = transpose_rgb(aux->img);
            nome = strdup("transp1");
            break;

        case TRANSPOSE2:
            editedImage = transpose2_rgb(aux->img);
            nome = strdup("transp2");
            break;

        case CLAHE:
            editedImage = clahe_rgb(aux->img, larguraClaheGlobal, alturaClaheGlobal);
            nome = strdup("clahe");
            break;

        case BLUR:
            editedImage = median_blur_rgb(aux->img, tileBlurGlobal);
            nome = strdup("blur");
            break;

        case NEGATIVO:
            editedImage = negativo_rgb(aux->img);
            nome = strdup("negativ");
            break;
    }

    if(editedImage != NULL){
        sufixo = nome;
        nome = alocarStr(strlen(aux->nome) + strlen(nome) + 2);
        sprintf(nome, "%s_%s", aux->nome, sufixo);

        aux = add_historico_rgb(editedImage, l, nome);
        editedImage = NULL;

        sufixo = liberarVetor(sufixo);
        nome = liberarVetor(nome);
        g_print("\nNova imagem: \"%s\"\n", aux->nome);
    }

    return aux;
}


void Executar_Gray(GtkWidget *widget, gpointer data)
{
    int op = *((int *) data);

    if(historyGray)
    {
        g_print("Imagem atual: %s\n\n", historyGray->nome);

        if(op >= TRANSPOSE1 && op <= ALEATORIO)
        {
            int aux = 1, aux2 = op;
            if(op == ALEATORIO)
                aux = randomGlobal;

            for(int i = 0; i < aux; i++)
            {
                if(op == ALEATORIO)
                    aux2 = rand() % 9 + 2;

                if(historyGray->prox != NULL)
                {
                    free_Historico_gray(historyGray->prox);
                    historyGray->prox = NULL;
                }

                historyGray = edicoesGray(historyGray, aux2);
                if(historyGray != NULL)
                    salvar_gray(historyGray->img, pastaOriginal, historyGray->nome);
            }
        }

        else
        {
            switch(op)
            {
                case ANTERIOR:
                    historyGray = prev_Image_gray(historyGray);
                    break;

                case PROXIMO:
                    historyGray = next_Image_gray(historyGray);
                    break;

                case SALVAR:
                    g_print("Salvando imagem \"%s\"...\n\n", historyGray->nome);
                    salvar_gray(historyGray->img, pastaOriginal, historyGray->nome);
                    break;

                case SAIR:
                    g_print("Encerrando programa...\n");
                    txtOriginal = liberarVetor(txtOriginal);
                    pastaOriginal = liberarVetor(pastaOriginal);
                    break;
            }
        }
    }
}

void Executar_RGB(GtkWidget *widget, gpointer data)
{
    int op = *((int *) data);

    if(historyRGB)
    {
        g_print("Imagem atual: %s\n\n", historyRGB->nome);

        if(op >= TRANSPOSE1 && op <= ALEATORIO)
        {
            int aux = 1, aux2 = op;
            if(op == ALEATORIO)
                aux = randomGlobal;

            for(int i = 0; i < aux; i++)
            {
                if(op == ALEATORIO)
                    aux2 = rand() % 9 + 2;

                if(historyRGB->prox != NULL)
                {
                    free_Historico_rgb(historyRGB->prox);
                    historyRGB->prox = NULL;
                }

                historyRGB = edicoesRGB(historyRGB, aux2);
                if(historyRGB != NULL)
                    salvar_rgb(historyRGB->img, pastaOriginal, historyRGB->nome);
            }
        }

        else
        {
            switch(op)
            {
                case ANTERIOR:
                    historyRGB = prev_Image_rgb(historyRGB);
                    break;

                case PROXIMO:
                    historyRGB = next_Image_rgb(historyRGB);
                    break;

                case SALVAR:
                    g_print("Salvando imagem \"%s\"...\n\n", historyRGB->nome);
                    salvar_rgb(historyRGB->img, pastaOriginal, historyRGB->nome);
                    break;

                case SAIR:
                    g_print("Encerrando programa...\n");
                    txtOriginal = liberarVetor(txtOriginal);
                    pastaOriginal = liberarVetor(pastaOriginal);
                    break;
            }
        }
    }
}

///////////// Interface Gráfica /////////////

void numeroDigitado(GtkEditable *entrada, gpointer user_data)
{
    const gchar *texto = gtk_editable_get_text(entrada);
    int *pont = (int *) user_data;

    if(validarNumero(texto))
    {
        int num = atoi(texto);
        if(num > 0)
            *pont = num;
    }
}

void imparDigitado(GtkEditable *entrada, gpointer user_data)
{
    const gchar *texto = gtk_editable_get_text(entrada);
    int *pont = (int *) user_data;

    if(validarNumero(texto))
    {
        int num = atoi(texto);
        if(num > 0 && num % 2 == 1)
            *pont = num;
    }
}


void criarBotaoGray(char *nome, int x, int y, int largura, int altura, int posFuncao)
{
    GtkWidget *button = gtk_button_new_with_label(nome);
    g_signal_connect(button, "clicked", G_CALLBACK(Executar_Gray), &funcoes[posFuncao]);
    gtk_grid_attach(GTK_GRID(grid), button, x, y, largura, altura);
}

void janelaGray(GtkFileDialog *dialog, GAsyncResult *res, gpointer window)
{
    GError *error = NULL;
    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);

    if(error != NULL)
    {
        g_print("Erro ao selecionar a imagem: %s\n", error->message);
        g_error_free(error);
    }
    else if(file != NULL)
    {
        char *imagemOriginal = g_file_get_path(file);
        g_print("Imagem selecionada: %s\n", imagemOriginal);
        g_object_unref(file);

        char *nome = nomeCaminho(imagemOriginal);
        
        char *caminhoOriginal = "imagens";
        pastaOriginal = pastaPrincipal(caminhoOriginal);
        char *caminho1 = gerarCaminho(pastaOriginal, "/", nome);
        txtOriginal = gerarCaminho(caminho1, ".", "txt");
    
        ImageGray *image = lerImagemGray(imagemOriginal, txtOriginal);

        historyGray = criar_lista_gray();
        historyGray = add_historico_gray(image, historyGray, nome); //adiciona a imagem original na cabeça da lista
        salvar_gray(image, pastaOriginal, nome);
        
        g_print("Imagem: \"%s\" foi adicionada\n\n", historyGray->nome);

        nome = liberarVetor(nome);

        tileBlurGlobal = 9;
        alturaClaheGlobal = 64;
        larguraClaheGlobal = 64;
        clipLimitGlobal = 40;
        randomGlobal = 3;

        criarBotaoGray("Transpose 1 ↗️", 0, TRANSPOSE1, 1, 1, TRANSPOSE1);
        criarBotaoGray("Transpose 2 ↘️", 0, TRANSPOSE2, 1, 1, TRANSPOSE2);
        criarBotaoGray("Rotate 90º 🔁", 0, ROTATE_90, 1, 1, ROTATE_90);
        criarBotaoGray("Rotate 90º 🔄", 0, ROTATE_90_ANTI, 1, 1, ROTATE_90_ANTI);
        criarBotaoGray("Flip Vertical", 0, VERTICAL, 1, 1, VERTICAL);
        criarBotaoGray("Flip Horizontal", 0, HORIZONTAL, 1, 1, HORIZONTAL);
        criarBotaoGray("Median Blur", 0, BLUR, 1, 1, BLUR);
        criarBotaoGray("CLAHE", 0, CLAHE, 1, 1, CLAHE);
        criarBotaoGray("Negativo", 0, NEGATIVO+1, 1, 1, NEGATIVO);
        criarBotaoGray("Aleatório", 0, ALEATORIO+1, 1, 1, ALEATORIO);
        criarBotaoGray("Imagem Anterior", 0, ANTERIOR+1, 1, 1, ANTERIOR);
        criarBotaoGray("Imagem Seguinte", 1, ANTERIOR+1, 1, 1, PROXIMO);
        criarBotaoGray("Salvar", 0, SALVAR, 1, 1, SALVAR);

        GtkWidget *entrada = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entrada), "Tamanho: ");
        g_signal_connect(entrada, "changed", G_CALLBACK(imparDigitado), &tileBlurGlobal);
        gtk_grid_attach(GTK_GRID(grid), entrada, 1, BLUR, 1, 1);

        entrada = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entrada), "Clip Limit: ");
        g_signal_connect(entrada, "changed", G_CALLBACK(numeroDigitado), &clipLimitGlobal);
        gtk_grid_attach(GTK_GRID(grid), entrada, 1, CLAHE, 1, 1);

        entrada = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entrada), "Largura: ");
        g_signal_connect(entrada, "changed", G_CALLBACK(numeroDigitado), &larguraClaheGlobal);
        gtk_grid_attach(GTK_GRID(grid), entrada, 0, CLAHE+1, 1, 1);

        entrada = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entrada), "Altura: ");
        g_signal_connect(entrada, "changed", G_CALLBACK(numeroDigitado), &alturaClaheGlobal);
        gtk_grid_attach(GTK_GRID(grid), entrada, 1, CLAHE+1, 1, 1);

        entrada = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entrada), "Quantidade: ");
        g_signal_connect(entrada, "changed", G_CALLBACK(numeroDigitado), &randomGlobal);
        gtk_grid_attach(GTK_GRID(grid), entrada, 1, ALEATORIO+1, 1, 1);

        exibirImagemGray(historyGray);
    }
    else
        g_print("Você cancelou\n");
}

void selecionaGray(GtkWidget *widget, gpointer window)
{
    // Cria uma nova caixa de diálogo de seleção de arquivos usando GtkFileDialog
    GtkFileDialog *dialog = gtk_file_dialog_new();

    // Configura as opções de diálogo (abrir arquivos)
    gtk_file_dialog_open(dialog,
                         GTK_WINDOW(window),
                         NULL,
                         (GAsyncReadyCallback) janelaGray,
                         window);
}


void criarBotaoRGB(char *nome, int x, int y, int largura, int altura, int posFuncao)
{
    GtkWidget *button = gtk_button_new_with_label(nome);
    g_signal_connect(button, "clicked", G_CALLBACK(Executar_RGB), &funcoes[posFuncao]);
    gtk_grid_attach(GTK_GRID(grid), button, x, y, largura, altura);
}

void janelaRGB(GtkFileDialog *dialog, GAsyncResult *res, gpointer window)
{
    GError *error = NULL;
    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);

    if(error != NULL)
    {
        g_print("Erro ao selecionar a imagem: %s\n", error->message);
        g_error_free(error);
    }
    else if(file != NULL)
    {
        char *imagemOriginal = g_file_get_path(file);
        g_print("Imagem selecionada: %s\n", imagemOriginal);
        g_object_unref(file);

        char *nome = nomeCaminho(imagemOriginal);
        
        char *caminhoOriginal = "imagens";
        pastaOriginal = pastaPrincipal(caminhoOriginal);
        char *caminho1 = gerarCaminho(pastaOriginal, "/", nome);
        txtOriginal = gerarCaminho(caminho1, ".", "txt");
    
        ImageRGB *image = lerImagemRGB(imagemOriginal, txtOriginal);
    
        historyRGB = criar_lista_rgb();
        historyRGB = add_historico_rgb(image, historyRGB, nome); //adiciona a imagem original na cabeça da lista
        salvar_rgb(image, pastaOriginal, nome);
        
        g_print("Imagem: \"%s\" foi adicionada\n\n", historyRGB->nome);

        nome = liberarVetor(nome);

        tileBlurGlobal = 9;
        alturaClaheGlobal = 64;
        larguraClaheGlobal = 64;
        clipLimitGlobal = 40;
        randomGlobal = 3;

        criarBotaoRGB("Transpose 1 ↗️", 0, TRANSPOSE1, 1, 1, TRANSPOSE1);
        criarBotaoRGB("Transpose 2 ↘️", 0, TRANSPOSE2, 1, 1, TRANSPOSE2);
        criarBotaoRGB("Rotate 90º 🔁", 0, ROTATE_90, 1, 1, ROTATE_90);
        criarBotaoRGB("Rotate 90º 🔄", 0, ROTATE_90_ANTI, 1, 1, ROTATE_90_ANTI);
        criarBotaoRGB("Flip Vertical", 0, VERTICAL, 1, 1, VERTICAL);
        criarBotaoRGB("Flip Horizontal", 0, HORIZONTAL, 1, 1, HORIZONTAL);
        criarBotaoRGB("Median Blur", 0, BLUR, 1, 1, BLUR);
        criarBotaoRGB("CLAHE", 0, CLAHE, 1, 1, CLAHE);
        criarBotaoRGB("Negativo", 0, NEGATIVO+1, 1, 1, NEGATIVO);
        criarBotaoRGB("Aleatório", 0, ALEATORIO+1, 1, 1, ALEATORIO);
        criarBotaoRGB("Imagem Anterior", 0, ANTERIOR+1, 1, 1, ANTERIOR);
        criarBotaoRGB("Imagem Seguinte", 1, ANTERIOR+1, 1, 1, PROXIMO);
        criarBotaoRGB("Salvar", 0, SALVAR, 1, 1, SALVAR);

        GtkWidget *entrada = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entrada), "Tamanho: ");
        g_signal_connect(entrada, "changed", G_CALLBACK(imparDigitado), &tileBlurGlobal);
        gtk_grid_attach(GTK_GRID(grid), entrada, 1, BLUR, 1, 1);

        entrada = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entrada), "Clip Limit: ");
        g_signal_connect(entrada, "changed", G_CALLBACK(numeroDigitado), &clipLimitGlobal);
        gtk_grid_attach(GTK_GRID(grid), entrada, 1, CLAHE, 1, 1);

        entrada = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entrada), "Largura: ");
        g_signal_connect(entrada, "changed", G_CALLBACK(numeroDigitado), &larguraClaheGlobal);
        gtk_grid_attach(GTK_GRID(grid), entrada, 0, CLAHE+1, 1, 1);

        entrada = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entrada), "Altura: ");
        g_signal_connect(entrada, "changed", G_CALLBACK(numeroDigitado), &alturaClaheGlobal);
        gtk_grid_attach(GTK_GRID(grid), entrada, 1, CLAHE+1, 1, 1);

        entrada = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entrada), "Quantidade: ");
        g_signal_connect(entrada, "changed", G_CALLBACK(numeroDigitado), &randomGlobal);
        gtk_grid_attach(GTK_GRID(grid), entrada, 1, ALEATORIO+1, 1, 1);

        exibirImagemRGB(historyRGB);
    }
    else
        g_print("Você cancelou\n");
}

void selecionaRGB(GtkWidget *widget, gpointer window)
{
    // Cria uma nova caixa de diálogo de seleção de arquivos usando GtkFileDialog
    GtkFileDialog *dialog = gtk_file_dialog_new();

    // Configura as opções de diálogo (abrir arquivos)
    gtk_file_dialog_open(dialog,
                         GTK_WINDOW(window),
                         NULL,
                         (GAsyncReadyCallback) janelaRGB,
                         window);
}


static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *button;

    window = gtk_application_window_new(app);

    gtk_window_set_title(GTK_WINDOW(window), "Edição de imagens");
    // gtk_window_set_default_size(GTK_WINDOW(window), 780, 1280);
    
    // Cria uma grid
    grid = gtk_grid_new();
    // "Linka" a grid com a janela
    gtk_window_set_child(GTK_WINDOW(window), grid);

    button = gtk_button_new_with_label("Abrir RGB");
    g_signal_connect(button, "clicked", G_CALLBACK(selecionaRGB), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 0, RGB, 1, 1);

    button = gtk_button_new_with_label("Abrir Gray");
    g_signal_connect(button, "clicked", G_CALLBACK(selecionaGray), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 0, GRAY, 1, 1);

    button = gtk_button_new_with_label("Sair");
    g_signal_connect(button, "clicked", G_CALLBACK(Executar_Gray), &funcoes[SAIR]);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_window_destroy), window);
    gtk_grid_attach(GTK_GRID(grid), button, 0, SAIR, 1, 1);

    gtk_window_present(GTK_WINDOW(window));
}

int iniciar(int argc, char **argv)
{
    GtkApplication *app;
    int status;
    srand(time(NULL));

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    
    return status;
}