#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// defines definidos na especificacao do trabalho
#define MAXVIDAGALINHA 20
#define MAXCARROSPORPISTA 20
#define MAXPISTAS 12
#define NUMERODEFRAMESCARRO 4

typedef struct{
    int alturamaximachegou;
    int alturamaximamorreu;
    int alturamaximamorreu_aux;
    int colisao;
    int menorcolisao;
    int alturaminimamorreu;
    int numerodirecaooposta;
    int totalmovimentos;
} tEstatisticas;

typedef struct{
    int iteracoes[MAXVIDAGALINHA];
    int carros[MAXVIDAGALINHA];
    int pista[MAXVIDAGALINHA];
    int totaliteracoes;
} tRanking;

typedef struct{
    int coluna;
    int pistas;
    int linhas;
} tMapa;

typedef struct {
    char direcao;
    int velocidade;
    int num_carros;
    int posicoes_carros[MAXCARROSPORPISTA];
} tPista;

typedef struct {
    int colunagalinha;
    int linhagalinha;
    int possivel_linha_galinha;
    int vidas;
    char G;
} tGalinha;

typedef struct{
    tEstatisticas estatisticas;
    tRanking ranking;
    tMapa mapa;
    tPista pista[MAXPISTAS];
    tGalinha galinha;
    int pontos;
    int iteracoes;
    int modo;
    int modoatual;
    char movimento;
    char modelogalinha[6];
    char modelocarro[NUMERODEFRAMESCARRO][6];
    int heatmap[100][100];
    int linhasdascolisoes[MAXPISTAS];
    int auxcolisoes;
} tJogo;

tJogo EntraInfo(int argc, char argv[]){
    tJogo jogo = {0};

    if(argc == 1){
        printf("ERRO: Informe o diretorio com os arquivos de configuracao");
        exit(1);
    }

    FILE *pFileConfig;
    char nome[1000];
    sprintf(nome, "%s/config_inicial.txt", argv);
    // abre o arquivo para leitura "read"
    pFileConfig = fopen (nome, "r");

    if (!pFileConfig){
        printf("ERRO: O diretorio nao foi encontrado no diretorio %s", argv);
        exit(1);
    }

    //scan do mapa/modo
    fscanf(pFileConfig, " %d", &jogo.modo);
    fscanf(pFileConfig, " %d %d", &jogo.mapa.coluna, &jogo.mapa.pistas);

    int i, j;

    //scan dos carros
    fgetc(pFileConfig);

    for (i=0; i<jogo.mapa.pistas-1; i++) {
        char primeirochar;
        primeirochar = fgetc(pFileConfig);
   
        if (primeirochar == 'D' || primeirochar == 'E') {
            jogo.pista[i].direcao = primeirochar;

            fscanf(pFileConfig, "%d %d", &jogo.pista[i].velocidade, &jogo.pista[i].num_carros);

            for (j = 0; j < jogo.pista[i].num_carros; j++) {
                fscanf(pFileConfig, "%d", &jogo.pista[i].posicoes_carros[j]);
            }
       
            fgetc(pFileConfig);

        } else {
            jogo.pista[i].num_carros = 0;
        }
    }

    //scan das infos da galinha
    fscanf(pFileConfig, " %c", &jogo.galinha.G);
    fscanf(pFileConfig, " %d", &jogo.galinha.colunagalinha);
    fscanf(pFileConfig, " %d", &jogo.galinha.vidas);

    // fecha arquivo config_inicial.txt
    fclose(pFileConfig);

    // SEGUNDA PARTE DO SCAN, AGORA ESCANEANDO OS PERSONAGENS
    FILE *pFilePersonagens;
    char nomes[1000];
    sprintf(nomes, "%s/personagens.txt", argv);
    // abre o arquivo para leitura "read"
    pFilePersonagens = fopen (nomes, "r");

    if (!pFilePersonagens){
        printf("ERRO: O diretorio nao foi encontrado no diretorio %s", argv);
        exit(1);
    }

    //scan do galo
    for (i=0; i<6; i++){
        jogo.modelogalinha[i] = fgetc(pFilePersonagens);
        if(i == 2){
            fgetc(pFilePersonagens);
        }
    }

    fgetc(pFilePersonagens);

    // scan do carro
    for(j=0; j<4; j++){
        for (i=0; i<6; i++){
            jogo.modelocarro[j][i] = fgetc(pFilePersonagens);
            if(i == 2){
                fgetc(pFilePersonagens);
            }
        }
        fgetc(pFilePersonagens);
    }

    // fecha arquivo personagens.txt
    fclose(pFilePersonagens);

    return jogo;
}

