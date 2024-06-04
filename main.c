#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "image.c"

int main()
{
    ImageGray *imagem = lerTxtGray("output/imagens", "lena");
    // imagem = median_blur_gray(imagem, 9);
    imagem = clahe_gray(imagem, 128, 128);
    salvarTxtGray(imagem, "output/imagens", "lena_clahe");

    return 0;
}