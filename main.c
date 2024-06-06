#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "image.c"

int main()
{
    txt_from_image("utils/lena.png", "imagens/lenaTeste.txt", 1);
    
    ImageGray *imagem = lerTxtGray("imagens", "lenaTeste");
    imagem = median_blur_gray(imagem, 15);
    // imagem = clahe_gray(imagem, 8, 8);
    salvarTxtGray(imagem, "imagens", "lenaTeste");

    image_from_txt("imagens/lenaTeste.txt", "imagens/lenaTeste.png", 1);
    return 0;
}