void FazMapa(tJogo jogo){
    int i, j, k;

    char MatrizMae[jogo.mapa.linhas][jogo.mapa.coluna];

    // preenchendo a matriz/limpando a matriz
    for(i=0; i<jogo.mapa.linhas; i++){
        for(j=0; j<jogo.mapa.coluna; j++){
            if((i+1) % 3 == 0){
                if((j+1) % 3 == 0){
                    MatrizMae[i][j] = ' ';
                } else MatrizMae[i][j] = '-';
            } else MatrizMae[i][j] = ' ';
        } 
    }
 
    for (i = 0; i < jogo.mapa.pistas - 1; i++) {
        for (j = 0; j < jogo.pista[i].num_carros; j++) {
            int ydocarro;
            ydocarro = i * 3;
    
            // esquerda (k=-1), meio (k=0) direita (k=1)
            for (k = -1; k <= 1; k++) {
                int xdocarro;
                xdocarro = (jogo.pista[i].posicoes_carros[j] - 1 + k + jogo.mapa.coluna) % jogo.mapa.coluna;

                if(jogo.modo == 1){
                    // bonus parte de cima
                    MatrizMae[ydocarro][xdocarro] = jogo.modelocarro[jogo.modoatual][k+1];
                    // bonus parte de baixo
                    MatrizMae[ydocarro + 1][xdocarro] = jogo.modelocarro[jogo.modoatual][k+4];
                } else {
                    // parte de cima
                    MatrizMae[ydocarro][xdocarro] = jogo.modelocarro[0][k+1];
                    // parte de baixo
                    MatrizMae[ydocarro + 1][xdocarro] = jogo.modelocarro[0][k+4];
                }
            } 
        } 
    }
    
    MatrizMae[jogo.galinha.linhagalinha - 1][jogo.galinha.colunagalinha-2] = jogo.modelogalinha[0];
    MatrizMae[jogo.galinha.linhagalinha - 1][jogo.galinha.colunagalinha-1] = jogo.modelogalinha[1];
    MatrizMae[jogo.galinha.linhagalinha - 1][jogo.galinha.colunagalinha] = jogo.modelogalinha[2];
    MatrizMae[jogo.galinha.linhagalinha][jogo.galinha.colunagalinha-2] = jogo.modelogalinha[3];
    MatrizMae[jogo.galinha.linhagalinha][jogo.galinha.colunagalinha-1] = jogo.modelogalinha[4];
    MatrizMae[jogo.galinha.linhagalinha][jogo.galinha.colunagalinha] = jogo.modelogalinha[5];

    // limite superior
    printf("Pontos: %d | Vidas: %d | Iteracoes: %d\n", jogo.pontos, jogo.galinha.vidas, jogo.iteracoes);
    
    printf("|");
    for(i=0; i<jogo.mapa.coluna; i++){
        printf("-");
    } printf("|\n");
    
    // mapa
    for(i=0; i<jogo.mapa.linhas; i++){
        printf("|");
        for(j=0; j<jogo.mapa.coluna; j++){
            printf("%c", MatrizMae[i][j]);
        }
        printf("|\n");
    }

    // limite inferior
    printf("|");
    for(i=0; i<jogo.mapa.coluna; i++){
        printf("-");
    } printf("|\n");
}

tJogo NumeroDirecaoOpostaAdd(tJogo jogo){
    jogo.estatisticas.numerodirecaooposta++;
    return jogo;
}

tJogo TotalMovimentosAdd(tJogo jogo){
    jogo.estatisticas.totalmovimentos++;
    return jogo;
}

