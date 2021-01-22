#include "header.h"

void *mostra_ecra(void *nome) { //mostra o ecra de jogo

    int i, j, auxLinhas, auxColunas, fd, maxX, maxY, meioTamanhoString, distancia;
    labirinto lab;
    char fifo_labirinto[29] = {0};

    strcpy(fifo_labirinto, "labirinto_");
    strcat(fifo_labirinto, nome);

    fflush(stdout);
    while (1) {

        fd = open(fifo_labirinto, O_RDONLY);

        read(fd, &lab, sizeof (labirinto));
        clear();
        getmaxyx(stdscr, maxX, maxY);

        system("printf '\e[8;25;90t'");

        //PROCURA NO TERMINAL O SITIO PARA O MAPA FICAR CENTRADO
        auxLinhas = maxX / 2 - (lab.altura / 2);
        auxColunas = (maxY / 2 - (lab.largura / 2));

        meioTamanhoString = strlen("Prime 'q' para sair\n") / 2;
        init_pair(6, COLOR_RED, COLOR_BLACK);
        attron(COLOR_PAIR(6));
        mvprintw(0, auxColunas / 2 - meioTamanhoString, "Prime 'q' para sair\n");
        attroff(COLOR_PAIR(6));

        meioTamanhoString = strlen("BOMBERMAN") / 2;
        init_pair(10, COLOR_RED, COLOR_BLACK);
        attron(COLOR_PAIR(10));
        mvprintw(1, maxY / 2 - meioTamanhoString, "BOMBERMAN");
        attroff(COLOR_PAIR(10));

        init_pair(7, COLOR_YELLOW, COLOR_BLACK);
        attron(COLOR_PAIR(7));
        meioTamanhoString = strlen("A/W/S/D - Mover\n") / 2;
        mvprintw(maxX / 2 - 1, auxColunas / 2 - meioTamanhoString, "A/W/S/D - Mover\n");

        meioTamanhoString = strlen("Bombinha - b\n") / 2;
        mvprintw(maxX / 2, auxColunas / 2 - meioTamanhoString, "Bombinha - b\n");

        meioTamanhoString = strlen("Mega Bomba - n\n") / 2;
        mvprintw(maxX / 2 + 1, auxColunas / 2 - meioTamanhoString, "Mega Bomba - n\n");

        distancia = 3 * (auxColunas / 2) + lab.largura; // MEIO DA DISTANCIA DO LABIRINTO DO LADO DIREITO E DO TERMINAL(OPOSTO DO auxLinhas)
        attroff(COLOR_PAIR(7));

        fflush(stdout);

        start_color();

        for (i = 0; i < lab.altura; i++) {
            if (maxX != 25 && maxY != 80) {
                system("printf '\e[8;25;90t'");
            }
            move(auxLinhas++, auxColunas);
            for (j = 0; j < lab.largura; j++) {
                if (lab.mapa[i][j] == '#') {
                    init_pair(1, COLOR_GREEN, COLOR_GREEN);
                    attron(COLOR_PAIR(1));
                    printw("%c", lab.mapa[i][j]);
                    attroff(COLOR_PAIR(1));
                    fflush(stdout);

                } else if (lab.mapa[i][j] == 'I') {
                    init_pair(2, COLOR_RED, COLOR_BLACK);
                    attron(COLOR_PAIR(2));
                    printw("%c", lab.mapa[i][j]);
                    fflush(stdout);
                    attroff(COLOR_PAIR(1));
                } else if (lab.mapa[i][j] == '%') {
                    init_pair(3, COLOR_YELLOW, COLOR_YELLOW);
                    attron(COLOR_PAIR(3));
                    printw("%c", lab.mapa[i][j]);
                    fflush(stdout);
                    attroff(COLOR_PAIR(1));
                } else if (lab.mapa[i][j] == '*') {
                    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
                    attron(COLOR_PAIR(4));
                    printw("%c", lab.mapa[i][j]);
                    fflush(stdout);
                    attroff(COLOR_PAIR(1));
                } else if (lab.mapa[i][j] == 'P') {
                    init_pair(5, COLOR_BLUE, COLOR_BLACK);
                    attron(COLOR_PAIR(5));
                    printw("%c", lab.mapa[i][j]);
                    fflush(stdout);
                    attroff(COLOR_PAIR(5));
                } else if (lab.mapa[i][j] == 'O') {
                    init_pair(9, COLOR_WHITE, COLOR_BLACK);
                    attron(COLOR_PAIR(9));
                    printw("%c", lab.mapa[i][j]);
                    fflush(stdout);
                    attroff(COLOR_PAIR(9));
                } else if (lab.mapa[i][j] == '+') {
                    init_pair(8, COLOR_CYAN, COLOR_BLACK);
                    attron(COLOR_PAIR(8));
                    printw("%c", lab.mapa[i][j]);
                    fflush(stdout);
                    attroff(COLOR_PAIR(8));
                } else {
                    printw("%c", lab.mapa[i][j]);
                    fflush(stdout);
                }
            }
            printw("\n");
            fflush(stdout);
        }
        attron(COLOR_PAIR(7));
        meioTamanhoString = strlen("Drops:") / 2;
        mvprintw(maxX / 2 - 3, distancia - meioTamanhoString, "Drops:");

        meioTamanhoString = strlen("1- Mega Bomba") / 2;
        mvprintw(maxX / 2 + -1, distancia - meioTamanhoString, "1- Mega Bomba");

        meioTamanhoString = strlen("2- Bombinha") / 2;
        mvprintw(maxX / 2, distancia - meioTamanhoString, "2- Bombinha");

        meioTamanhoString = strlen("3 - Coletor Automatico") / 2;
        mvprintw(maxX / 2 + 1, distancia - meioTamanhoString, "3 - Coletor Automatico");

        meioTamanhoString = strlen("4 - Pontos Extra") / 2;
        mvprintw(maxX / 2 + 2, distancia - meioTamanhoString, "4 - Pontos Extra");

        meioTamanhoString = strlen("5-Freeze Inimigos") / 2;
        mvprintw(maxX / 2 + 3, distancia - meioTamanhoString, "5-Freeze Inimigos");
        attroff(COLOR_PAIR(7));
        move(maxX - 2, 0);

        refresh();
        close(fd);

        noecho();
    }
}

