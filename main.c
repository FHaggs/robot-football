#include <math.h>
#include <chipmunk.h>
#include <SOIL.h>
#include <stdio.h>
#include <time.h>
// Rotinas para acesso da OpenGL
#include "opengl.h"

// Funções para movimentação de objetos
void moveAtacante(cpBody *body, void *data);
void moveDefensor(cpBody *body, void *data);
void moveGoalie(cpBody *body, void *data);

void check_goal(cpBody *body, void *data);

const int MEIO_CAMPO = LARGURA_JAN / 2;
// Conveniencia
inline body_data init_body_data(cpVect resting_pos, team team, int id_num)
{
    body_data j = {resting_pos, team, id_num};
    return j;
}
// Prototipos
void initCM();
void freeCM();
void restartCM();
cpShape *newLine(cpVect inicio, cpVect fim, cpFloat fric, cpFloat elast);
cpBody *newCircle(cpVect pos, cpFloat radius, cpFloat mass, char *img, bodyMotionFunc func, cpFloat fric, cpFloat elast, body_data j);
int is_my_ball_in_side(float ballpos_x, team my_time);
// Score do jogo
int score1 = 0;
int score2 = 0;

// Flag de controle: 1 se o jogo tiver acabado
int gameOver = 0;

// cpVect e' um vetor 2D e cpv(freeCM) e' uma forma rapida de inicializar ele.
cpVect gravity;

// O ambiente
cpSpace *space;

// Paredes "invisíveis" do ambiente
cpShape *leftWall, *rightWall, *topWall, *bottomWall;

// A bola
cpBody *ballBody;

// Robots
cpBody *goalie_left;
cpBody *defender1_left;
cpBody *defender2_left;
cpBody *defender3_left;
cpBody *striker1_left;
cpBody *striker2_left;

cpBody *goalie_right;
cpBody *defender1_right;
cpBody *defender2_right;
cpBody *defender3_right;
cpBody *striker1_right;
cpBody *striker2_right;

// Cada passo de simulação é 1/60 seg.
cpFloat timeStep = 1.0 / 60.0;

