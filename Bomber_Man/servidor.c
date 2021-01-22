#include"header.h"

int processa_comando(char *comando) {

    int conta_espacos = 0;
    int i;
    for (i = 0; i < 26; i++) {
        if (comando[i] == ' ') {
            conta_espacos++;
        }
    }

    char *word;
    word = strtok(comando, " ");
    while (word != NULL) {


        if (strcmp("users", word) == 0) {
            word = strtok(NULL, " ");
            if (word == NULL) {
                return 2;
            }
        } else {
            if (strcmp("game", word) == 0) {
                word = strtok(NULL, " ");
                if (word == NULL) {
                    return 4;
                }
            } else {
                if (strcmp("shutdown", word) == 0) {
                    word = strtok(NULL, " ");
                    if (word == NULL) {
                        return 5;
                    }
                } else {
                    if (strcmp("kick", word) == 0 && conta_espacos == 1) {
                        word = strtok(NULL, " ");
                        return 3;
                    } else {
                        if (strcmp("map", word) == 0 && conta_espacos == 1) {
                            word = strtok(NULL, " ");
                            if (strcmp("nome-ficheiro", word) == 0) {
                                word = strtok(NULL, " ");
                                if (word == NULL) {
                                    return 6;
                                }
                            }
                        } else {
                            if (strcmp("add", word) == 0 && conta_espacos == 2) {
                                return 1;
                            } else {
                                printf("Erro de sintaxe\n\n");
                                return 0;
                            }
                        }
                    }
                }
            }
        }
    }
}

void users_ativos(pontuser u, int users) {// mostra os users que fizeram login com sucesso no servidor
    int i;
    int temp = 0;

    printf("Utilizadores online:\n");

    for (i = 0; i < users; i++) {
        if (u[i].estado == 1) {
            printf("%s estado: %d a_jogar: %d\n", u[i].nome, u[i].estado, u[i].a_jogar);
            temp++;
        }
    }
    if (temp == 0) {
        printf("Nao ha utilizadores ativos\n\n");
    }
}

pontuser kick_username(pontuser u, int users, char *comando) {  //faz com que um determinado user saia do servidor
    int i;
    char *utilizador, *cmd;
    int fd;
    char fifo_labirinto[29] = {0};

    cmd = strtok(comando, " ");
    utilizador = strtok(NULL, " ");

    for (i = 0; i < users; i++) {
        if (strcmp(utilizador, u[i].nome) == 0) {
            if (u[i].estado == 0) {
                printf("%s nao esta online\n\n", utilizador);
                return u;
            }
            strcpy(fifo_labirinto, "labirinto_");
            strcat(fifo_labirinto, u[i].nome);
            unlink(fifo_labirinto);     //elimina os fifos referentes ao utilizador
            unlink(utilizador);     
            u[i].ultimo_com = 'd';
            printf("%s expulso!\n\n", utilizador);
            return u;
        }
    }
    printf("%s nao e um utilizador valido\n\n", utilizador);
    return u;
}

void game(pontuser u, int users, int online, labirinto lab) {       //mostra os users,e os seus pontos, objetos restantes para o jogo terminar e o mapa atual.
    int i, j;
    printf("\n%d Utilizadores a jogar:\n", online);
    for (i = 0; i < users; i++) {
        if (u[i].a_jogar == 1) {
            printf("Nome: %s Pontos: %d\n", u[i].nome, u[i].pontos);
        }
    }
    if (online != 0) {
        printf("\nObjetos Restantes: %d\n\n", lab.nObjetos);
    }
    if (online == 0) {
        printf("Não há jogadores a jogar\n\n");
    }
    for(i=0;i<lab.altura;i++){
        for(j=0;j<lab.largura;j++){
            printf("%c",lab.mapa[i][j]);
        }
        printf("\n");
    }
}

void trata(int sinal) {     //sinal SIGUSR1 para encerrar o servidor
    if (sinal == SIGUSR1) {
        exit(0);
    }
}

void shutdown(pontuser u, int users) {  //encerra o servidor, fechando todos os fifos
    int i;
    mensagem msg;
    char fifo_labirinto[29] = {0};

    for (i = 0; i < users; i++) {
        if (u[i].estado == 1) {

            strcpy(fifo_labirinto, "labirinto_");
            strcat(fifo_labirinto, u[i].nome);
            unlink(fifo_labirinto);

            unlink(u[i].nome);
        }
    }
    unlink("fifologin");
    unlink("labirinto");
    unlink("fifo_comandos");
    raise(SIGUSR1);
}

pontuser ler_vector(int *users, char *ficheiro) {   //le o ficheiro de texto contendo os users registados

    pontuser u;
    int i = 0;
    FILE *f;
    int tamanho;
    int novo_user = 0;

    if (*users > 0) {
        novo_user = 1;
    }


    f = fopen(ficheiro, "rt");
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro");
        fflush(stdout);
        exit(0);
    }

    fseek(f, 0, SEEK_END);  
    tamanho = ftell(f);     //verifica se há utilizadores registados
    fseek(f, 0, SEEK_SET);

    if (tamanho == 0) {
        printf("\nNao ha users registados\n");
        return u;
    }

    while (!feof(f)) {
        if (i == 0) {
            u = malloc(sizeof (user));
            if (u == NULL) {
                printf("Erro na alocacao da memoria");
                exit(0);
            }
        } else {
            u = realloc(u, (i + 1) * sizeof (user));
            if (u == NULL) {
                printf("Erro na alocacao da memoria");
                exit(0);
            }
        }

        fscanf(f, "%s %s", u[i].nome, u[i].pass);
        u[i].estado = 0;
        u[i].pontos = 0;
        u[i].a_jogar = 0;
        u[i].altura = 0;
        u[i].largura = 0;
        u[i].ultimo_com = 'd';
        i++;
        if (novo_user == 0) {
            *users += 1;
        }
    }
    fclose(f);
    return u;
}

pontuser adiciona(pontuser u, int *users, char *ficheiro, char *comando) { //faz o registo de novos utilizadores, atualizando também o ficheiro de texto
    int i;
    char *cmd, *nome, *pass;
    cmd = strtok(comando, " ");
    nome = strtok(NULL, " ");
    pass = strtok(NULL, " ");

    for (i = 0; i<*users; i++) {
        if (strcmp(nome, u[i].nome) == 0) {
            printf("\nNome de utilizador ja utilizado\n");
            return u;
        }
    }

    *users += 1;

    if (*users == 1) {
        u = malloc(sizeof (user));
    } else if (*users > 1) {
        u = realloc(u, (*users) * sizeof (user));
    }

    if (u == NULL) {
        printf("Erro na alocacao da memoria");
        exit(0);
    }

    strcpy(u[i].nome, nome);
    strcpy(u[i].pass, pass);

    u[i].estado = 0;
    u[i].pontos = 0;
    u[i].a_jogar = 0;
    u[i].altura = 0;
    u[i].largura = 0;
    u[i].ultimo_com = 'd';

    FILE *f;
    f = fopen(ficheiro, "a");
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro");
        return u;
    }

    if (i == 0) {
        fprintf(f, "%s %s", u[i].nome, u[i].pass);
    } else {
        fprintf(f, "\n%s %s", u[i].nome, u[i].pass);
    }
    printf("Utilizador adicionado com sucesso!\n");

    fclose(f);
    return u;
}

