#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

// Definindo cores para o terminal
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

#define MAX_TENTATIVAS 8
#define TAM_PALAVRA 20
#define MAX_JOGADORES 10

typedef struct {
    char nome[50];
    int pontuacao;
    double tempo;
    int nivel;
    int palavras_acertadas;
} Jogador;
// Estrutura para armazenar os dados dos jogadores
Jogador ranking[MAX_JOGADORES];
int num_jogadores = 0;

// Função para desenhar a forca de acordo com os erros
void desenhaForca(int erros) {
    printf(RED"  _______     \n");
    printf(" |/      |    \n");
    
    if(erros >= 1) printf(" |      (_)   \n");
    else printf(" |            \n");
    
    if(erros >= 4) printf(" |      /|\\   \n");
    else if(erros >= 3) printf(" |      /|    \n");
    else if(erros >= 2) printf(" |       |    \n");
    else printf(" |            \n");
    
    if(erros >= 5) printf(" |       |    \n");
    else printf(" |            \n");
    
    if(erros >= 6) printf(" |      / \\   \n");
    else printf(" |            \n");
    
    printf(" |            \n");
    printf("|__         \n"RESET);
}

// Carrega o ranking do arquivo
void carregarRanking() {
    FILE *arquivo = fopen("ranking.dat", "rb");
    if (arquivo != NULL) {
        fread(&num_jogadores, sizeof(int), 1, arquivo); 
        fread(ranking, sizeof(Jogador), num_jogadores, arquivo);
        fclose(arquivo);
    }
}

// Salva o ranking no arquivo
void salvarRanking() {
    FILE *arquivo = fopen("ranking.dat", "wb");
    if (arquivo != NULL) {
        fwrite(&num_jogadores, sizeof(int), 1, arquivo);
        fwrite(ranking, sizeof(Jogador), num_jogadores, arquivo);
        fclose(arquivo);
    }
}

// Mostra o ranking na tela
void mostrarRanking() {
    printf(RED"\n================ RANKING ================\n");
    printf(YELLOW"Pos. Nome       Nivel   Pontos   Tempo   Acertos\n");
    for (int i = 0; i < num_jogadores; i++) {
        const char* nivel_str;
        switch(ranking[i].nivel) {
            case 1: nivel_str = "Facil"; break;
            case 2: nivel_str = "Medio"; break;
            case 3: nivel_str = "Dificil"; break;
            default: nivel_str = "N/A"; break;
        }
        printf("%2d. %-12s% -7s%6d  %6.1fs   %5d\n", 
               i+1, ranking[i].nome, nivel_str, ranking[i].pontuacao, 
               ranking[i].tempo, ranking[i].palavras_acertadas);
    }
    printf("\n");
}

// Adiciona um jogador ao ranking
void adicionarAoRanking(char *nome, int pontuacao, double tempo, int nivel, int palavras_acertadas) {
    if (num_jogadores < MAX_JOGADORES) {
        strcpy(ranking[num_jogadores].nome, nome);
        ranking[num_jogadores].pontuacao = pontuacao;
        ranking[num_jogadores].tempo = tempo;
        ranking[num_jogadores].nivel = nivel;
        ranking[num_jogadores].palavras_acertadas = palavras_acertadas;
        num_jogadores++;
    } else {
        // Substitui o pior colocado se necessário
        int menor_pontuacao = ranking[0].pontuacao;
        int pos_menor = 0;
        
        for (int i = 1; i < num_jogadores; i++) {
            if (ranking[i].pontuacao < menor_pontuacao || 
                (ranking[i].pontuacao == menor_pontuacao && ranking[i].tempo > tempo)) {
                menor_pontuacao = ranking[i].pontuacao;
                pos_menor = i;
            }
        }
        
        if (pontuacao > menor_pontuacao || 
            (pontuacao == menor_pontuacao && tempo < ranking[pos_menor].tempo)) {
            strcpy(ranking[pos_menor].nome, nome);
            ranking[pos_menor].pontuacao = pontuacao;
            ranking[pos_menor].tempo = tempo;
            ranking[pos_menor].nivel = nivel;
            ranking[pos_menor].palavras_acertadas = palavras_acertadas;
        }
    }
    
    // Ordena o ranking
    for (int i = 0; i < num_jogadores - 1; i++) {
        for (int j = i + 1; j < num_jogadores; j++) {
            if (ranking[j].pontuacao > ranking[i].pontuacao || 
                (ranking[j].pontuacao == ranking[i].pontuacao && ranking[j].tempo < ranking[i].tempo)) {
                Jogador temp = ranking[i];
                ranking[i] = ranking[j];
                ranking[j] = temp;
            }
        }
    }
    
    // Mantém apenas os melhores jogadores
    if (num_jogadores > MAX_JOGADORES) {
        num_jogadores = MAX_JOGADORES;
    }
}