cpShape *cornerWall_left_bottom, *cornerWall_left_top, *cornerWall_right_bottom, *cornerWall_right_top;
// Inicializa o ambiente: é chamada por init() em opengl.c, pois necessita do contexto
// OpenGL para a leitura das imagens,
void initCM()
{
    gravity = cpv(0, 100);

    // Cria o universo
    space = cpSpaceNew();

    // Seta o fator de damping, isto é, de atrito do ar
    cpSpaceSetDamping(space, 0.8);

    // Descomente a linha abaixo se quiser ver o efeito da gravidade!
    // cpSpaceSetGravity(space, gravity);


    // Adiciona 4 linhas estáticas para formarem as "paredes" do ambiente
    leftWall = newLine(cpv(0, 0), cpv(0, ALTURA_JAN), 0, 1.0);
    rightWall = newLine(cpv(LARGURA_JAN, 0), cpv(LARGURA_JAN, ALTURA_JAN), 0, 1.0);
    bottomWall = newLine(cpv(0, 0), cpv(LARGURA_JAN, 0), 0, 1.0);
    topWall = newLine(cpv(0, ALTURA_JAN), cpv(LARGURA_JAN, ALTURA_JAN), 0, 1.0);

    cornerWall_left_bottom = newLine(cpv(0, ALTURA_JAN -50), cpv(50, ALTURA_JAN), 0, 2.0);
    cornerWall_left_top = newLine(cpv(0, 50), cpv(50, 0), 0, 2.0);
    
    cornerWall_right_bottom = newLine(cpv(LARGURA_JAN, ALTURA_JAN -50), cpv(LARGURA_JAN -50, ALTURA_JAN), 0, 2.0);
    cornerWall_right_top = newLine(cpv(LARGURA_JAN -50, 0), cpv(LARGURA_JAN, ALTURA_JAN -50), 0, 2.0);


    // Agora criamos a bola...
    // Os parâmetros são:
    //   - posição: cpVect (vetor: x e y)
    //   - raio
    //   - massa
    //   - imagem a ser carregada
    //   - ponteiro para a função de movimentação (chamada a cada passo, pode ser NULL)
    //   - coeficiente de fricção
    //   - coeficiente de elasticidade
    ballBody = newCircle(cpv(512, 350), 8, 1, "small_football.png", check_goal, 0.2, 1, init_body_data(cpv(512, 350), LEFT, 0));

    // Creating goalkeeper
    cpVect golie_pos = cpv(70, ALTURA_JAN / 2);
    goalie_left = newCircle(golie_pos, 20, 5, "emacs.png", moveGoalie, 0.2, 0.5, init_body_data(golie_pos, LEFT, 0));
    // Creating defenders
    cpVect defender1_left_pos = cpv(150, 200);
    cpVect defender2_left_pos = cpv(150, 350);
    cpVect defender3_left_pos = cpv(150, 500);

    defender1_left = newCircle(defender1_left_pos, 20, 5, "emacs.png", moveDefensor, 0.2, 0.5, init_body_data(defender1_left_pos, LEFT, 1));
    defender2_left = newCircle(defender2_left_pos, 20, 5, "emacs.png", moveDefensor, 0.2, 0.5, init_body_data(defender2_left_pos, LEFT, 2));
    defender3_left = newCircle(defender3_left_pos, 20, 5, "emacs.png", moveDefensor, 0.2, 0.5, init_body_data(defender3_left_pos, LEFT, 3));
    // Creating strikers
    cpVect s1_pos = cpv(440, 250);
    cpVect s2_pos = cpv(450, 450);

    striker1_left = newCircle(s1_pos, 20, 5, "emacs.png", moveAtacante, 0.2, 0.5, init_body_data(s1_pos, LEFT, 1));
    striker2_left = newCircle(s2_pos, 20, 5, "emacs.png", moveAtacante, 0.2, 0.5, init_body_data(s2_pos, LEFT, 2));

    // RIGTH
    cpVect golie_pos_r = cpv(LARGURA_JAN-70, ALTURA_JAN / 2);
    goalie_right = newCircle(golie_pos_r, 20, 5, "vim.png", moveGoalie, 0.2, 0.5, init_body_data(golie_pos_r, RIGHT, 0));
    // Creating defenders
    cpVect defender1_right_pos = cpv(LARGURA_JAN-150, 200);
    cpVect defender2_right_pos = cpv(LARGURA_JAN-150, 350);
    cpVect defender3_right_pos = cpv(LARGURA_JAN-150, 500);

    defender1_right = newCircle(defender1_right_pos, 20, 5, "vim.png", moveDefensor, 0.2, 0.5, init_body_data(defender1_right_pos, RIGHT, 1));
    defender2_right = newCircle(defender2_right_pos, 20, 5, "vim.png", moveDefensor, 0.2, 0.5, init_body_data(defender2_right_pos, RIGHT, 2));
    defender3_right = newCircle(defender3_right_pos, 20, 5, "vim.png", moveDefensor, 0.2, 0.5, init_body_data(defender3_right_pos, RIGHT, 3));
    // Creating strikers
    cpVect s1_pos_r = cpv(LARGURA_JAN-440, 250);
    cpVect s2_pos_r = cpv(LARGURA_JAN-450, 450);

    striker1_right = newCircle(s1_pos_r, 20, 5, "vim.png", moveAtacante, 0.2, 0.5, init_body_data(s1_pos_r, RIGHT, 1));
    striker2_right = newCircle(s2_pos_r, 20, 5, "vim.png", moveAtacante, 0.2, 0.5, init_body_data(s2_pos_r, RIGHT, 2));
}
void check_goal(cpBody *body, void *data){
    UserData *ud = (UserData *)data;
    body_data j = ud->BodyData;

    cpVect ballPos = cpBodyGetPosition(body);
    cpVect goleiraPos_left;
    cpVect goleiraPos_right;

    goleiraPos_left.y = ALTURA_JAN / 2;
    goleiraPos_left.x = LARGURA_JAN - 10;

    goleiraPos_right.y = ALTURA_JAN / 2;
    goleiraPos_right.x = 10;

    
}

