#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "image.c"

int main()
{
    ImageGray *imagem = lerImagemGray("imagens/lena.png", "imagens", "lena_blur");
    imagem = median_blur_gray(imagem, 9);
    salvarImagemGray(imagem, "imagens", "lena_blur");

    return 0;
}