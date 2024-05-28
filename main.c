#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "image.c"

int main()
{
    ImageGray *imagem = lerImagemGray("utils/output/lena.txt");
    imagem = median_blur_gray(imagem, 511);
    salvarTxtGray(imagem, "utils/output", "blur511.txt");
    // int tam = 128;
    // char comando[tam];

    // int tipo, cor;
    // tipo = 1;
    // cor = 1;
    // char origem[] = "imagem.txt";
    // char saida[] = "pasta/saida.png";

    // snprintf(comando, tam, "python3 utils/image_utils.py %d %d %s %s", tipo, cor, origem, saida);

    // printf("%s", comando);
    return 0;
}