int is_near(float value, float target, float offset)
{
    if ((value >= target - offset) && (value <= target + offset))
    {
        return 1;
    }
    return 0;
}
int get_motivacao(int min, int max){
    return min + rand() % (max - min + 1);
}
double euclidean_distance(cpVect point1, cpVect point2)
{
    return sqrt(pow(point2.x - point1.x, 2) + pow(point2.y - point1.y, 2));
}

void moveAtacante(cpBody *body, void *data)
{
    UserData *ud = (UserData *)data;
    body_data j = ud->BodyData;
    // Veja como obter e limitar a velocidade do robô...
    cpVect vel = cpBodyGetVelocity(body);
    //    printf("vel: %f %f", vel.x,vel.y);

    // Limita o vetor no fator motivacao dele
    vel = cpvclamp(vel, get_motivacao(50, 80));
    // E seta novamente a velocidade do corpo
    cpBodySetVelocity(body, vel);

    // Obtém a posição do robô e da bola...
    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos = cpBodyGetPosition(ballBody);

    cpVect striker_friend_pos;
    if(j.team == LEFT){
        if(j.id_number == 1){
            cpVect aux = cpBodyGetPosition(striker2_left);
            striker_friend_pos.x = aux.x;
            striker_friend_pos.y = aux.y;
        }else{
            cpVect aux = cpBodyGetPosition(striker1_left);
            striker_friend_pos.x = aux.x;
            striker_friend_pos.y = aux.y;
        }
    }else{
        if(j.id_number == 1){
            cpVect aux = cpBodyGetPosition(striker2_right);
            striker_friend_pos.x = aux.x;
            striker_friend_pos.y = aux.y;
        }else{
            cpVect aux = cpBodyGetPosition(striker1_right);
            striker_friend_pos.x = aux.x;
            striker_friend_pos.y = aux.y;
        }
    }

    cpVect goleiraPos;
    if (j.team == LEFT)
    {
        goleiraPos.x = LARGURA_JAN - 10;
    }
    else
    {
        goleiraPos.x = 10;
    }

    goleiraPos.y = ALTURA_JAN / 2;

    // Cos de angulo entre vetores
    cpVect u;
    u.x = robotPos.x - ballPos.x;
    u.y = robotPos.y - ballPos.y;

    cpVect v;
    v.x = goleiraPos.x - ballPos.x;
    v.y = goleiraPos.y - ballPos.y;

    double escalar = u.x * v.x + u.y * v.y;
    double u_abs = sqrt(pow(u.x, 2) + pow(u.y, 2));
    double v_abs = sqrt(pow(v.x, 2) + pow(v.y, 2));

    double cos = escalar / (u_abs * v_abs);

    // Calcula um vetor do robô à bola (DELTA = B - R)
    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta;
    // printf("forca: %f \n", delta.x);
    if (is_near(cos, -0.8, 0.2) && !(is_my_ball_in_side(ballPos.x, j.team)))
    {
        // Caso de chute ao gol
        double my_friends_distance = euclidean_distance(ballPos, striker_friend_pos);
        double my_distance = euclidean_distance(ballPos, robotPos);
        if (my_distance < my_friends_distance){
            delta = cpvadd(ballPos, pos);
        }else{
            cpVect advanced_pos = j.resting_pos;
            advanced_pos.x = ballPos.x;
            delta = cpvadd(pos, advanced_pos);
        }
    }
    else if (is_near(cos, 0, 0.8) && !(is_my_ball_in_side(ballPos.x, j.team)))
    {
        if(get_motivacao(1, 10) >= 8){
            // O atacante pode tentar um chute arriscado, mesmo com angulo ruim
            delta = cpvadd(ballPos, pos);
        }else{
            delta = cpvadd(ballPos, pos);
            cpVect rearrange;
            rearrange.x = -delta.x;
            rearrange.y = 0;
            delta = cpvadd(delta, rearrange);
        }

    }
    else
    {
        // vetor robo à resting pos
        delta = cpvadd(pos, j.resting_pos);
        // printf("Go home...:x=%d \n",delta.x);
    }

    // Limita o impulso dependendo da motivacao do jogador
    delta = cpvmult(cpvnormalize(delta), get_motivacao(40, 70));
    // Finalmente, aplica impulso no robô
    cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
}
int is_my_ball_in_side(float ballpos_x, team my_time)
{
    if ((my_time == LEFT && ballpos_x < MEIO_CAMPO) || ((my_time == RIGHT) && ballpos_x > MEIO_CAMPO))
    {
        return 1;
    }
    return 0;
}


