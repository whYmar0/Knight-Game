#include <stdio.h>
#include <string.h>

#define SIZE 3

void print_board(char board[SIZE][SIZE]) {
    printf("  0 1 2\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%d ", i);
        for (int j = 0; j < SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

int cw(char board[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ')
            return board[i][0];
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ')
            return board[0][i];
    }
    
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ')
        return board[0][0];
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ')
        return board[0][2];

    return ' ';
}

int main() {
    char board[SIZE][SIZE] = { {' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '} };
    char player1[20], player2[20];
    char player = 'X';
    int row, col, moves = 0;

    printf("Боец 1: ");
    fgets(player1, 20, stdin);
    player1[strcspn(player1, "\n")] = 0; 

    printf("Боец 2: ");
    fgets(player2, 20, stdin);
    player2[strcspn(player2, "\n")] = 0; 

    while (1) {
        print_board(board);

        while (1) {
            printf("Ел хьай ход %s: ", (player == 'X') ? player1 : player2);
            scanf("%d %d", &row, &col);


            if (row >= 0 && row < SIZE && col >= 0 && col < SIZE) {

                if (board[row][col] == ' ') {
                    board[row][col] = player;
                    break;
                } else {
                    printf("Шнекс мич вал г1ерт хьо\n");
                }
            } else {
                printf("Хьай прицел нисье\n");
            }
        }

        moves++;

        char winner = cw(board);
        if (winner != ' ') {
            print_board(board);
            printf("Яьккха ахь и все таки %s\n", (winner == 'X') ? player1 : player2);
            break;
        }

        if (moves == SIZE * SIZE) {
            print_board(board);
            printf("Ловз 1амде шаьшшин\n");
            break;
        }

        player = (player == 'X') ? 'O' : 'X';
    }

    return 0;
}

