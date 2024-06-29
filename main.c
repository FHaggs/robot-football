#include <math.h>
#include <chipmunk.h>
#include <SOIL.h>
#include <stdio.h>


// Rotinas para acesso da OpenGL
#include "opengl.h"

// Funções para movimentação de objetos
void moveAtacante(cpBody* body, void* data);
void moveDefensor(cpBody* body, void* data);
void moveGoalie(cpBody* body, void* data);

const int MEIO_CAMPO = LARGURA_JAN / 2;

// Conveniencia
inline jogador_data init_jogador_data(cpVect resting_pos, time time){
    jogador_data j = {resting_pos, time};
    return j;
}
// Prototipos
void initCM();
void freeCM();
void restartCM();
cpShape* newLine(cpVect inicio, cpVect fim, cpFloat fric, cpFloat elast);
cpBody* newCircle(cpVect pos, cpFloat radius, cpFloat mass, char* img, bodyMotionFunc func, cpFloat fric, cpFloat elast, jogador_data j);

// Score do jogo
int score1 = 0;
int score2 = 0;

// Flag de controle: 1 se o jogo tiver acabado
int gameOver = 0;

// cpVect e' um vetor 2D e cpv(freeCM) e' uma forma rapida de inicializar ele.
cpVect gravity;

// O ambiente
cpSpace* space;

// Paredes "invisíveis" do ambiente
cpShape* leftWall, *rightWall, *topWall, *bottomWall;

// A bola
cpBody* ballBody;


// Robots
cpBody* goalie_left;
cpBody* defender1_left;
cpBody* defender2_left;
cpBody* defender3_left;
cpBody* striker1_left;
cpBody* striker2_left;

// Cada passo de simulação é 1/60 seg.
cpFloat timeStep = 1.0/60.0;

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
    //cpSpaceSetGravity(space, gravity);

    // Adiciona 4 linhas estáticas para formarem as "paredes" do ambiente
    leftWall   = newLine(cpv(0,0), cpv(0,ALTURA_JAN), 0, 1.0);
    rightWall  = newLine(cpv(LARGURA_JAN,0), cpv(LARGURA_JAN,ALTURA_JAN), 0, 1.0);
    bottomWall = newLine(cpv(0,0), cpv(LARGURA_JAN,0), 0, 1.0);
    topWall    = newLine(cpv(0,ALTURA_JAN), cpv(LARGURA_JAN,ALTURA_JAN), 0, 1.0);

    // Agora criamos a bola...
    // Os parâmetros são:
    //   - posição: cpVect (vetor: x e y)
    //   - raio
    //   - massa
    //   - imagem a ser carregada
    //   - ponteiro para a função de movimentação (chamada a cada passo, pode ser NULL)
    //   - coeficiente de fricção
    //   - coeficiente de elasticidade
    ballBody = newCircle(cpv(512,350), 8, 1, "small_football.png", NULL, 0.2, 1, init_jogador_data(cpv(LARGURA_JAN/2 ,ALTURA_JAN/2), LEFT));

    // Creating goalkeeper
    cpVect golie_pos = cpv(70, ALTURA_JAN / 2);
    goalie_left = newCircle(golie_pos, 20, 5, "ship1.png", moveGoalie, 0.2, 0.5, init_jogador_data(golie_pos, LEFT));
    // Creating defenders
    cpVect defender1_left_pos = cpv(150, 200);
    cpVect defender2_left_pos = cpv(150, 350);
    cpVect defender3_left_pos = cpv(150, 500);

    defender1_left = newCircle(defender1_left_pos, 20, 5, "ship1.png", moveDefensor, 0.2, 0.5, init_jogador_data(defender1_left_pos, LEFT));
    defender2_left = newCircle(defender2_left_pos, 20, 5, "ship1.png", moveDefensor, 0.2, 0.5, init_jogador_data(defender2_left_pos, LEFT));
    defender3_left = newCircle(defender3_left_pos, 20, 5, "ship1.png", moveDefensor, 0.2, 0.5, init_jogador_data(defender3_left_pos, LEFT));
    // Creating strikers
    cpVect s1_pos = cpv(450,250);
    cpVect s2_pos = cpv(450,450);

    striker1_left = newCircle(s1_pos, 20, 5, "ship1.png", moveAtacante, 0.2, 0.5, init_jogador_data(s1_pos, LEFT));
    striker2_left = newCircle(s2_pos, 20, 5, "ship1.png", moveAtacante, 0.2, 0.5, init_jogador_data(s2_pos, LEFT));
}

