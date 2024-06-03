#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "image.c"

int main()
{
    ImageGray *imagem = lerTxtGray("imagens", "lena");
    imagem = median_blur_gray(imagem, 9);
    salvarTxtGray(imagem, "imagens", "lena_blur");

    return 0;
}