void moveDefensor(cpBody *body, void *data)
{
    UserData *ud = (UserData *)data;
    body_data j = ud->BodyData;
    // Veja como obter e limitar a velocidade do robô...
    cpVect vel = cpBodyGetVelocity(body);

    // Limita o vetor em 50 unidades
    vel = cpvclamp(vel, get_motivacao(30, 50));
    // E seta novamente a velocidade do corpo
    cpBodySetVelocity(body, vel);

    // Obtém a posição do robô e da bola...
    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos = cpBodyGetPosition(ballBody);

    // strikers pos
    cpVect striker_left_1_pos = cpBodyGetPosition(striker1_left);
    cpVect striker_left_2_pos = cpBodyGetPosition(striker2_left);

    cpVect striker_right_1_pos = cpBodyGetPosition(striker1_right);
    cpVect striker_right_2_pos = cpBodyGetPosition(striker2_right);

    cpVect goleiraPos;
    if (j.team == RIGHT)
    {
        goleiraPos.x = LARGURA_JAN - 10;
    }
    else
    {
        goleiraPos.x = 10;
    }

    goleiraPos.y = ALTURA_JAN / 2;

    // Cos de angulo entre vetores
    cpVect u;
    u.x = robotPos.x - ballPos.x;
    u.y = robotPos.y - ballPos.y;

    cpVect v;
    v.x = goleiraPos.x - ballPos.x;
    v.y = goleiraPos.y - ballPos.y;

    double escalar = u.x * v.x + u.y * v.y;
    double u_abs = sqrt(pow(u.x, 2) + pow(u.y, 2));
    double v_abs = sqrt(pow(v.x, 2) + pow(v.y, 2));

    double cos = escalar / (u_abs * v_abs);

    // Calcula um vetor do robô à bola (DELTA = B - R)
    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta;
    if (is_my_ball_in_side(ballPos.x, j.team))
    {
        if (cos > 0.2)
        {
            // chutar a bola para frente
            delta = cpvadd(ballPos, pos);
        }
        else{
            if (j.team == LEFT)
            {
                double distance1 = euclidean_distance(ballPos, striker_right_1_pos);
                double distance2 = euclidean_distance(ballPos, striker_right_2_pos);
                if (distance1 < distance2)
                {
                    // (DELTA = S - R)
                    delta = cpvadd(striker_right_1_pos, pos);
                }else{
                    delta = cpvadd(striker_right_2_pos, pos);
                }
            }else{
                double distance1 = euclidean_distance(ballPos, striker_left_1_pos);
                double distance2 = euclidean_distance(ballPos, striker_left_2_pos);
                if (distance1 < distance2)
                {
                    // (DELTA = S - R)
                    delta = cpvadd(striker_left_1_pos, pos);
                }else{
                    delta = cpvadd(striker_left_2_pos, pos);
                }
            }
        }
    }
    else
    {
        // Go home
        delta = cpvadd(pos, j.resting_pos);
    }

    // Limita o impulso em 20 unidades
    delta = cpvmult(cpvnormalize(delta), get_motivacao(40, 60));
    // Finalmente, aplica impulso no robô
    cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
}