// Função principal do jogo
void jogarForca(int nivel, char *nome_jogador) {
    char palavras[200][TAM_PALAVRA];
    int num_palavras = 0;
    const char *nome_arquivo;
    
    // Configurações por nível
    switch(nivel) {
        case 1: nome_arquivo = "Facil.txt"; break;
        case 2: nome_arquivo = "Medio.txt"; break;
        case 3: nome_arquivo = "Dificil.txt"; break;
        default: printf(RED"Nivel invalido!\n"RESET); return;
    }
    
    // Carregar palavras do arquivo
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        printf(RED"Erro ao abrir o arquivo de palavras!\n"RESET);
        return;
    }
    
    // Ler palavras do arquivo e armazenar em um array
    while (fscanf(arquivo, "%19s", palavras[num_palavras]) == 1 && num_palavras < 200) {
        for (int i = 0; palavras[num_palavras][i]; i++) {
            palavras[num_palavras][i] = tolower(palavras[num_palavras][i]);
            if (palavras[num_palavras][i] == '\n' || palavras[num_palavras][i] == '\r') {
                palavras[num_palavras][i] = '\0';
            }
        }
        num_palavras++;
    }
    fclose(arquivo);
    
    if (num_palavras == 0) {
        printf(RED"Nenhuma palavra encontrada no arquivo!\n"RESET);
        return;
    }
    
    int pontuacao_total = 0;
    int palavras_acertadas = 0;
    double tempo_total = 0;
    int continuar_jogando = 1;
    int sair_por_escolha = 0;
    
    while(continuar_jogando && num_palavras > 0) {
        // Selecionar palavra aleatória e removê-la da lista
        srand(time(0));
        int indice = rand() % num_palavras;
        char palavra_secreta[TAM_PALAVRA];
        strcpy(palavra_secreta, palavras[indice]);
        
        // Remove a palavra selecionada
        for(int i = indice; i < num_palavras - 1; i++) {
            strcpy(palavras[i], palavras[i+1]);
        }
        num_palavras--;
        
        int tamanho_palavra = strlen(palavra_secreta);
        char palavra_adivinhada[TAM_PALAVRA];
        
        for(int i = 0; i < tamanho_palavra; i++) {
            palavra_adivinhada[i] = '_';
        }
        palavra_adivinhada[tamanho_palavra] = '\0';
        
        int tentativas = 0;
        int acertos = 0;
        char letras_usadas[26] = {0};
        
        system("clear || cls");
        
        printf(GREEN"\n%s - Nivel %s\n", nome_jogador, 
              (nivel == 1) ? "Facil" : (nivel == 2) ? "Medio" : "Dificil");
        printf("Palavras restantes: %d\n", num_palavras);
        printf("Pontuacao atual: %d\n", pontuacao_total);
        printf("Palavras acertadas: %d\n", palavras_acertadas);
        
        time_t inicio = time(NULL);
        
        while(tentativas < MAX_TENTATIVAS && acertos < tamanho_palavra) {
            desenhaForca(tentativas);
            printf(YELLOW"Palavra: %s\n"RESET, palavra_adivinhada);
            printf(YELLOW"Letras usadas: "RESET);
            
            for(int i = 0; i < 26; i++) {
                if(letras_usadas[i]) {
                    printf("%c ", 'a' + i);
                }
            }
            printf("\n");
            
            printf(CYAN"Digite uma letra (ou 0 para sair): "RESET);
            char letra;
            scanf(" %c", &letra);
            
            if(letra == '0') {
                sair_por_escolha = 1;
                continuar_jogando = 0;
                break;
            }
            
            letra = tolower(letra);
            
            if(letra < 'a' || letra > 'z') {
                printf(RED"Por favor, digite uma letra valida.\n"RESET);
                sleep(1);
                continue;
            }
            
            if(letras_usadas[letra - 'a']) {
                printf(RED"Voce ja tentou essa letra.\n"RESET);
                sleep(1);
                continue;
            }
            
            letras_usadas[letra - 'a'] = 1;
            
            int letra_correta = 0;
            for(int i = 0; i < tamanho_palavra; i++) {
                if(palavra_secreta[i] == letra) {
                    palavra_adivinhada[i] = letra;
                    acertos++;
                    letra_correta = 1;
                }
            }
            
            if(!letra_correta) {
                tentativas++;
                printf(RED"Letra incorreta! Tentativas restantes: %d\n"RESET, MAX_TENTATIVAS - tentativas);
                sleep(1);
            }
        }
        
        time_t fim = time(NULL);
        double tempo_jogo = difftime(fim, inicio);
        tempo_total += tempo_jogo;
        
        system("clear || cls");
        desenhaForca(tentativas);
        
        if(sair_por_escolha) {
            if(palavras_acertadas > 0) {
                printf(GREEN"\nJogo interrompido pelo jogador.\n");
                printf("Pontuacao acumulada: %d\n", pontuacao_total);
                printf("Palavras acertadas: %d\n", palavras_acertadas);
                printf("Tempo total de jogo: %.1f segundos\n"RESET, tempo_total);
                
                adicionarAoRanking(nome_jogador, pontuacao_total, tempo_total, nivel, palavras_acertadas);
            } else {
                printf(YELLOW"\nJogo interrompido pelo jogador.\n");
                printf("Nenhuma palavra foi acertada.\n"RESET);
            }
            break;
        }
        
        if(acertos == tamanho_palavra) {
            int pontuacao = tamanho_palavra * nivel * 100 - (int)(tempo_jogo * 10);
            if (pontuacao < 0) pontuacao = 0;
            pontuacao_total += pontuacao;
            palavras_acertadas++;
            
            printf(GREEN"\nParabens! Voce acertou a palavra: %s\n", palavra_secreta);
            printf("Tempo nesta palavra: %.1f segundos\n", tempo_jogo);
            printf("Pontuacao nesta palavra: %d\n", pontuacao);
            printf("Pontuacao total: %d\n", pontuacao_total);
            printf("Palavras acertadas: %d\n"RESET, palavras_acertadas);
            
            if(num_palavras > 0) {
                printf(GREEN"\nPreparando a proxima palavra...\n"RESET);
                sleep(2);
            } else {
                printf(GREEN"\nVoce acertou todas as palavras deste nivel!\n"RESET);
                continuar_jogando = 0;
            }
        } else if (continuar_jogando) {
            printf(RED"\nGame Over! A palavra era: %s\n", palavra_secreta);
            printf("Pontuacao total: %d\n", pontuacao_total);
            printf("Palavras acertadas: %d\n"RESET, palavras_acertadas);
            continuar_jogando = 0;
        }
    }
    
    if(!sair_por_escolha && palavras_acertadas > 0) {
        adicionarAoRanking(nome_jogador, pontuacao_total, tempo_total, nivel, palavras_acertadas);
    }
    
    printf(RED"\nPressione Enter para voltar ao menu..."RESET);
    getchar();
    getchar();
    system("clear || cls");
}