void jogo(char *nome, char *movimento, int *sair) {//pede comandos ao utilizador, e envia-os para processamento ao servidor

    int fd;
    char fifo_labirinto[29] = {0};
    int temp = 0;
    mensagem msg;
    comando com;

    strcpy(msg.p1, "jogar");
    strcpy(msg.p2, nome);

    fd = open("fifologin", O_WRONLY);
    write(fd, &msg, sizeof (msg));
    close(fd);

    pthread_t ecra_jogo;

    strcpy(fifo_labirinto, "labirinto_");
    strcat(fifo_labirinto, nome);

    mkfifo(fifo_labirinto, 0666);

    strcpy(com.user, nome);

    initscr();

    pthread_create(&ecra_jogo, NULL, &mostra_ecra, (void *) nome);

    do {
        if (*sair == 1) {
            break;
        } else {
            *movimento = getch();
        }
        if (*sair == 1) {
            break;
        }
        if (*movimento == 'q' || *movimento == 'a' || *movimento == 's' || *movimento == 'd' || *movimento == 'w' ||
                *movimento == 'b' || *movimento == 'n');
        {

            com.tecla = *movimento;

            fd = open("fifo_comandos", O_WRONLY);
            write(fd, &com, sizeof (com));
            close(fd);
        }
    } while (*movimento != 'q');
    unlink(fifo_labirinto);
    endwin();
}

void sair(char *nome) { //pede ao servidor para sair do mesmo

    mensagem msg;
    int fd;

    strcpy(msg.p1, "sair");
    strcpy(msg.p2, nome);

    fd = open("fifologin", O_WRONLY);

    write(fd, &msg, sizeof (msg));
    close(fd);

    exit(0);
}

void *verifica_online(void *nome) {//verifica se o servidor foi desligado ou se foi expulso isto verifica-se porque o fifo com o nome do utilizador será eliminado

    while (1) {
        sleep(1);
        if (access(nome, F_OK) == -1) {

            endwin();
            printf("Servidor: Encerrou ou foi expulso\n");
            fflush(stdout);
            exit(0);
        }
    }
}

int verifica_n_jogadores(char *nome) {//pergunta ao servidor se existem lugares "vagos" no jogo.
    int fd, fd2;
    char resposta[200];
    mensagem msg;

    strcpy(msg.p1, "entrar");
    strcpy(msg.p2, nome);

    fd = open("fifologin", O_WRONLY);
    write(fd, &msg, sizeof (msg));
    close(fd);
    fd2 = open(msg.p2, O_RDONLY);
    read(fd2, resposta, sizeof (resposta));
    close(fd2);

    if (strcmp(resposta, "Jogo Cheio!") == 0) {
        printf("Servidor: %s\n", resposta);
        return 0;
    } else {

        return 1;
    }
}