int is_near(float value, float target, float offset) {
    if ((value >= target - offset) && (value <= target + offset)) {
        return 1;
    }
    return 0;
}

void moveAtacante(cpBody* body, void* data)
{
    UserData* ud = (UserData*)data;
    jogador_data j = ud->jogadorData; 
    // Veja como obter e limitar a velocidade do robô...
    cpVect vel = cpBodyGetVelocity(body);
//    printf("vel: %f %f", vel.x,vel.y);

    // Limita o vetor em 50 unidades
    vel = cpvclamp(vel, 50);
    // E seta novamente a velocidade do corpo
    cpBodySetVelocity(body, vel);

    // Obtém a posição do robô e da bola...
    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);
    cpVect goleiraPos;
    if(j.time == LEFT){
        goleiraPos.x = LARGURA_JAN - 10;
    }else {
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
    if (is_near(cos, -0.8, 0.2)){
        // Caso de chute ao gol
        delta = cpvadd(ballPos,pos);
        //printf("Near CHUTE:x=%d \n",delta.x);
    }
    else if(is_near(cos, 0, 0.8)){
        delta = cpvadd(ballPos,pos);
        cpVect rearrange;
        rearrange.x = -delta.x;
        rearrange.y = 0;
        delta = cpvadd(delta, rearrange);
        //printf("Near 0:x=%d \n",delta.x);
    }
    else{
        //vetor robo à resting pos
        delta = cpvadd(pos, j.resting_pos);
        //printf("Go home...:x=%d \n",delta.x);
    }

    // Limita o impulso em 20 unidades
    delta = cpvmult(cpvnormalize(delta), 10);
    // Finalmente, aplica impulso no robô
    cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
}
int is_my_ball_in_side(float ballpos_x, time my_time){
    if((my_time == LEFT && ballpos_x < MEIO_CAMPO) || ((my_time == RIGHT) && ballpos_x > MEIO_CAMPO)){
        return 1;
    }
    return 0;
}

void moveDefensor(cpBody* body, void* data)
{
    UserData* ud = (UserData*)data;
    jogador_data j = ud->jogadorData; 
    // Veja como obter e limitar a velocidade do robô...
    cpVect vel = cpBodyGetVelocity(body);

    // Limita o vetor em 50 unidades
    vel = cpvclamp(vel, 50);
    // E seta novamente a velocidade do corpo
    cpBodySetVelocity(body, vel);

    // Obtém a posição do robô e da bola...
    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);
    int max_y = ALTURA_JAN / 2 + 50;
    int min_y = ALTURA_JAN / 2 - 50;

    cpVect goleiraPos;
    if(j.time == RIGHT){
        goleiraPos.x = LARGURA_JAN - 10;
    }else {
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
    // Fazer isso
    // if(cos > 0 && is_my_ball_in_side(ballPos.x, j.time)){
    //     // chutar a bola para frente
    //     delta = cpvadd(ballBody, pos);
    // }
    // else {
    //     // JOGO DE CORPO
    //     if (j.time == RIGHT){

    //     }else{

    //     }

    // }

    // Limita o impulso em 20 unidades
    delta = cpvmult(cpvnormalize(delta), 10);
    // Finalmente, aplica impulso no robô
    cpBodyApplyImpulseAtWorldPoint(body, delta, robotPos);
    
}