tJogo CalculaTurno(tJogo jogo){
    char opcao;

    scanf("%c", &opcao);

    while (opcao != 'w' && opcao != 's' && opcao != ' '){
        scanf("%c", &opcao);
    }

    int i, j;
   
    for (i = 0; i<jogo.mapa.pistas-1; i++) {
        for (j = 0; j<jogo.pista[i].num_carros; j++) {
            if (jogo.pista[i].direcao == 'D') {
             
                jogo.pista[i].posicoes_carros[j] = (jogo.pista[i].posicoes_carros[j] + jogo.pista[i].velocidade) % jogo.mapa.coluna;
            }

            if (jogo.pista[i].direcao == 'E') {
                jogo.pista[i].posicoes_carros[j] = ((jogo.pista[i].posicoes_carros[j] - jogo.pista[i].velocidade) % jogo.mapa.coluna + jogo.mapa.coluna) % jogo.mapa.coluna;
            }
        }
    }

    if(opcao == 'w'){
        jogo.galinha.possivel_linha_galinha = jogo.galinha.linhagalinha-3;
        jogo = TotalMovimentosAdd(jogo);
    } 

    if (opcao == 's'){
        jogo = TotalMovimentosAdd(jogo);
        jogo = NumeroDirecaoOpostaAdd(jogo);

        if(jogo.galinha.linhagalinha + 3 < jogo.mapa.linhas){
            jogo.galinha.possivel_linha_galinha = jogo.galinha.linhagalinha+3;   
        }
    }
    
    if (opcao == ' '){
        jogo.galinha.possivel_linha_galinha = jogo.galinha.linhagalinha;
    }

    jogo.movimento = opcao;
        
    return jogo;
}

tJogo IteracoesAdd(tJogo jogo){
    jogo.iteracoes++;
    return jogo;
}

tJogo AuxColisoesAdd(tJogo jogo){
    jogo.auxcolisoes++;
    return jogo;
}

tJogo PontosJogoAdd(tJogo jogo){
    jogo.pontos++;

    return jogo;
}

tJogo AtualizaRanking(tJogo jogo, int i, int carroquecolidiu){
    jogo.ranking.pista[jogo.ranking.totaliteracoes] = i+1;
    jogo.ranking.carros[jogo.ranking.totaliteracoes] = carroquecolidiu + 1;
    jogo.ranking.iteracoes[jogo.ranking.totaliteracoes] = jogo.iteracoes;
    jogo.ranking.totaliteracoes++;

    return jogo;
}

tJogo AtualizaGalinha(tJogo jogo){
    jogo.galinha.vidas--;
    jogo.pontos=0;
    jogo.galinha.linhagalinha = jogo.mapa.linhas - 1;

    return jogo;
}

tJogo DiminuiVelocidadePista(tJogo jogo, int i){
    jogo.pista[i].velocidade--;

    return jogo;
}

void FazResumo(FILE * pFileResumo, tJogo jogo, int carroquecolidiu, int i){
    fprintf(pFileResumo, "[%d] Na pista %d o carro %d atropelou a galinha na posicao (%d,%d).\n", jogo.iteracoes, i+1, carroquecolidiu+1, jogo.galinha.colunagalinha, jogo.galinha.possivel_linha_galinha);
}

tJogo ChecaPossivelColisao(tJogo jogo, char * argv){
    FILE *pFileResumo;
    char nome[1000];
    sprintf(nome, "%s/saida/resumo.txt", argv);
    pFileResumo = fopen(nome, "a");

    int i, j;
    int carroquecolidiu;
    int colidiu = 0;
    
    i = jogo.galinha.possivel_linha_galinha / 3;
    
    jogo = IteracoesAdd(jogo);
    
    for (j = 0; j < jogo.pista[i].num_carros; j++) {
        if ((jogo.galinha.colunagalinha + 1) >= jogo.pista[i].posicoes_carros[j] - 1 && (jogo.galinha.colunagalinha - 1) <= jogo.pista[i].posicoes_carros[j] + 1) {
            carroquecolidiu = j;
            colidiu = 1;
            break;       
        }
    }

    if (colidiu) {
        // bonus
        if(jogo.modo == 1){
            if(jogo.pista[i].velocidade > 1){
                jogo = DiminuiVelocidadePista(jogo, i);
            }
        }
        // heatmap
        jogo.linhasdascolisoes[jogo.auxcolisoes] = jogo.galinha.possivel_linha_galinha;
        jogo = AuxColisoesAdd(jogo);

        // estatisticas
        jogo.estatisticas.colisao = jogo.galinha.possivel_linha_galinha;

        // ranking
        jogo = AtualizaRanking(jogo, i, carroquecolidiu);
        
        // galinha perde vida, zera os pontos, volta ao inicio
        jogo = AtualizaGalinha(jogo);
      
        // checar morte mais baixo e mais alta

        if (jogo.estatisticas.colisao > jogo.estatisticas.menorcolisao){
            jogo.estatisticas.menorcolisao = jogo.estatisticas.colisao;
        }

        if (jogo.galinha.possivel_linha_galinha < jogo.estatisticas.alturamaximamorreu){
            jogo.estatisticas.alturamaximamorreu = jogo.galinha.possivel_linha_galinha;
        }

        // se colidiu gera resumo
        FazResumo(pFileResumo, jogo, carroquecolidiu, i);

    } else {
        // se nao bateu, atualiza a galinha

        jogo.galinha.linhagalinha = jogo.galinha.possivel_linha_galinha;

        // atualizar altura maxima que chegou

        if (jogo.estatisticas.alturamaximachegou > jogo.galinha.linhagalinha){
            jogo.estatisticas.alturamaximachegou = jogo.galinha.linhagalinha;
            jogo.estatisticas.alturamaximamorreu_aux++;
        }

        if(jogo.movimento == 'w'){
            jogo = PontosJogoAdd(jogo);
        }
    }

    // se chega no topo, ganha 10 pontos
    if(jogo.galinha.linhagalinha == 1){
        jogo.pontos = jogo.pontos + 10;
    }

    fclose(pFileResumo);

    return jogo;
}

