#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "image.c"

int main()
{
    Geral();
    // char *caminhoOriginal = "imagens";
    // char *txtOriginal = gerarCaminho(caminhoOriginal, "/", "lena.txt");
    // char *imagemOriginal = "utils/lena.png";

    // char *pasta = pastaPrincipal(caminhoOriginal);
    // char *caminho = gerarCaminho(pasta, "/", "lenaTranspose");
    // char *txt = gerarCaminho(caminho, ".", "txt");
    // char *png = gerarCaminho(caminho, ".", "png");

    // ImageGray *imagem = lerImagemGray(imagemOriginal, txtOriginal);
    //ImageGray *imagem = lerTxtGray(txtOriginal);
    // imagem = median_blur_gray(imagem, 7);
  
    // imagem = clahe_gray(imagem, 64, 128);
    //imagem = negativo_gray(imagem);
    // imagem = flip_vertical_gray(imagem);
    // salvarImagemGray(imagem, pasta, txt, png);

    return 0;
}