void moveGoalie(cpBody *body, void *data)
{
    UserData *ud = (UserData *)data;
    body_data j = ud->BodyData;
    // Veja como obter e limitar a velocidade do robô...
    cpVect vel = cpBodyGetVelocity(body);
    //    printf("vel: %f %f", vel.x,vel.y);

    // Limita o vetor em 50 unidades
    vel = cpvclamp(vel, get_motivacao(50, 70));
    // E seta novamente a velocidade do corpo
    cpBodySetVelocity(body, vel);

    // Obtém a posição do robô e da bola...
    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos = cpBodyGetPosition(ballBody);
    int max_y = ALTURA_JAN / 2 + 50;
    int min_y = ALTURA_JAN / 2 - 50;

    cpVect goleiraPos;
    if (j.team == RIGHT)
    {
        goleiraPos.x = LARGURA_JAN - 10;
    }
    else
    {
        goleiraPos.x = 10;
    }

    goleiraPos.y = ALTURA_JAN / 2;

    // Cos de angulo entre vetores
    cpVect u;
    u.x = robotPos.x - ballPos.x;
    u.y = robotPos.y - ballPos.y;

    cpVect v;
    v.x = goleiraPos.x - ballPos.x;
    v.y = goleiraPos.y - ballPos.y;

    double escalar = u.x * v.x + u.y * v.y;
    double u_abs = sqrt(pow(u.x, 2) + pow(u.y, 2));
    double v_abs = sqrt(pow(v.x, 2) + pow(v.y, 2));

    double cos = escalar / (u_abs * v_abs);

    // Calcula um vetor do robô à bola (DELTA = B - R)
    cpVect pos = robotPos;
    pos.x = -robotPos.x;
    pos.y = -robotPos.y;
    cpVect delta;
    delta.x = 0;
    delta.y = 0;

    if (!(is_near(cos, 1, 0.01)) && robotPos.y < max_y && robotPos.y > min_y)
    {
        // Fechar o angulo
        delta = cpvadd(ballPos, pos);
        delta.x = 0;
    }
    else
    {
        delta = cpvadd(pos, j.resting_pos);
    }

    // Limita o impulso em 20 unidades
    delta = cpvmult(cpvnormalize(delta), 10);
    // Finalmente, aplica impulso no robô
    cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
}

// Libera memória ocupada por cada corpo, forma e ambiente
// Acrescente mais linhas caso necessário
void freeCM()
{
    printf("Cleaning up!\n");
    UserData *ud = cpBodyGetUserData(ballBody);
    cpShapeFree(ud->shape);
    cpBodyFree(ballBody);

    // Liberar o goleiro
    ud = cpBodyGetUserData(goalie_left);
    cpShapeFree(ud->shape);
    cpBodyFree(goalie_left);
    free(ud);

    // Liberar os defensores
    ud = cpBodyGetUserData(defender1_left);
    cpShapeFree(ud->shape);
    cpBodyFree(defender1_left);
    free(ud);

    ud = cpBodyGetUserData(defender2_left);
    cpShapeFree(ud->shape);
    cpBodyFree(defender2_left);
    free(ud);

    ud = cpBodyGetUserData(defender3_left);
    cpShapeFree(ud->shape);
    cpBodyFree(defender3_left);
    free(ud);

    // Liberar os atacantes
    ud = cpBodyGetUserData(striker1_left);
    cpShapeFree(ud->shape);
    cpBodyFree(striker1_left);
    free(ud);

    ud = cpBodyGetUserData(striker2_left);
    cpShapeFree(ud->shape);
    cpBodyFree(striker2_left);
    free(ud);
    // RIGHT
    // Liberar o goleiro
    ud = cpBodyGetUserData(goalie_right);
    cpShapeFree(ud->shape);
    cpBodyFree(goalie_right);
    free(ud);

    // Liberar os defensores
    ud = cpBodyGetUserData(defender1_right);
    cpShapeFree(ud->shape);
    cpBodyFree(defender1_right);
    free(ud);

    ud = cpBodyGetUserData(defender2_right);
    cpShapeFree(ud->shape);
    cpBodyFree(defender2_right);
    free(ud);

    ud = cpBodyGetUserData(defender3_right);
    cpShapeFree(ud->shape);
    cpBodyFree(defender3_right);
    free(ud);

    // Liberar os atacantes
    ud = cpBodyGetUserData(striker1_right);
    cpShapeFree(ud->shape);
    cpBodyFree(striker1_right);
    free(ud);

    ud = cpBodyGetUserData(striker2_right);
    cpShapeFree(ud->shape);
    cpBodyFree(striker2_right);
    free(ud);

    cpShapeFree(leftWall);
    cpShapeFree(rightWall);
    cpShapeFree(bottomWall);
    cpShapeFree(topWall);
    cpShapeFree(cornerWall_left_bottom);
    cpShapeFree(cornerWall_left_top);
    cpShapeFree(cornerWall_right_bottom);
    cpShapeFree(cornerWall_right_top);


    cpSpaceFree(space);
}