void Inicializacao(tJogo jogo, char argv[]){
    FILE *pFileInicializacao;
    char nome[1000];
    sprintf(nome, "%s/saida/inicializacao.txt", argv);
    pFileInicializacao = fopen(nome, "w");

    int i, j;
    char MatrizMae[jogo.mapa.linhas][jogo.mapa.coluna];

    // preenchendo a matriz/limpando a matriz

    for(i=0; i<jogo.mapa.linhas; i++){
        for(j=0; j<jogo.mapa.coluna; j++){
            if((i+1) % 3 == 0){
                if((j+1) % 3 == 0){
                    MatrizMae[i][j] = ' ';
                } else MatrizMae[i][j] = '-';
            } else MatrizMae[i][j] = ' ';
        } 
    }

    int k;

    for (i = 0; i < jogo.mapa.pistas - 1; i++) {
        for (j = 0; j < jogo.pista[i].num_carros; j++) {
        int ydocarro = i * 3;
    
            // esquerda (k=-1), meio (k=0) direita (k=1)
            for (k = -1; k <= 1; k++) {
                // como qualquer vetor começa em 0, tem o -1 ali pra compensar, se um mapa tem 25 colunas, a coluna 25 na verdade esta no index 24
                int xdocarro;
                xdocarro = (jogo.pista[i].posicoes_carros[j] - 1 + k + jogo.mapa.coluna) % jogo.mapa.coluna;
                // parte de cima
                MatrizMae[ydocarro][xdocarro] = jogo.modelocarro[0][k+1];
                // parte de baixo
                MatrizMae[ydocarro + 1][xdocarro] = jogo.modelocarro[0][k+4];
            }
        } 
    } 
    
    MatrizMae[jogo.galinha.linhagalinha - 1][jogo.galinha.colunagalinha-2] = jogo.modelogalinha[0];
    MatrizMae[jogo.galinha.linhagalinha - 1][jogo.galinha.colunagalinha-1] = jogo.modelogalinha[1];
    MatrizMae[jogo.galinha.linhagalinha - 1][jogo.galinha.colunagalinha] = jogo.modelogalinha[2];
    MatrizMae[jogo.galinha.linhagalinha][jogo.galinha.colunagalinha-2] = jogo.modelogalinha[3];
    MatrizMae[jogo.galinha.linhagalinha][jogo.galinha.colunagalinha-1] = jogo.modelogalinha[4];
    MatrizMae[jogo.galinha.linhagalinha][jogo.galinha.colunagalinha] = jogo.modelogalinha[5];

    // limite superior
    fprintf(pFileInicializacao, "|");
    for(i=0; i<jogo.mapa.coluna; i++){
        fprintf(pFileInicializacao, "-");
    } fprintf(pFileInicializacao, "|\n");
    
    // mapa
    for(i=0; i<jogo.mapa.linhas; i++){
        fprintf(pFileInicializacao, "|");
        for(j=0; j<jogo.mapa.coluna; j++){
            fprintf(pFileInicializacao, "%c", MatrizMae[i][j]);
        }
        fprintf(pFileInicializacao, "|\n");
    }
    
    // limite inferior
    fprintf(pFileInicializacao, "|");
    for(i=0; i<jogo.mapa.coluna; i++){
        fprintf(pFileInicializacao, "-");
    } fprintf(pFileInicializacao, "|\n");

    fprintf(pFileInicializacao, "A posicao central da galinha iniciara em (%d %d).", jogo.galinha.colunagalinha, jogo.galinha.linhagalinha);

    fclose(pFileInicializacao);
}