void *login(void *dados) {//trata dos logins dos users, e ve se o mapa está com demasiados jogadores
    int fd;
    int fd2;
    int i, j;
    int *sair;
    int estado = 0;
    int responde;
    pthread *x = (pthread*) dados;
    int *users = x->users;
    pontuser u = x->u;
    int *online = x->online;

    char resposta[200];
    mensagem msg;

    fd = open("fifologin", O_RDWR);

    while (1) {
        i = read(fd, &msg, sizeof (msg));

        if (i == sizeof (msg)) {

            u = ler_vector(users, x->ficheiro);

            for (i = 0; i < *users; i++) {
                responde = 0;
                if (strcmp("login", msg.p1) == 0) {
                    if (strcmp(msg.p2, u[i].nome) == 0) {
                        if (strcmp(msg.p3, u[i].pass) == 0) {
                            if (x->u[i].estado == 1) {
                                strcpy(resposta, "Utilizador já logado");
                                break;
                            }
                            strcpy(resposta, "Logado!");
                            x->u[i].estado = 1;
                            break;
                        } else {
                            strcpy(resposta, "Password errada!");
                            break;
                        }
                    } else
                        if (i == *users - 1) {
                        strcpy(resposta, "Nao registado!");
                    }
                } else
                    if (strcmp(msg.p1, "sair") == 0) {
                    if (strcmp(msg.p2, u[i].nome) == 0) {
                        responde = 1;
                        x->u[i].estado = 0;
                        unlink(msg.p2);
                        break;
                    }
                } else
                    if (strcmp(msg.p1, "jogar") == 0) {
                    if (strcmp(msg.p2, u[i].nome) == 0) {
                        responde = 1;
                        x->u[i].a_jogar = 1;
                        break;
                    }
                } else
                    if (strcmp(msg.p1, "entrar") == 0) {
                    if (strcmp(msg.p2, u[i].nome) == 0) {
                        strcpy(resposta, "Logado!");
                        if (*online >= 20) {
                            strcpy(resposta, "Jogo Cheio!");
                        }
                    }
                }
            }

            if (responde == 0) {
                mkfifo(msg.p2, 0666);
                fd2 = open(msg.p2, O_WRONLY);
                write(fd2, resposta, sizeof (resposta));
                close(fd2);
                if (strcmp(resposta, "Logado!") != 0 && strcmp(resposta, "Jogo Cheio!") != 0) {
                    unlink(msg.p2);
                }
            }
        }
    }

    close(fd);
    pthread_exit(0);
}

pontuser vec_jogadores(labirinto lab, pontuser u, int z, int *novo_jogador, int *online) {  //faz um vetor com a informação dos users que estão a jogar
    int i, j;

    if (u[z].altura != 0 || u[z].largura != 0) {
        return NULL;
    }

    for (i = 0; i < lab.altura; i++) {
        for (j = 0; j < lab.largura; j++) {
            if (lab.mapa[i][j] == ' ') {

                u[z].altura = i;
                u[z].largura = j;
                u[z].nbombinhas = 3;
                u[z].nbombas = 2;
                u[z].freeze = 0;
                u[z].coletor_aut = 0;
                i = lab.altura;
                break;
            }
        }
    }

    *online += 1;
    *novo_jogador = 1;
}

labirinto labirinto_1(labirinto lab, int *obj_lab, int n_inimigos) { //cria o labirinto básico
    int i, j, k;
    lab.largura = 30;
    lab.altura = 20;

    for (i = 0; i < lab.altura; i++) {
        for (j = 0; j < lab.largura; j++) {

            if (i == 0 || i == lab.altura - 1) {
                lab.mapa[i][j] = '#';
            } else
                if (j == 0 || j == lab.largura - 1) {
                lab.mapa[i][j] = '#';
            } else
                lab.mapa[i][j] = ' ';
        }
    }
    for (i = 2; i < lab.altura - 1; i += 3) {
        for (j = 2; j < lab.largura - 1; j += 6) {
            lab.mapa[i][j] = '#';
        }
    }

    for (i = 1; i < lab.altura - 1; i += 3) {
        for (j = 5; j < lab.largura - 1; j += 6) {
            lab.mapa[i][j] = '%';
        }
    }


    for (j = 5; j < lab.largura - 1; j += 6) {
        lab.mapa[6][j] = '*';
        lab.mapa[12][j] = '*';
    }

    for (i = 3; i < lab.altura - 1; i += 6) {
        lab.mapa[i][1] = 'I';
        lab.mapa[i][28] = 'I';
    }

    lab.mapa[18][2] = '%';
    lab.mapa[17][1] = '%';
    lab.mapa[18][1] = '*';
    lab.mapa[18][28] = 'O';

    lab.nParedesDestr = 26;
    lab.nParedes = 30;
    lab.nObjetos = *obj_lab;
    lab.passagem = 0;
    lab.nInimigos = n_inimigos;

    if (n_inimigos > 6) {
        for (k = 6; k < n_inimigos; k++) {
            for (i = lab.altura; i > 0; i--) {
                for (j = lab.largura; j > 0; j--) {
                    if (lab.mapa[i][j] == ' ') {

                        lab.mapa[i][j] = 'I';
                        i = 0;
                        break;
                    }
                }
            }
        }
    }

    int objetos_restantes = *obj_lab - 9;

    if (*obj_lab > 9) {
        for (k = 9; k<*obj_lab; k++) {
            for (i = lab.altura; i >= 0; i--) {
                for (j = 0; j < lab.largura; j++) {
                    if (lab.mapa[i][j] == ' ') {

                        lab.mapa[i][j] = '*';
                        i = -1;
                        objetos_restantes -= 1;
                        break;
                    }
                }
            }
        }
    }
    if (objetos_restantes != 0) {
        printf("sem espaco");
    }

    return lab;
}

