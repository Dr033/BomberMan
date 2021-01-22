#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <ctype.h>
#include <pthread.h>
#include <ncurses.h>
#define TAM 20

typedef struct USER 
{
    char nome[TAM];
    char pass[TAM];
    int estado;
    int a_jogar;
    int pontos;
    int altura;
    int largura;
    char ultimo_com;
    int nbombas;
    int nbombinhas;
    int coletor_aut;
    int freeze;
}user, *pontuser;

typedef struct Mensagem
{
    char p1[TAM], p2[TAM], p3[TAM];
}mensagem;

typedef struct Comando
{
    char user[TAM];
    char tecla;
}comando;

typedef struct Verifica
{
    char user[TAM];
    char *tecla;
    int *sair;
}verifica;

typedef struct Labirinto
{
    char mapa[25][50];
    int nInimigos;
    int nObjetos;
    int nParedesDestr;
    int nParedes;
    int altura;
    int largura;
    int passagem;
	
}labirinto;

typedef struct Inimigos {
    int altura;
    int largura;
} inimigos, *pontInimigo;

typedef struct pThread 
{
    int *users;
    char ficheiro[TAM];
    pontuser u;
    int *online;
    labirinto lab;
    int lar_bomba;
    int alt_bomba;
    int raio_bomba;
    pontInimigo ini;
}pthread;