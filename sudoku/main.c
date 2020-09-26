#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int check(int** grid, int x, int y, int dim, int n){
    int l=sqrt(dim),startx,starty;
    for(int i=0;i<dim;i=i+l) if(x>=i) startx=i;
    for(int i=0;i<dim;i=i+l) if(y>=i) starty=i;
    for(int i=startx;i<startx+l && i<dim;i++) for(int j=starty;j<starty+l &&j<dim;j++) if(grid[i][j]==n && (i!=x || j!=y)) return 0;
    for(int i=0;i<dim;i++) if(grid[i][y]==n || grid[x][i]==n) return 0;
    return 1;
}
int select_diff()
{
    int x=0;
    while(x<1 || x>3){
        printf("Select the difficulty(1 easy-2 normal-3 hard): ");
        scanf("%d",&x);
    }
    return x;
}
int get_n_already_checked(int diff)
{
    if(diff==1) return (rand() % 21) + 30;
    if(diff==2) return (rand() % 16) + 15;
    if(diff==3) return (rand() % 16);
    return 0;
}
void disp_rip(int pos, int** grid, int** sol, int n, int* end){
    int t;
    if(pos>=n*n){
        int k, q;
        *end = 1;
        for(q = 0; q<n; q++){
            for(k = 0; k<n; k++){
                grid[q][k] = sol[q][k];
            }
        }
        return;
    }
    int i = pos/n; int j = pos%n;
    if(sol[i][j] != 0){
        disp_rip(pos+1, grid, sol, n, end);
        return;
    }
    for(t=1; t<=n; t++){
        if(check(sol, i, j, n, t)){
            sol[i][j] = t;
            disp_rip(pos+1, grid, sol, n, end);
            if(*end == 1)
                return;
        }
        sol[i][j] = 0;
    }
    return;
}
void find_sol(int** grid){
    int pos = 0;
    int* end = malloc(sizeof(int));
    *end = 0;
    int** sol = malloc(9*sizeof(int));
    for(int i = 0; i<9; i++)
        sol[i] = malloc(9*sizeof(int));
    for(int i = 0; i<9; i++){
        for(int j = 0; j<9; j++){
            sol[i][j] = grid[i][j];
        }
    }
    int n = 9;
    disp_rip(pos, grid, sol, n, end);
    for(int i = 0; i<n; i++)
        free(sol[i]);
    free(sol);
    free(end);
}
void print_grid(int** grid)
{
    for(int i = 0; i<9; i++){
        for(int j = 0; j<9; j++){
            if(j==3 || j==6){
                if(grid[i][j] == 0) printf(" | : ");
                else    printf(" | %d ", grid[i][j]);
            }
            else{
                if(grid[i][j] == 0) printf(" : ");
                else    printf(" %d ", grid[i][j]);
            }
        }
        if(i==2 || i==5)    printf("\n -----------------------------\n");
        else printf("\n");
    }
}
int finish(int** grid,int dim)
{
    for(int i=0;i<dim;i++) for(int j=0;j<dim;j++) if(grid[i][j]==0) return 0;
    return 1;
}
void create_hard_grid_and_complete_it(int** grid)
{
    int n,x,y;
    int ins = 0,num=0;
    for(int i = 0; i<10; i++){
        while(ins == 0){
            x = rand() % 9;
            y = rand() % 9; //coordinate di inserimento
            if(grid[x][y] == 0){
                num = 0;
                while(num == 0){
                    n = rand() % 9 + 1; //numero da inserire
                    if(check(grid, x, y,9,n)){
                        ins = 1;
                        num = 1;
                        grid[x][y] = n;
                    }
                }
            }
        }
        ins = 0;
    }
    find_sol(grid);
}
void create_grid(int** grid,int** grid_sol)
{
    create_hard_grid_and_complete_it(grid_sol);
    int checked=get_n_already_checked(select_diff());
    int x,y;
    int ins = 0;
    for(int i = 0; i<checked; i++){
        while(ins == 0){
            x = rand() % 9;
            y = rand() % 9; //coordinate di inserimento
            if(grid[x][y] == 0){
                grid[x][y]=grid_sol[x][y];
                ins=1;
            }
        }
        ins = 0;
    }
}
int start_game(int** grid)
{
    int game_on=1,win=0;
    int number,row,column;
    int** grid_game = malloc(9*sizeof(int));
    for(int i = 0; i<9; i++)
        grid_game[i] = malloc(9*sizeof(int));
    for(int i = 0; i<9; i++){
        for(int j = 0; j<9; j++){
            grid_game[i][j] = grid[i][j];
        }
    }
    while(game_on){
        int change=0;
        printf("\nSelect the number to put : ");
        scanf("%d",&number);
        printf("Select the row where to put the number : ");
        scanf("%d",&row);
        printf("Select the column where to put the number : ");
        scanf("%d",&column);
        row--;column--;
        if(grid[row][column]!=0) printf("There is checked number here that cannot be changed!");
        else if(grid[row][column]==0 && grid_game[row][column]!=0){
            int choice=0;
            while(choice<1 || choice>2){
                printf("There is already a checked number that can be changed, do you want to change it(1 yes-2 no): ");
                scanf("%d",&choice);
            }
            if(choice==1)   change=1;
        }
        else if(grid_game[row][column]==0)  change=1;
        if(change==1){
            if(check(grid_game,row,column,9,number)){
                grid_game[row][column]=number;
                print_grid(grid_game);
                if(finish(grid_game,9)){
                    win=1;
                    printf("\n\nYOU WON CONGRATULATIONS!!!");
                    game_on=0;
                }
            }
            else{
                printf("\nYou lost!!");
                game_on=0;
            }
        }
        change=0;
    }
    for(int i = 0; i<9; i++)
        free(grid_game[i]);
    free(grid_game);
    if(win) return 1;
    else return 0;
}
int main()
{
    srand(time(0));
    int** grid;
    grid = calloc(9, sizeof(int));
    for(int i = 0; i<9; i++)
        grid[i] = calloc(9, sizeof(int));
    int** grid_sol;
    grid_sol = calloc(9, sizeof(int));
    for(int i = 0; i<9; i++)
        grid_sol[i] = calloc(9, sizeof(int));
    create_grid(grid,grid_sol);
    print_grid(grid);
    int choice=0;
    while(choice<1 || choice>2){
        printf("\nDo you want to play or to see the resolution?(1 Play-2 Resolution): ");
        scanf("%d",&choice);
    }
    if(choice==1){
        if(start_game(grid)==0){
            printf("\nResolution: \n");
            print_grid(grid_sol);
        }
    }
    else{
        printf("\nResolution: \n");
        print_grid(grid_sol);
    }
    for(int i = 0; i<9; i++){
        free(grid[i]);
        free(grid_sol[i]);
    }
    free(grid);
    free(grid_sol);
    return 0;
}
