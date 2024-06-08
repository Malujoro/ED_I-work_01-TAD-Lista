#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "image.c"

int main()
{
    char *caminhoOriginal = "imagens";
    char *txtOriginal = gerarCaminho(caminhoOriginal, "/", "lena.txt");
    char *imagemOriginal = "utils/lena.png";

    char *pasta = pastaPrincipal(caminhoOriginal);
    char *caminho = gerarCaminho(pasta, "/", "lenaNegativo");
    char *txt = gerarCaminho(caminho, ".", "txt");
    char *png = gerarCaminho(caminho, ".", "png");

    
    // ImageGray *imagem = lerImagemGray(imagemOriginal, txtOriginal);
    ImageGray *imagem = lerTxtGray(txtOriginal);
    // imagem = median_blur_gray(imagem, 7);
    // imagem = clahe_gray(imagem, 64, 128);
    imagem = negativo(imagem);
    salvarImagemGray(imagem, pasta, txt, png);

    return 0;
}