void moveGoalie(cpBody* body, void* data)
{
    UserData* ud = (UserData*)data;
    jogador_data j = ud->jogadorData; 
    // Veja como obter e limitar a velocidade do robô...
    cpVect vel = cpBodyGetVelocity(body);
//    printf("vel: %f %f", vel.x,vel.y);

    // Limita o vetor em 50 unidades
    vel = cpvclamp(vel, 50);
    // E seta novamente a velocidade do corpo
    cpBodySetVelocity(body, vel);

    // Obtém a posição do robô e da bola...
    cpVect robotPos = cpBodyGetPosition(body);
    cpVect ballPos  = cpBodyGetPosition(ballBody);
    int max_y = ALTURA_JAN / 2 + 50;
    int min_y = ALTURA_JAN / 2 - 50;

    cpVect goleiraPos;
    if(j.time == RIGHT){
        goleiraPos.x = LARGURA_JAN - 10;
    }else {
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
    if (is_near(cos, 1, 0.01)){
        printf("Olhos na bola \n");
    }
    else if (robotPos.y < max_y && robotPos.y > min_y){
        // Fechar o angulo
        delta = cpvadd(ballPos,pos);
        delta.x = 0;
        printf("Fechando algulo:x=%d \n",delta.x);
    }else {
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
    UserData* ud = cpBodyGetUserData(ballBody);
    cpShapeFree(ud->shape);
    cpBodyFree(ballBody);

    //Liberar o goleiro
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

    cpShapeFree(leftWall);
    cpShapeFree(rightWall);
    cpShapeFree(bottomWall);
    cpShapeFree(topWall);

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

int main(int argc, char** argv)
{
    // Inicialização da janela gráfica
    init(argc,argv);

    // Não retorna... a partir daqui, interação via teclado e mouse apenas, na janela gráfica
    glutMainLoop();
    return 0;
}

// Cria e adiciona uma nova linha estática (segmento) ao ambiente
cpShape* newLine(cpVect inicio, cpVect fim, cpFloat fric, cpFloat elast)
{
   cpShape* aux = cpSegmentShapeNew(cpSpaceGetStaticBody(space), inicio, fim, 0);
   cpShapeSetFriction(aux, fric);
   cpShapeSetElasticity(aux, elast);
   cpSpaceAddShape(space, aux);
   return aux;
}

// Cria e adiciona um novo corpo dinâmico, com formato circular
cpBody* newCircle(cpVect pos, cpFloat radius, cpFloat mass, char* img, bodyMotionFunc func, cpFloat fric, cpFloat elast, jogador_data j)
{
    // Primeiro criamos um cpBody para armazenar as propriedades fisicas do objeto
    // Estas incluem: massa, posicao, velocidade, angulo, etc do objeto
    // A seguir, adicionamos formas de colisao ao cpBody para informar o seu formato e tamanho

    // O momento de inercia e' como a massa, mas para rotacao
    // Use as funcoes cpMomentFor*() para calcular a aproximacao dele
    cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);

    // As funcoes cpSpaceAdd*() retornam o que voce esta' adicionando
    // E' conveniente criar e adicionar um objeto na mesma linha
    cpBody* newBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));

    // Por fim, ajustamos a posicao inicial do objeto
    cpBodySetPosition(newBody, pos);

    // Agora criamos a forma de colisao do objeto
    // Voce pode criar multiplas formas de colisao, que apontam ao mesmo objeto (mas nao e' necessario para o trabalho)
    // Todas serao conectadas a ele, e se moverao juntamente com ele
    cpShape* newShape = cpSpaceAddShape(space, cpCircleShapeNew(newBody, radius, cpvzero));
    cpShapeSetFriction(newShape, fric);
    cpShapeSetElasticity(newShape, elast);

    UserData* newUserData = malloc(sizeof(UserData));
    newUserData->tex = loadImage(img);
    newUserData->radius = radius;
    newUserData->shape= newShape;
    newUserData->func = func;
    newUserData->jogadorData = j;
    cpBodySetUserData(newBody, newUserData);
    printf("newCircle: loaded img %s\n", img);
    return newBody;
}