void OrdenaRanking(tRanking ranking, char argv[]){
    FILE *pFileRanking;
    char nome[1000];
    sprintf(nome, "%s/saida/ranking.txt", argv);
    pFileRanking = fopen(nome, "w");

    int i, j, min, auxpista, auxcarro, auxiteracao;

    for(i=0; i<ranking.totaliteracoes-1; i++){
        min = i;
        for(j=i+1; j<ranking.totaliteracoes; j++){
            if (ranking.pista[j] < ranking.pista[min]){
                min = j;
            } else {
                if (ranking.pista[j] == ranking.pista[min]){
                    if(ranking.carros[j] < ranking.carros[min]){
                        min = j;
                    } else {
                        if(ranking.carros[j] == ranking.carros[min]){
                            if (ranking.iteracoes[j] > ranking.iteracoes[min]){
                                min = j;
                            }
                        }
                    }
                }
            }     
        }

        // selection sort da pista que colidiu, carro que atingiu e iteracao
        auxpista = ranking.pista[i];
        ranking.pista[i] = ranking.pista[min];
        ranking.pista[min] = auxpista;

        auxcarro = ranking.carros[i];
        ranking.carros[i] = ranking.carros[min];
        ranking.carros[min] = auxcarro;

        auxiteracao = ranking.iteracoes[i];
        ranking.iteracoes[i] = ranking.iteracoes[min];
        ranking.iteracoes[min] = auxiteracao;
    }

    fprintf(pFileRanking, "id_pista,id_carro,iteracao\n");

    for (i=0; i<ranking.totaliteracoes; i++){
        fprintf(pFileRanking, "%d,%d,%d\n", ranking.pista[i], ranking.carros[i], ranking.iteracoes[i]);
    }

    fclose(pFileRanking);
}

void FazEstatisticas(tJogo jogo, char argv[]){
    FILE *pFileEstatistica;
    char nome[1000];
    sprintf(nome, "%s/saida/estatistica.txt", argv);
    pFileEstatistica = fopen(nome, "w");

    jogo.estatisticas.alturamaximachegou = (jogo.mapa.pistas*3) - jogo.estatisticas.alturamaximachegou;
    
    if(jogo.estatisticas.alturamaximamorreu == 999 && jogo.estatisticas.alturaminimamorreu == 999){
        jogo.estatisticas.alturamaximamorreu = 0;
        jogo.estatisticas.alturaminimamorreu = 0;
    }

    if(jogo.estatisticas.alturamaximamorreu != 0){
        jogo.estatisticas.alturamaximamorreu = (jogo.mapa.pistas*3) - jogo.estatisticas.alturamaximamorreu;
    }
    
    if(jogo.estatisticas.alturaminimamorreu != 0){
        jogo.estatisticas.alturaminimamorreu = (jogo.mapa.pistas*3) - jogo.estatisticas.menorcolisao;
    }

    fprintf(pFileEstatistica, "Numero total de movimentos: %d\nAltura maxima que a galinha chegou: %d\nAltura maxima que a galinha foi atropelada: %d\nAltura minima que a galinha foi atropelada: %d\nNumero de movimentos na direcao oposta: %d", jogo.estatisticas.totalmovimentos, jogo.estatisticas.alturamaximachegou, jogo.estatisticas.alturamaximamorreu, jogo.estatisticas.alturaminimamorreu, jogo.estatisticas.numerodirecaooposta);

    fclose(pFileEstatistica);
}