// Função chamada para reiniciar a simulação
void restartCM()
{
    // Escreva o código para reposicionar os jogadores, ressetar o score, etc.

    // Não esqueça de ressetar a variável gameOver!
    gameOver = 0;
}

// ************************************************************
//
// A PARTIR DESTE PONTO, O PROGRAMA NÃO DEVE SER ALTERADO
//
// A NÃO SER QUE VOCÊ SAIBA ***EXATAMENTE*** O QUE ESTÁ FAZENDO
//
// ************************************************************

int main(int argc, char **argv)
{
    // Inicialização da janela gráfica
    init(argc, argv);
    srand((unsigned int)time(NULL));


    // Não retorna... a partir daqui, interação via teclado e mouse apenas, na janela gráfica
    glutMainLoop();
    return 0;
}

// Cria e adiciona uma nova linha estática (segmento) ao ambiente
cpShape *newLine(cpVect inicio, cpVect fim, cpFloat fric, cpFloat elast)
{
    cpShape *aux = cpSegmentShapeNew(cpSpaceGetStaticBody(space), inicio, fim, 0);
    cpShapeSetFriction(aux, fric);
    cpShapeSetElasticity(aux, elast);
    cpSpaceAddShape(space, aux);
    return aux;
}

// Cria e adiciona um novo corpo dinâmico, com formato circular
cpBody *newCircle(cpVect pos, cpFloat radius, cpFloat mass, char *img, bodyMotionFunc func, cpFloat fric, cpFloat elast, body_data j)
{
    // Primeiro criamos um cpBody para armazenar as propriedades fisicas do objeto
    // Estas incluem: massa, posicao, velocidade, angulo, etc do objeto
    // A seguir, adicionamos formas de colisao ao cpBody para informar o seu formato e tamanho

    // O momento de inercia e' como a massa, mas para rotacao
    // Use as funcoes cpMomentFor*() para calcular a aproximacao dele
    cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);

    // As funcoes cpSpaceAdd*() retornam o que voce esta' adicionando
    // E' conveniente criar e adicionar um objeto na mesma linha
    cpBody *newBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));

    // Por fim, ajustamos a posicao inicial do objeto
    cpBodySetPosition(newBody, pos);

    // Agora criamos a forma de colisao do objeto
    // Voce pode criar multiplas formas de colisao, que apontam ao mesmo objeto (mas nao e' necessario para o trabalho)
    // Todas serao conectadas a ele, e se moverao juntamente com ele
    cpShape *newShape = cpSpaceAddShape(space, cpCircleShapeNew(newBody, radius, cpvzero));
    cpShapeSetFriction(newShape, fric);
    cpShapeSetElasticity(newShape, elast);

    UserData *newUserData = malloc(sizeof(UserData));
    newUserData->tex = loadImage(img);
    newUserData->radius = radius;
    newUserData->shape = newShape;
    newUserData->func = func;
    newUserData->BodyData = j;
    cpBodySetUserData(newBody, newUserData);
    printf("newCircle: loaded img %s\n", img);
    return newBody;
}