int main() {
    carregarRanking();
    int opcao;
    char nome_jogador[50] = {0};
    // Menu principal do jogo
    do {
        printf(YELLOW"\n================ JOGO DA FORCA ================\n");
        printf(MAGENTA"1. Jogar (Facil)\n");
        printf(MAGENTA"2. Jogar (Medio)\n");
        printf(MAGENTA"3. Jogar (Dificil)\n");
         printf(MAGENTA"4. Ver Ranking\n");
        printf(MAGENTA"5. Sair\n");
        printf(CYAN"Escolha uma opcao: "RESET);
        
        scanf("%d", &opcao);
        getchar(); // Limpa o buffer do teclado
        
        system("clear || cls");
        
        switch(opcao) {
            case 1:
            case 2:
            case 3:
                if (nome_jogador[0] == '\0') {
                    printf(CYAN"Digite seu nome: "RESET);
                    fgets(nome_jogador, sizeof(nome_jogador), stdin);
                    nome_jogador[strcspn(nome_jogador, "\n")] = '\0';
                    system("clear || cls");
                }
                jogarForca(opcao, nome_jogador);
                break;
            case 4:
                mostrarRanking();
                printf(RED"\nPressione Enter para voltar ao menu..."RESET);
                getchar();
                system("clear || cls");
                break;
            case 5:
                salvarRanking();
                printf(GREEN"Obrigado por jogar!\n"RESET);
                break;
            default:
                printf(RED"Opcao invalida! Por favor, escolha entre 1 e 5.\n"RESET);
        }
    } while(opcao != 5);
    
    return 0;
}