tJogo AtualizaHeatmap(tJogo jogo){
    int i, j;

    for(i=-1; i<1; i++){
        for(j=-2; j<1; j++){
            jogo.heatmap[jogo.galinha.linhagalinha + i][jogo.galinha.colunagalinha + j]++;
        }
    }
    

    for(i=-1; i<1; i++){
        for(j=-2; j<1; j++){
            if (jogo.heatmap[jogo.galinha.linhagalinha + i][jogo.galinha.colunagalinha + j] > 99){
                jogo.heatmap[jogo.galinha.linhagalinha + i][jogo.galinha.colunagalinha + j] = 99;
            }      
        }
    }
    
    return jogo;
}

tJogo InicializacaoEstatisticas(tJogo jogo){
    jogo.estatisticas.alturaminimamorreu = 999;
    jogo.estatisticas.alturamaximachegou = 999;
    jogo.estatisticas.alturamaximamorreu = 999;

    return jogo;
}

void PrintaHeatMap(tJogo jogo, char argv[]){
    FILE *pFileHeatMap;
    char nome[1000];
    sprintf(nome, "%s/saida/heatmap.txt", argv);
    pFileHeatMap = fopen(nome, "w");

    int i, j, k;
    int linhadecolisao;

    for (i = 0; i < jogo.mapa.linhas; i++) {
        for (j = 0; j < jogo.mapa.coluna; j++) {
            int linhadecolisao = 0;
            for (k = 0; k < jogo.auxcolisoes; k++) {
                if (i == jogo.linhasdascolisoes[k] || i == jogo.linhasdascolisoes[k] - 1) {
                    linhadecolisao = 1;
                    break; 
                }
            }

            if (linhadecolisao == 1) {
                fprintf(pFileHeatMap, "  *"); 
            } else {
                fprintf(pFileHeatMap, "%3d", jogo.heatmap[i][j]);
            }
        }
        fprintf(pFileHeatMap, "\n");
    }
}

tJogo DefineLinhas(tJogo jogo){
    jogo.mapa.linhas = (jogo.mapa.pistas*3)-1;
    jogo.galinha.linhagalinha = jogo.mapa.linhas-1;

    return jogo;
}

int FimDeJogo(tJogo jogo, char * argv){
    FILE *pFileResumo;
    char nome[1000];
    sprintf(nome, "%s/saida/resumo.txt", argv);
    pFileResumo = fopen(nome, "a");

    // se a galinha chegou ao topo, ganha, caso zera a vida, perde
    if(jogo.galinha.linhagalinha == 1){
            printf("Parabens! Voce atravessou todas as pistas e venceu!");
            fprintf(pFileResumo, "[%d] Fim de jogo", jogo.iteracoes);
            return 0;
    } else{
        if(jogo.galinha.vidas == 0){
            printf("Voce perdeu todas as vidas! Fim de jogo.");
            fprintf(pFileResumo, "[%d] Fim de jogo", jogo.iteracoes);
            return 0;
        }
    }

    fclose(pFileResumo);

    return 1;
}

tJogo AtualizaFrameCarro(tJogo jogo){
    jogo.modoatual = jogo.iteracoes % NUMERODEFRAMESCARRO;
    
    return jogo;
}

int main(int argc, char * argv[]){
    tJogo jogo;

    // entrada de todas as informacoes (config inicial e personagens)
    jogo = EntraInfo(argc, argv[1]);

    // calculos necessarios pro funcionamento das funcoes e saidas
    jogo = DefineLinhas(jogo);
    jogo = InicializacaoEstatisticas(jogo);

    // atualiza o heatmap com a posicao inicial da galinha
    jogo = AtualizaHeatmap(jogo);
    
    // faz a saida inicializacao
    Inicializacao(jogo, argv[1]);

    // faz o mapa inicial, o mesmo da inicializacao, a unica diferença é que um printa na entrada padrao e um num arquivo txt
    FazMapa(jogo);

    //loop que contem as entradas do usuario, checa e atualiza o mapa e o heatmap, alem de printar o mapa
    while(FimDeJogo(jogo, argv[1]) == 1){
        jogo = CalculaTurno(jogo);
        jogo = ChecaPossivelColisao(jogo, argv[1]);
        jogo = AtualizaHeatmap(jogo);
        // bonus
        jogo = AtualizaFrameCarro(jogo);

        FazMapa(jogo);
    }

    // faz as saidas restantes
    FazEstatisticas(jogo, argv[1]);
    OrdenaRanking(jogo.ranking, argv[1]);
    PrintaHeatMap(jogo, argv[1]);

    return 0;
}