void atualiza_mapa(pontuser u, labirinto lab, int online) { //sempre que há mudanças no mapa, esta função envia essas mudanças para todos os utilizadores
    int fd, k;
    char fifo_labirinto[29] = {0};

    for (k = 0; k <= online; k++) {
        strcpy(fifo_labirinto, "labirinto_");
        strcat(fifo_labirinto, u[k].nome);

        fd = open(fifo_labirinto, O_RDWR);
        if (fd != -1) {
            write(fd, &lab, sizeof (labirinto));
        }
        close(fd);
    }
}

void *rebenta_bombas(void* dados) { //Responsável por explodir as bombas, verificar se há paredes destrutíveis,                                    
    int i, j, k;                    //players ou inimigos para os eliminar. Por fim apaga a explosão.
    pthread *x = (pthread *) dados;

    pontuser u = x->u;
    int *online = x->online;

    int alt_bomba = x->alt_bomba;
    int lar_bomba = x->lar_bomba;
    int raio = x->raio_bomba;

    int morre = 0;
    int morre_alt;
    int morre_lar;

    int drop;

    char fifo_labirinto[29] = {0};

    sleep(2);
    for (i = 0; i < raio; i++) {
        if (x->lab.mapa[alt_bomba + i][lar_bomba] == '#' || x->lab.mapa[alt_bomba + i][lar_bomba] == '*' ||
                x->lab.mapa[alt_bomba + i][lar_bomba] == '+' || x->lab.mapa[alt_bomba + i][lar_bomba] == 'O') {
            break;
        }
        if (x->lab.mapa[alt_bomba + i][lar_bomba] == 'P') {
            morre = 1;
            morre_alt = alt_bomba + i;
            morre_lar = lar_bomba;
        }
        if (x->lab.mapa[alt_bomba + i][lar_bomba] == 'I') {
            for (j = 0; j < x->lab.nInimigos; j++) {
                if (x->ini[j].altura == alt_bomba + i && x->ini[j].largura == lar_bomba) {
                    drop = rand() % 100;                                            //probabilidades de haver o drop de um item.
                    if (drop >= 0 && drop < 20) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '1';
                    }
                    if (drop >= 20 && drop < 45) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '2';
                    }
                    if (drop >= 45 && drop < 60) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '3';
                    }
                    if (drop >= 60 && drop < 70) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '4';
                    }
                    if (drop >= 70 && drop < 90) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '5';
                    }
                    x->ini[j].altura = 0;
                    x->ini[j].largura = 0;
                }
            }
        }
        x->lab.mapa[alt_bomba + i][lar_bomba] = '$';
    }
    for (i = -1; i>-raio; i--) {
        if (x->lab.mapa[alt_bomba + i][lar_bomba] == '#' || x->lab.mapa[alt_bomba + i][lar_bomba] == '*' ||
                x->lab.mapa[alt_bomba + i][lar_bomba] == '+' || x->lab.mapa[alt_bomba + i][lar_bomba] == 'O') {
            break;
        }
        if (x->lab.mapa[alt_bomba + i][lar_bomba] == 'P') {
            morre = 1;
            morre_alt = alt_bomba + i;
            morre_lar = lar_bomba;
        }
        if (x->lab.mapa[alt_bomba + i][lar_bomba] == 'I') {
            for (j = 0; j < x->lab.nInimigos; j++) {
                if (x->ini[j].altura == alt_bomba + i && x->ini[j].largura == lar_bomba) {
                    drop = rand() % 100;
                    if (drop >= 0 && drop < 20) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '1';
                    }
                    if (drop >= 20 && drop < 45) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '2';
                    }
                    if (drop >= 45 && drop < 60) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '3';
                    }
                    if (drop >= 60 && drop < 70) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '4';
                    }
                    if (drop >= 70 && drop < 90) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '5';
                    }
                    x->ini[j].altura = 0;
                    x->ini[j].largura = 0;
                }
            }
        }
        x->lab.mapa[alt_bomba + i][lar_bomba] = '$';
    }
    for (i = 0; i < raio; i++) {
        if (x->lab.mapa[alt_bomba][lar_bomba + i] == '#' || x->lab.mapa[alt_bomba][lar_bomba + i] == '*' ||
                x->lab.mapa[alt_bomba][lar_bomba + i] == '+' || x->lab.mapa[alt_bomba][lar_bomba + i] == 'O') {
            break;
        }
        if (x->lab.mapa[alt_bomba][lar_bomba + i] == 'P') {
            morre = 1;
            morre_alt = alt_bomba;
            morre_lar = lar_bomba + i;
        }
        if (x->lab.mapa[alt_bomba][lar_bomba + i] == 'I') {
            for (j = 0; j < x->lab.nInimigos; j++) {
                if (x->ini[j].altura == alt_bomba && x->ini[j].largura == lar_bomba + i) {
                    drop = rand() % 100;
                    if (drop >= 0 && drop < 20) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '1';
                    }
                    if (drop >= 20 && drop < 45) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '2';
                    }
                    if (drop >= 45 && drop < 60) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '3';
                    }
                    if (drop >= 60 && drop < 70) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '4';
                    }
                    if (drop >= 70 && drop < 90) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '5';
                    }
                    x->ini[j].altura = 0;
                    x->ini[j].largura = 0;
                }
            }
        }
        x->lab.mapa[alt_bomba][lar_bomba + i] = '$';
    }
    for (i = -1; i>-raio; i--) {
        if (x->lab.mapa[alt_bomba][lar_bomba + i] == '#' || x->lab.mapa[alt_bomba][lar_bomba + i] == '*' ||
                x->lab.mapa[alt_bomba][lar_bomba + i] == '+' || x->lab.mapa[alt_bomba][lar_bomba + i] == 'O') {
            break;
        }
        if (x->lab.mapa[alt_bomba][lar_bomba + i] == 'P') {
            morre = 1;
            morre_alt = alt_bomba;
            morre_lar = lar_bomba + i;
        }
        if (x->lab.mapa[alt_bomba][lar_bomba + i] == 'I') {
            for (j = 0; j < x->lab.nInimigos; j++) {
                if (x->ini[j].altura == alt_bomba && x->ini[j].largura == lar_bomba + i) {
                    drop = rand() % 100;
                    if (drop >= 0 && drop < 20) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '1';
                    }
                    if (drop >= 20 && drop < 45) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '2';
                    }
                    if (drop >= 45 && drop < 60) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '3';
                    }
                    if (drop >= 60 && drop < 70) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '4';
                    }
                    if (drop >= 70 && drop < 90) {
                        x->lab.mapa[x->ini[k].altura][x->ini[k].largura + 1] = '5';
                    }
                    x->ini[j].altura = 0;
                    x->ini[j].largura = 0;
                }
            }
        }
        x->lab.mapa[alt_bomba][lar_bomba + i] = '$';
    }
    atualiza_mapa(u, x->lab, *online);
    sleep(1);

    if (morre = 1) {
        for (i = 0; i<*online; i++) {
            if (x->u[i].altura == morre_alt && x->u[i].largura == morre_lar) {
                x->u[i].pontos = 0;
                x->u[i].a_jogar = 0;
                *online -= 1;
                strcpy(fifo_labirinto, "labirinto_");
                strcat(fifo_labirinto, x->u[i].nome);
                unlink(fifo_labirinto);
            }
        }
    }

    //apaga explosão das bombas
    sleep(1);
    for (i = 0; i < raio; i++) {
        if (x->lab.mapa[alt_bomba + i][lar_bomba] == '$') {
            x->lab.mapa[alt_bomba + i][lar_bomba] = ' ';
        }
    }
    for (i = -1; i>-raio; i--) {
        if (x->lab.mapa[alt_bomba + i][lar_bomba] == '$') {
            x->lab.mapa[alt_bomba + i][lar_bomba] = ' ';
        }
    }
    for (i = 0; i < raio; i++) {
        if (x->lab.mapa[alt_bomba][lar_bomba + i] == '$') {
            x->lab.mapa[alt_bomba][lar_bomba + i] = ' ';
        }
    }
    for (i = -1; i>-raio; i--) {
        if (x->lab.mapa[alt_bomba][lar_bomba + i] == '$') {
            x->lab.mapa[alt_bomba][lar_bomba + i] = ' ';
        }
    }
    atualiza_mapa(u, x->lab, *online);

    return NULL;
}