void *verifica_Morte(void* dados) {//verifica se o jogador foi eliminado por uma bomba ou inimigo, isto verifica-se porque o fifo "labirinto_'nome'" será eliminado
    verifica *x = (verifica *) dados;

    char fifo_labirinto[29] = {0};
    strcpy(fifo_labirinto, "labirinto_");
    strcat(fifo_labirinto, x->user);

    char *movimento = x->tecla;
    while (1) {
        sleep(1);
        if (strcmp(movimento, "q") != 0) {
            if (access(fifo_labirinto, F_OK) == -1) {
                printf("\nMorreste!!!\t");
                fflush(stdout);
                printf("Prime qualquer tecla para sair");
                fflush(stdout);
                *x->sair = 1;
                fflush(stdout);

                break;
            }
        }
    }
    pthread_exit(NULL);
}

void ecra(char *nome) {
    int operacao;
    int pode_entrar;
    int sai = 0;
    char movimento;

    pthread_t verifica_aJogar;
    verifica *x = (verifica *) malloc(sizeof (verifica));
    strcpy(x->user, nome);
    x->tecla = &movimento;
    x->sair = &sai;

    while (1) {

        do {

            system("clear");
            printf("    ____                  __                                  \n");
            fflush(stdout);
            printf("   / __ )____  ____ ___  / /_  ___  _________ ___  ____ _____ \n");
            fflush(stdout);
            printf("  / __  / __ \\/ __ `__ \\/ __ \\/ _ \\/ ___/ __ `__ \\/ __ `/ __ \\\n");
            fflush(stdout);
            printf(" / /_/ / /_/ / / / / / / /_/ /  __/ /  / / / / / / /_/ / / / /\n");
            fflush(stdout);
            printf("/_____/\\____/_/ /_/ /_/_.___/\\___/_/  /_/ /_/ /_/\\__,_/_/ /_/ \n");
            fflush(stdout);

            printf("\n1. jogo");
            fflush(stdout);
            printf("\n2. sair");
            fflush(stdout);
            printf("\n>");
            fflush(stdout);
            scanf("%d", &operacao);
            setbuf(stdin, NULL);

            switch (operacao) {
                case 1: sai = 0;
                    pode_entrar = verifica_n_jogadores(nome);
                    if (pode_entrar == 1) {
                        pthread_create(&verifica_aJogar, NULL, &verifica_Morte, (void *) x);
                        jogo(nome, &movimento, &sai);
                        if (pthread_join(verifica_aJogar, NULL) != 0) {
                            printf("fds");
                        }
                    }
                    break;

                case 2: sair(nome);
            }
        } while (operacao != 1 || operacao != 2);
    }
}

int main() { //pede as credencias do utilizador, envia-as ao servidor, e caso estejam corretas, inicializa o jogo
    char nome[TAM];
    char pass[TAM];
    int fd;
    int fd1;
    int fd2;
    mensagem msg;
    char resposta[200];
    pthread_t verifica_on;

    do {
        do {
            printf("Nome de utilizador:");
            scanf("%19s", nome);

            if (access(nome, F_OK) != -1) {
                printf("O utilizador %s ja esta online\n\n", nome);
            }
        } while (access(nome, F_OK) != -1);
        fd2 = open(nome, O_WRONLY);
        printf("Password: ");
        scanf("%19s", pass);


        /*escreve para a estrutura*/
        strcpy(msg.p1, "login");
        strcpy(msg.p2, nome);
        strcpy(msg.p3, pass);

        /*cria, abre e envia a estrutura mensagem*/

        fd = open("fifologin", O_WRONLY);
        if (fd == -1) {
            printf("Servidor offline\n");
            fflush(stdout);
            return 0;
        }
        write(fd, &msg, sizeof (msg));
        close(fd);

        /*abre o fifo da resposta*/

        do {
            fd = open(msg.p2, O_RDONLY);
        } while (fd == -1);

        read(fd, resposta, sizeof (resposta));
        printf("Servidor: %s\n", resposta);
        close(fd);

    } while (strcmp(resposta, "Logado!") != 0);

    pthread_create(&verifica_on, NULL, &verifica_online, (void *) nome);

    ecra(nome);

    return 0;
}