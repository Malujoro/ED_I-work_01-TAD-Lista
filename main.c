#include <stdio.h>
#include <stdlib.h>

int main()
{
    int tam = 128;
    char comando[tam];

    int tipo, cor;
    tipo = 1;
    cor = 1;
    char origem[] = "imagem.txt";
    char saida[] = "pasta/saida.png";

    snprintf(comando, tam, "python3 utils/image_utils.py %d %d %s %s", tipo, cor, origem, saida);

    printf("%s", comando);
    return 0;
}