void *processa_comandos_utilizador(void *dados) {
    int fd;
    int i, j, k;
    int permite_bomba;
    pthread *x = (pthread *) dados;
    comando com;

    int *users = x->users;
    pontuser u = x->u;
    int *online = x->online;
    labirinto lab = x->lab;

    int coletor_bombas;

    pthread_t bombas;

    char fifo_labirinto[29] = {0};

    mkfifo("fifo_comandos", 0666);
    fd = open("fifo_comandos", O_RDWR);

    while (1) {
        coletor_bombas = 5;
        permite_bomba = 0;
        if (x->lab.nObjetos == 0) { //verifica se todos os objetos foram apanhados para abilitar a saida
            x->lab.passagem = 1;
            x->lab.mapa[18][28] = '+';
        }

        for (k = 0; k<*online; k++) { //verifica se algum jogador apanhou o item de coletar 5 objetos
            if (x->u[k].coletor_aut == 1) {
                if (x->lab.nObjetos > 0) {
                    for (i = 0; i < x->lab.altura; i++) {
                        for (j = 0; j < x->lab.largura; j++) {
                            if (coletor_bombas >= 0 && x->lab.nObjetos > 0) {
                                if (x->lab.mapa[i][j] == '*') {
                                    x->lab.mapa[i][j] = ' ';
                                    x->lab.nObjetos -= 1;
                                    coletor_bombas -= 1;

                                    if (coletor_bombas == 0) {
                                        x->u[k].coletor_aut = 0;
                                        i = x->lab.altura;
                                        j = x->lab.largura;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        read(fd, &com, sizeof (com)); //lê todos os comandos introuduzidos por todos os users que estejam a jogar
        for (i = 0; i < *users; i++) {
            if (strcmp(u[i].nome, com.user) == 0) {
                if (com.tecla == 'q') {
                    u[i].a_jogar = 0;
                    *online = *online - 1;
                } else {
                    if (com.tecla == 'd') {
                        if (x->lab.mapa[u[i].altura][u[i].largura + 1] == ' ' ||
                                x->lab.mapa[u[i].altura][u[i].largura + 1] == '*' ||
                                x->lab.mapa[u[i].altura][u[i].largura + 1] == 'I' ||
                                x->lab.mapa[u[i].altura][u[i].largura + 1] == '$' ||
                                x->lab.mapa[u[i].altura][u[i].largura + 1] == '+' ||
                                x->lab.mapa[u[i].altura][u[i].largura + 1] == '1' ||
                                x->lab.mapa[u[i].altura][u[i].largura + 1] == '2' ||
                                x->lab.mapa[u[i].altura][u[i].largura + 1] == '3' ||
                                x->lab.mapa[u[i].altura][u[i].largura + 1] == '4' ||
                                x->lab.mapa[u[i].altura][u[i].largura + 1] == '5') {

                            if (x->lab.mapa[u[i].altura][u[i].largura + 1] == '1') {
                                x->u[i].nbombas += 1;
                            }
                            if (x->lab.mapa[u[i].altura][u[i].largura + 1] == '2') {
                                x->u[i].nbombinhas += 1;
                            }
                            if (x->lab.mapa[u[i].altura][u[i].largura + 1] == '3') {
                                x->u[i].coletor_aut = 1;
                            }
                            if (x->lab.mapa[u[i].altura][u[i].largura + 1] == '4') {
                                x->u[i].pontos += 30;
                            }
                            if (x->lab.mapa[u[i].altura][u[i].largura + 1] == '5') {
                                x->u[i].freeze = 1;
                            }
                            if (x->lab.mapa[u[i].altura][u[i].largura + 1] == '*') {
                                x->lab.nObjetos -= 1;
                                x->u[i].pontos += 10;
                            }
                            if (x->lab.mapa[u[i].altura][u[i].largura + 1] == 'I' || //se for um inimigo ou bomba, o utilizador morre
                                x->lab.mapa[u[i].altura][u[i].largura + 1] == '$') {
                                x->u[i].pontos = 0;
                                x->u[i].a_jogar = 0;
                                *online -= 1;
                                strcpy(fifo_labirinto, "labirinto_");
                                strcat(fifo_labirinto, x->u[i].nome);
                                unlink(fifo_labirinto);
                                break;
                            }
                            if (x->lab.mapa[u[i].altura][u[i].largura + 1] == '+') {
                                
                                shutdown(u, *users);
                            }
                            x->lab.mapa[u[i].altura][u[i].largura] = ' ';
                            x->lab.mapa[u[i].altura][u[i].largura + 1] = 'P';
                            x->u[i].largura++;
                            x->u[i].ultimo_com = 'd';

                            atualiza_mapa(u, x->lab, *online);
                        }
                    } else {
                        if (com.tecla == 's') {
                            if (x->lab.mapa[u[i].altura + 1][u[i].largura] == ' ' ||
                                    x->lab.mapa[u[i].altura + 1][u[i].largura] == '*' ||
                                    x->lab.mapa[u[i].altura + 1][u[i].largura] == 'I' ||
                                    x->lab.mapa[u[i].altura + 1][u[i].largura] == '$' ||
                                    x->lab.mapa[u[i].altura + 1][u[i].largura] == '1' ||
                                    x->lab.mapa[u[i].altura + 1][u[i].largura] == '2' ||
                                    x->lab.mapa[u[i].altura + 1][u[i].largura] == '3' ||
                                    x->lab.mapa[u[i].altura + 1][u[i].largura] == '4' ||
                                    x->lab.mapa[u[i].altura + 1][u[i].largura] == '5') {

                                if (x->lab.mapa[u[i].altura + 1][u[i].largura] == '1') {
                                    x->u[i].nbombas += 1;
                                }
                                if (x->lab.mapa[u[i].altura + 1][u[i].largura] == '2') {
                                    x->u[i].nbombinhas += 1;
                                }
                                if (x->lab.mapa[u[i].altura + 1][u[i].largura] == '3') {
                                    x->u[i].coletor_aut = 1;
                                }
                                if (x->lab.mapa[u[i].altura + 1][u[i].largura] == '4') {
                                    x->u[i].pontos += 30;
                                }
                                if (x->lab.mapa[u[i].altura + 1][u[i].largura] == '5') {
                                    x->u[i].freeze = 1;
                                }
                                if (x->lab.mapa[u[i].altura + 1][u[i].largura] == '*') {
                                    x->lab.nObjetos -= 1;
                                    x->u[i].pontos += 10;
                                }
                                if (x->lab.mapa[u[i].altura + 1][u[i].largura] == 'I' ||
                                        x->lab.mapa[u[i].altura + 1][u[i].largura] == '$') {
                                    x->u[i].pontos = 0;
                                    x->u[i].a_jogar = 0;
                                    *online -= 1;
                                    strcpy(fifo_labirinto, "labirinto_");
                                    strcat(fifo_labirinto, x->u[i].nome);
                                    unlink(fifo_labirinto);
                                    break;
                                }
                                if (x->lab.mapa[u[i].altura + 1][u[i].largura] == '+') {
                                    shutdown(u, *users);
                                }
                                x->lab.mapa[u[i].altura][u[i].largura] = ' ';
                                x->lab.mapa[u[i].altura + 1][u[i].largura] = 'P';
                                x->u[i].altura++;
                                x->u[i].ultimo_com = 's';

                                atualiza_mapa(u, x->lab, *online);
                            }
                        } else {
                            if (com.tecla == 'a') {
                                if (x->lab.mapa[u[i].altura][u[i].largura - 1] == ' ' ||
                                        x->lab.mapa[u[i].altura][u[i].largura - 1] == '*' ||
                                        x->lab.mapa[u[i].altura][u[i].largura - 1] == 'I' ||
                                        x->lab.mapa[u[i].altura][u[i].largura - 1] == '$' ||
                                        x->lab.mapa[u[i].altura][u[i].largura - 1] == '1' ||
                                        x->lab.mapa[u[i].altura][u[i].largura - 1] == '2' ||
                                        x->lab.mapa[u[i].altura][u[i].largura - 1] == '3' ||
                                        x->lab.mapa[u[i].altura][u[i].largura - 1] == '4' ||
                                        x->lab.mapa[u[i].altura][u[i].largura - 1] == '5') {

                                    if (x->lab.mapa[u[i].altura][u[i].largura - 1] == '1') {
                                        x->u[i].nbombas += 1;
                                    }
                                    if (x->lab.mapa[u[i].altura][u[i].largura - 1] == '2') {
                                        x->u[i].nbombinhas += 1;
                                    }
                                    if (x->lab.mapa[u[i].altura][u[i].largura - 1] == '3') {
                                        x->u[i].coletor_aut = 1;
                                    }
                                    if (x->lab.mapa[u[i].altura][u[i].largura - 1] == '4') {
                                        x->u[i].pontos += 30;
                                    }
                                    if (x->lab.mapa[u[i].altura][u[i].largura - 1] == '5') {
                                        x->u[i].freeze = 1;
                                    }
                                    if (x->lab.mapa[u[i].altura][u[i].largura - 1] == '*') {
                                        x->lab.nObjetos -= 1;
                                        x->u[i].pontos += 10;
                                    }
                                    if (x->lab.mapa[u[i].altura][u[i].largura - 1] == 'I' ||
                                            x->lab.mapa[u[i].altura][u[i].largura - 1] == '$') {
                                        x->u[i].pontos = 0;
                                        x->u[i].a_jogar = 0;
                                        *online -= 1;
                                        strcpy(fifo_labirinto, "labirinto_");
                                        strcat(fifo_labirinto, x->u[i].nome);
                                        unlink(fifo_labirinto);
                                        break;
                                    }
                                    if (x->lab.mapa[u[i].altura][u[i].largura - 1] == '+') {
                                        shutdown(u, *users);
                                    }
                                    x->lab.mapa[u[i].altura][u[i].largura] = ' ';
                                    x->lab.mapa[u[i].altura][u[i].largura - 1] = 'P';
                                    x->u[i].largura--;
                                    x->u[i].ultimo_com = 'a';

                                    atualiza_mapa(u, x->lab, *online);
                                }
                            } else {
                                if (com.tecla == 'w') {
                                    if (x->lab.mapa[u[i].altura - 1][u[i].largura] == ' ' ||
                                            x->lab.mapa[u[i].altura - 1][u[i].largura] == '*' ||
                                            x->lab.mapa[u[i].altura - 1][u[i].largura] == 'I' ||
                                            x->lab.mapa[u[i].altura - 1][u[i].largura] == '$' ||
                                            x->lab.mapa[u[i].altura - 1][u[i].largura] == '1' ||
                                            x->lab.mapa[u[i].altura - 1][u[i].largura] == '2' ||
                                            x->lab.mapa[u[i].altura - 1][u[i].largura] == '3' ||
                                            x->lab.mapa[u[i].altura - 1][u[i].largura] == '4' ||
                                            x->lab.mapa[u[i].altura - 1][u[i].largura] == '5') {

                                        if (x->lab.mapa[u[i].altura - 1][u[i].largura] == '1') {
                                            x->u[i].nbombas += 1;
                                        }
                                        if (x->lab.mapa[u[i].altura - 1][u[i].largura] == '2') {
                                            x->u[i].nbombinhas += 1;
                                        }
                                        if (x->lab.mapa[u[i].altura - 1][u[i].largura] == '3') {

                                        }
                                        if (x->lab.mapa[u[i].altura - 1][u[i].largura] == '4') {
                                            x->u[i].pontos += 30;
                                        }
                                        if (x->lab.mapa[u[i].altura - 1][u[i].largura] == '5') {
                                            x->u[i].freeze = 1;
                                        }
                                        if (x->lab.mapa[u[i].altura - 1][u[i].largura] == '*') {
                                            x->lab.nObjetos -= 1;
                                            x->u[i].pontos += 10;
                                        }
                                        if (x->lab.mapa[u[i].altura - 1][u[i].largura] == 'I' ||
                                                x->lab.mapa[u[i].altura - 1][u[i].largura] == '$') {
                                            x->u[i].pontos = 0;
                                            x->u[i].a_jogar = 0;
                                            *online -= 1;
                                            strcpy(fifo_labirinto, "labirinto_");
                                            strcat(fifo_labirinto, x->u[i].nome);
                                            unlink(fifo_labirinto);
                                            break;
                                        }
                                        if (x->lab.mapa[u[i].altura - 1][u[i].largura] == '+') {
                                            shutdown(u, *users);
                                        }
                                        x->lab.mapa[u[i].altura][u[i].largura] = ' ';
                                        x->lab.mapa[u[i].altura - 1][u[i].largura] = 'P';
                                        x->u[i].altura--;
                                        x->u[i].ultimo_com = 'w';

                                        atualiza_mapa(u, x->lab, *online);
                                    }

                                } else {
                                    if (com.tecla == 'b') {
                                        if (x->u[i].nbombinhas > 0) {//só funciona se o user tiver bombinhas no inventário
                                            
                                            if (x->u[i].ultimo_com == 'd') {
                                                if (x->lab.mapa[u[i].altura][u[i].largura + 1] == ' ') {
                                                    x->lab.mapa[u[i].altura][u[i].largura + 1] = 'b';
                                                    x->alt_bomba = u[i].altura;
                                                    x->lar_bomba = u[i].largura + 1;
                                                    permite_bomba = 1;
                                                }
                                            } else {
                                                if (x->u[i].ultimo_com == 'a') {
                                                    if (x->lab.mapa[u[i].altura][u[i].largura - 1] == ' ') {
                                                        x->lab.mapa[u[i].altura][u[i].largura - 1] = 'b';
                                                        x->alt_bomba = u[i].altura;
                                                        x->lar_bomba = u[i].largura - 1;
                                                        permite_bomba = 1;
                                                    }
                                                } else {
                                                    if (x->u[i].ultimo_com == 'w') {
                                                        if (x->lab.mapa[u[i].altura - 1][u[i].largura] == ' ') {
                                                            x->lab.mapa[u[i].altura - 1][u[i].largura] = 'b';
                                                            x->alt_bomba = u[i].altura - 1;
                                                            x->lar_bomba = u[i].largura;
                                                            permite_bomba = 1;
                                                        }
                                                    } else {
                                                        if (x->u[i].ultimo_com == 's') {
                                                            if (x->lab.mapa[u[i].altura + 1][u[i].largura] == ' ') {
                                                                x->lab.mapa[u[i].altura + 1][u[i].largura] = 'b';
                                                                x->alt_bomba = u[i].altura + 1;
                                                                x->lar_bomba = u[i].largura;
                                                                permite_bomba = 1;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            x->raio_bomba = 3;
                                            if(permite_bomba==1){
                                                x->u[i].nbombinhas -= 1;
                                                pthread_create(&bombas, NULL, &rebenta_bombas, (void *) x);
                                            }
                                            atualiza_mapa(u, x->lab, *online);
                                        }
                                    } else {
                                        if (com.tecla == 'n') {//só funciona se o user tiver bombas no inventário.
                                            if (x->u[i].nbombas > 0) {
                                                if (x->u[i].ultimo_com == 'd') {
                                                    if (x->lab.mapa[u[i].altura][u[i].largura + 1] == ' ') {
                                                        x->lab.mapa[u[i].altura][u[i].largura + 1] = 'B';
                                                        x->alt_bomba = u[i].altura;
                                                        x->lar_bomba = u[i].largura + 1;
                                                        permite_bomba = 1;
                                                    }
                                                } else {
                                                    if (x->u[i].ultimo_com == 'a') {
                                                        if (x->lab.mapa[u[i].altura][u[i].largura - 1] == ' ') {
                                                            x->lab.mapa[u[i].altura][u[i].largura - 1] = 'B';
                                                            x->alt_bomba = u[i].altura;
                                                            x->lar_bomba = u[i].largura - 1;
                                                            permite_bomba = 1;
                                                        }
                                                    } else {
                                                        if (x->u[i].ultimo_com == 'w') {
                                                            if (x->lab.mapa[u[i].altura - 1][u[i].largura] == ' ') {
                                                                x->lab.mapa[u[i].altura - 1][u[i].largura] = 'B';
                                                                x->alt_bomba = u[i].altura - 1;
                                                                x->lar_bomba = u[i].largura;
                                                                permite_bomba = 1;
                                                            }
                                                        } else {
                                                            if (x->u[i].ultimo_com == 's') {
                                                                if (x->lab.mapa[u[i].altura + 1][u[i].largura] == ' ') {
                                                                    x->lab.mapa[u[i].altura + 1][u[i].largura] = 'B';
                                                                    x->alt_bomba = u[i].altura + 1;
                                                                    x->lar_bomba = u[i].largura;
                                                                    permite_bomba = 1;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                x->raio_bomba = 5;
                                                if(permite_bomba==1){
                                                    x->u[i].nbombas -= 1;
                                                    pthread_create(&bombas, NULL, &rebenta_bombas, (void *) x);//faz as bombas/bombinhas explodirem
                                                }
                                                atualiza_mapa(u, x->lab, *online);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void *envia_labirinto(void *dados) {    //verifica em qual dos espaços do mapa é que um novo jogador iniciará
    int i;
    int novo_jogador = 0;
    char fifo_labirinto[29] = {0};

    pthread *x = (pthread*) dados;
    int *users = x->users;
    pontuser u = x->u;
    int *online = x->online;

    do {
        usleep(100000);
        for (i = 0; i < *users; i++) {
            if (u[i].a_jogar == 0) {
                if (u[i].altura != 0 || u[i].largura != 0) {
                    x->lab.mapa[u[i].altura][u[i].largura] = ' ';
                    u[i].altura = 0;
                    u[i].largura = 0;
                    atualiza_mapa(u, x->lab, *online);
                }
            }
        }
        for (i = 0; i < *users; i++) {
            if (u[i].a_jogar == 1) {
                novo_jogador = 0;

                vec_jogadores(x->lab, u, i, &novo_jogador, online);

                if (novo_jogador == 1) {

                    x->lab.mapa[u[i].altura][u[i].largura] = 'P';

                    atualiza_mapa(u, x->lab, *online);
                }
            }
        }
    } while (1);
}

void *movimento_inimigos(void *dados) {     //esta funcao é responsável por mover os inimigos pelo mapa, matando os players.
    int i, j, k = 0, l;
    int direcao;
    int drop;

    char fifo_labirinto[29] = {0};

    pthread *x = (pthread*) dados;
    int *online = x->online;
    int *users = x->users;
    pontuser u = x->u;
    pontInimigo ini = x->ini;

    for (i = 0; i < x->lab.altura; i++) { //dá coordenadas aos inimigos
        for (j = 0; j < x->lab.largura; j++) {
            if (x->lab.mapa[i][j] == 'I') {
                ini[k].altura = i;
                ini[k].largura = j;
                k++;
                if (k == x->lab.nInimigos) {
                    i = x->lab.altura;
                    break;
                }
            }
        }
    }

    srand(time(NULL));

    while (1) {
        atualiza_mapa(u, x->lab, *online); //verifica se algum user tem o drop especial de congelar os inimigos por 5 segundos
        for (i = 0; i<*online; i++) {
            if (x->u[i].freeze > 0) {
                x->u[i].freeze = 0;
                sleep(4);
                break;
            }
        }
        sleep(1);

        for (k = 0; k <= x->lab.nInimigos; k++) {//da um movimento aleatorio aos inimigos
            if (ini[k].altura != 0 || ini[k].largura != 0) {
                direcao = rand() % 4;
                if (direcao == 0) {
                    if (x->lab.mapa[ini[k].altura][ini[k].largura + 1] == ' ' ||
                            x->lab.mapa[ini[k].altura][ini[k].largura + 1] == 'P' ||
                            x->lab.mapa[ini[k].altura][ini[k].largura + 1] == '$') {

                        if (x->lab.mapa[ini[k].altura][ini[k].largura + 1] == ' ') {
                            x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                            x->lab.mapa[ini[k].altura][ini[k].largura + 1] = 'I';
                            ini[k].largura += 1;
                        }
                        if (x->lab.mapa[ini[k].altura][ini[k].largura + 1] == '$') {
                            x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                            drop = rand() % 100;
                            if (drop >= 0 && drop < 20) {                               //probabilidades de drop
                                x->lab.mapa[ini[k].altura][ini[k].largura + 1] = '1';
                            }
                            if (drop >= 20 && drop < 45) {
                                x->lab.mapa[ini[k].altura][ini[k].largura + 1] = '2';
                            }
                            if (drop >= 45 && drop < 60) {
                                x->lab.mapa[ini[k].altura][ini[k].largura + 1] = '3';
                            }
                            if (drop >= 60 && drop < 70) {
                                x->lab.mapa[ini[k].altura][ini[k].largura + 1] = '4';
                            }
                            if (drop >= 70 && drop < 90) {
                                x->lab.mapa[ini[k].altura][ini[k].largura + 1] = '5';
                            }
                            ini[k].altura = 0;
                            ini[k].largura = 0;
                        }
                        if (x->lab.mapa[ini[k].altura][ini[k].largura + 1] == 'P') {

                            x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                            x->lab.mapa[ini[k].altura][ini[k].largura + 1] = 'I';
                            ini[k].largura += 1;
                            for (l = 0; l<*online; l++) {
                                if (x->u[l].altura == ini[k].altura && x->u[l].largura == ini[k].largura + 1) {
                                    strcpy(fifo_labirinto, "labirinto_");
                                    strcat(fifo_labirinto, x->u[l].nome);
                                    unlink(fifo_labirinto);
                                    x->u[l].pontos = 0;
                                    x->u[l].a_jogar = 0;
                                    *online -= 1;
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    if (direcao == 1) {
                        if (x->lab.mapa[ini[k].altura][ini[k].largura - 1] == ' ' ||
                                x->lab.mapa[ini[k].altura][ini[k].largura - 1] == 'P' ||
                                x->lab.mapa[ini[k].altura][ini[k].largura - 1] == '$') {
                            if (x->lab.mapa[ini[k].altura][ini[k].largura - 1] = ' ') {
                                x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                                x->lab.mapa[ini[k].altura][ini[k].largura - 1] = 'I';
                                ini[k].largura -= 1;
                            }
                            if (x->lab.mapa[ini[k].altura][ini[k].largura - 1] == '$') {
                                x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                                drop = rand() % 100;
                                if (drop >= 0 && drop < 20) {
                                    x->lab.mapa[ini[k].altura][ini[k].largura - 1] = '1';
                                }
                                if (drop >= 20 && drop < 45) {
                                    x->lab.mapa[ini[k].altura][ini[k].largura - 1] = '2';
                                }
                                if (drop >= 45 && drop < 60) {
                                    x->lab.mapa[ini[k].altura][ini[k].largura - 1] = '3';
                                }
                                if (drop >= 60 && drop < 70) {
                                    x->lab.mapa[ini[k].altura][ini[k].largura - 1] = '4';
                                }
                                if (drop >= 70 && drop < 90) {
                                    x->lab.mapa[ini[k].altura][ini[k].largura - 1] = '5';
                                }
                                ini[k].altura = 0;
                                ini[k].largura = 0;
                            }
                            if (x->lab.mapa[ini[k].altura][ini[k].largura - 1] == 'P') {

                                x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                                x->lab.mapa[ini[k].altura][ini[k].largura - 1] = 'I';
                                ini[k].largura -= 1;

                                for (l = 0; l<*online; l++) {
                                    if (x->u[l].altura == ini[k].altura && x->u[l].largura == ini[k].largura - 1) {
                                        strcpy(fifo_labirinto, "labirinto_");
                                        strcat(fifo_labirinto, x->u[l].nome);
                                        unlink(fifo_labirinto);
                                        x->u[l].pontos = 0;
                                        x->u[l].a_jogar = 0;
                                        *online -= 1;
                                        break;
                                    }
                                }
                            }
                        }
                    } else {
                        if (direcao == 2) {
                            if (x->lab.mapa[ini[k].altura + 1][ini[k].largura] == ' ' ||
                                    x->lab.mapa[ini[k].altura + 1][ini[k].largura] == 'P' ||
                                    x->lab.mapa[ini[k].altura + 1][ini[k].largura] == '$') {
                                if (x->lab.mapa[ini[k].altura + 1][ini[k].largura] = ' ') {
                                    x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                                    x->lab.mapa[ini[k].altura + 1][ini[k].largura] = 'I';
                                    ini[k].altura += 1;
                                }
                                if (x->lab.mapa[ini[k].altura + 1][ini[k].largura] == '$') {
                                    x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                                    drop = rand() % 100;
                                    if (drop >= 0 && drop < 20) {
                                        x->lab.mapa[ini[k].altura + 1][ini[k].largura] = '1';
                                    }
                                    if (drop >= 20 && drop < 45) {
                                        x->lab.mapa[ini[k].altura + 1][ini[k].largura] = '2';
                                    }
                                    if (drop >= 45 && drop < 60) {
                                        x->lab.mapa[ini[k].altura + 1][ini[k].largura] = '3';
                                    }
                                    if (drop >= 60 && drop < 70) {
                                        x->lab.mapa[ini[k].altura + 1][ini[k].largura] = '4';
                                    }
                                    if (drop >= 70 && drop < 90) {
                                        x->lab.mapa[ini[k].altura + 1][ini[k].largura] = '5';
                                    }
                                    ini[k].altura = 0;
                                    ini[k].largura = 0;
                                }
                                if (x->lab.mapa[ini[k].altura + 1][ini[k].largura] == 'P') {
                                    x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                                    x->lab.mapa[ini[k].altura + 1][ini[k].largura] = 'I';
                                    ini[k].altura += 1;
                                    for (l = 0; l<*online; l++) {
                                        if (x->u[l].altura == ini[k].altura + 1 && x->u[l].largura == ini[k].largura) {
                                            strcpy(fifo_labirinto, "labirinto_");
                                            strcat(fifo_labirinto, x->u[l].nome);
                                            unlink(fifo_labirinto);
                                            x->u[l].pontos = 0;
                                            x->u[l].a_jogar = 0;
                                            *online -= 1;
                                            break;
                                            printf("nome:%s\n", x->u[l].nome);
                                            fflush(stdout);
                                        }
                                    }
                                }
                            }
                        } else {
                            if (direcao == 3) {
                                if (x->lab.mapa[ini[k].altura - 1][ini[k].largura] == ' ' ||
                                        x->lab.mapa[ini[k].altura - 1][ini[k].largura] == 'P' ||
                                        x->lab.mapa[ini[k].altura - 1][ini[k].largura] == '$') {
                                    if (x->lab.mapa[ini[k].altura - 1][ini[k].largura] = ' ') {
                                        x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                                        x->lab.mapa[ini[k].altura - 1][ini[k].largura] = 'I';
                                        ini[k].altura -= 1;
                                    }
                                    if (x->lab.mapa[ini[k].altura - 1][ini[k].largura] == '$') {
                                        x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                                        drop = rand() % 100;
                                        if (drop >= 0 && drop < 20) {
                                            x->lab.mapa[ini[k].altura - 1][ini[k].largura] = '1';
                                        }
                                        if (drop >= 20 && drop < 45) {
                                            x->lab.mapa[ini[k].altura - 1][ini[k].largura] = '2';
                                        }
                                        if (drop >= 45 && drop < 60) {
                                            x->lab.mapa[ini[k].altura - 1][ini[k].largura] = '3';
                                        }
                                        if (drop >= 60 && drop < 70) {
                                            x->lab.mapa[ini[k].altura - 1][ini[k].largura] = '4';
                                        }
                                        if (drop >= 70 && drop < 90) {
                                            x->lab.mapa[ini[k].altura - 1][ini[k].largura] = '5';
                                        }
                                        ini[k].altura = 0;
                                        ini[k].largura = 0;
                                    }
                                    if (x->lab.mapa[ini[k].altura - 1][ini[k].largura] == 'P') {
                                        x->lab.mapa[ini[k].altura][ini[k].largura] = ' ';
                                        x->lab.mapa[ini[k].altura - 1][ini[k].largura] = 'I';
                                        ini[k].altura -= 1;
                                        for (l = 0; l<*online; l++) {
                                            if (x->u[l].altura == ini[k].altura - 1 && x->u[l].largura == ini[k].largura) {
                                                strcpy(fifo_labirinto, "labirinto_");
                                                strcat(fifo_labirinto, x->u[l].nome);
                                                unlink(fifo_labirinto);
                                                x->u[l].pontos = 0;
                                                x->u[l].a_jogar = 0;
                                                *online -= 1;
                                                printf("nome:%s\n", x->u[l].nome);
                                                fflush(stdout);
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char** argv) {

    if(argc!=2){//verifica se o utilizador inicializou o servidor com o nome do ficheiro de utilizadores registados
        printf("Insira o nome do ficheiro de texto corretamente.\n");
        exit(0);
    }
    if (access("fifologin", F_OK) != -1) {
        printf("Um servidor ja esta ativo!\nA encerrar...\n");
        exit(0);
    }
    mkfifo("fifologin", 0666);

    labirinto lab;
    int objetos_lab = 9;
    int n_inimigos = 6;
    pontInimigo ini;
    ini = malloc(sizeof (inimigos) * n_inimigos);
    lab = labirinto_1(lab, &objetos_lab, n_inimigos);

    pontuser u;
    int i;
    int online = 0;
    int users = 0;
    int processo;
    char ficheiro[20];
    char comando[50], cmd[50];
    int sair = 0;

    signal(SIGUSR1, trata);

    pthread_t recebe;
    pthread_t envia_lab;
    pthread_t comandos;
    pthread_t inimigos;

    strcpy(ficheiro, argv[1]);
    u = ler_vector(&users, ficheiro);

    pthread *x = (pthread *) malloc(sizeof (pthread));  //cria estrutura com as informações que as thread necessitam para fazer as suas funções
    x->u = u;
    x->online = &online;
    x->users = &users;
    x->lab = lab;
    x->ini = ini;
    strcpy(x->ficheiro, ficheiro);

    pthread_create(&recebe, NULL, &login, (void *) x);  //inicializa as threads
    pthread_create(&envia_lab, NULL, &envia_labirinto, (void *) x);
    pthread_create(&comandos, NULL, &processa_comandos_utilizador, (void *) x);
    pthread_create(&inimigos, NULL, &movimento_inimigos, (void *) x);

    printf("\nComando\n");

    while (1) {
        printf("\n>");

        scanf("%25[^\n]s", comando);
        setbuf(stdin, NULL);
        strcpy(cmd, comando);
        processo = processa_comando(comando);

        switch (processo) {
            case 1: u = adiciona(u, &users, ficheiro, cmd);
                break;
            case 2: users_ativos(u, users);
                break;
            case 3: u = kick_username(u, users, cmd);
                break;
            case 4: game(x->u, users, online, x->lab);
                break;
            case 5:
                shutdown(u, users);
                sair = 1;
                break;
        }
    }
}
