#include "opengl.h"
#include <string.h>
#include <math.h>
#include <SOIL.h>

// ************************************************************
//
// A PARTIR DESTE PONTO, O PROGRAMA NÃO DEVE SER ALTERADO
//
// A NÃO SER QUE VOCÊ SAIBA ***EXATAMENTE*** O QUE ESTÁ FAZENDO
//
// ************************************************************

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif // M_PI

extern cpSpace* space;

extern cpFloat timeStep;

// Score do jogo - definido em main.c
extern int score1;
extern int score2;
extern int gameOver; // 1 se o jogo tiver acabado

// Tex id para o campo
GLuint pitchTex;

// Gravidade: usada aqui apenas para testar o liga/desliga
extern cpVect gravity;

// Está aqui apenas para o teste de movimentação via teclado
extern cpBody* robotBody;

// Opções para o debugdraw - não está sendo usado
cpSpaceDebugDrawOptions drawOptions;

// Funções presentes em main.c
extern void initCM();
extern void freeCM();
extern void restartCM();

// Funções da interface gráfica e OpenGL
void display();
void keyboard( unsigned char key, int x, int y );
void mouse(int button, int state, int x, int y);
void arrow_keys(int key, int x, int y);
void reshape(int w, int h);
void timer(int val);

#ifndef __FREEGLUT_H__
void glutBitmapString(void* font, char *string );
#endif

// Protótipos para o debugdraw - não estão sendo usados
void DrawCircle(cpVect p, cpFloat a, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data);
void DrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color, cpDataPointer data);
void DrawFatSegment(cpVect a, cpVect b, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data);
void DrawPolygon(int count, const cpVect *verts, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data);
void DrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor color, cpDataPointer data);

GLuint loadImage(char* img)
{
    GLuint t = SOIL_load_OGL_texture(img,SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_MULTIPLY_ALPHA);
    if(!t) {
       printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
       exit(1);
    }
    return t;
}

void loadImages()
{
    pitchTex = loadImage("pitch.png");
    printf("Pitch tex id: %d\n", pitchTex);
}

// Seria usada por debugdraw
cpSpaceDebugColor ColorForShape(cpShape *shape, cpDataPointer data)
{
    cpSpaceDebugColor cor = { 1, 0, 0, 1 };
    return cor;
}

void init(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(10, 10);

    // Define o tamanho da janela gráfica do programa
    glutInitWindowSize(LARGURA_JAN, ALTURA_JAN);
    glutCreateWindow("Robot Football");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutTimerFunc(1, timer, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

    loadImages();
    initCM();

    // Parâmetros para o debugdraw - não está sendo usado
    drawOptions.drawCircle = DrawCircle;
    drawOptions.drawSegment = DrawSegment;
    drawOptions.drawFatSegment = DrawFatSegment;
    drawOptions.drawDot = DrawDot;
    drawOptions.colorForShape = ColorForShape;
    drawOptions.flags = (CP_SPACE_DEBUG_DRAW_SHAPES | CP_SPACE_DEBUG_DRAW_CONSTRAINTS | CP_SPACE_DEBUG_DRAW_COLLISION_POINTS);
}

void timer(int val)
{
    cpSpaceStep(space, timeStep);
    glutTimerFunc(1, timer, 0);
    glutPostRedisplay();
}

const float C80PI = 180/M_PI;
// Desenha um corpo com a textura associada
void drawBody(cpVect pos, cpFloat angle, UserData* ud)
{
    cpFloat radius = ud->radius;
    // Aplica translação e rotação no objeto
    glPushMatrix();
    glTranslatef(pos.x,pos.y,0);
    glRotatef(angle*C80PI,0,0,1);

    glBindTexture(GL_TEXTURE_2D, ud->tex);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);

    glTexCoord2f(0,0);
    glVertex2f(-radius,-radius);

    glTexCoord2f(1,0);
    glVertex2f(radius,-radius);

    glTexCoord2f(1,1);
    glVertex2f(radius,radius);

    glTexCoord2f(0,1);
    glVertex2f(-radius,radius);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void eachBodyFunc(cpBody *body, void *data)
{
    cpVect pos = cpBodyGetPosition(body);
    cpFloat angle = cpBodyGetAngle(body);
    UserData* ud = cpBodyGetUserData(body);
    // Assumindo que todos os corpos são circulares...
    drawBody(pos, angle, ud);
    // Se há uma função de movimento associada ao corpo, chama ela
    if(ud->func != NULL && !gameOver)
        (*ud->func)(body, ud);
}

// Desenha a imagem do campo
void drawPitch()
{
    glBindTexture(GL_TEXTURE_2D, pitchTex);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);

    glTexCoord2f(0,0);
    glVertex2f(0,0);

    glTexCoord2f(1,0);
    glVertex2f(LARGURA_JAN-1,0);

    glTexCoord2f(1,1);
    glVertex2f(LARGURA_JAN-1,ALTURA_JAN-1);

    glTexCoord2f(0,1);
    glVertex2f(0,ALTURA_JAN-1);

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

// Escreve o score na tela
void drawScore()
{
    char strscore1[10];
    char strscore2[20];
    sprintf(strscore1, "%d", score1);
    sprintf(strscore2, "%d", score2);
    // Sistema de coordenadas: Y cresce para BAIXO
    glRasterPos2i(20, 30);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, strscore1);
    glRasterPos2i(980, 30);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, strscore2);

    // Se a simulação terminou, exibe a mensagem do vencedor
    if(gameOver) {
        char strgameover[100];
        glRasterPos2i(LARGURA_JAN/2-200,ALTURA_JAN/2);
        if(score1 == score2)
            sprintf(strgameover, "Final: EMPATE");
        else
            sprintf(strgameover, "Final: vencedor é %s com %d gols", score1 > score2? "JOGADOR 1" : "JOGADOR 2",
                score1 > score2 ? score1 : score2);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, strgameover);
    }
}

// Desenha a tela
void display()
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Fundo de tela preto
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);

    drawPitch();
    drawScore();

    // Desenha e movimenta todos os corpos: robôs e bola
    cpSpaceEachBody(space, eachBodyFunc, NULL);
    //cpSpaceDebugDraw(space, &drawOptions);

    // Exibe a tela
    glutSwapBuffers();
}

// Callback de redimensionamento da janela
void reshape(int w, int h)
{
    // Resseta a projeção
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Ajusta a viewport para ser toda a janela
    glViewport(0, 0, w, h);
    // Ajusta a janea de visualização para corresponder aos pixels
    gluOrtho2D(0,LARGURA_JAN-1,ALTURA_JAN-1,0);
    // Volta à matriz de transformação e visualização
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


// Callback de teclado
void keyboard( unsigned char key, int x, int y )
{
  cpVect pos, grav;
  int dx = 0;
  int dy = 0;
  int delta = 100;
  switch( key ) {
    case 27:
      // ESC libera memória e finaliza a aplicação
      freeCM();
      exit(1);
      break;
    case 'f':
      // F gera um game over (teste)
      gameOver = 1;
      break;
    case 'r':
      // R faz restart na simulação (chama função restartCM)
      restartCM();
      break;
      // Teste: controla robô com WASD
    case 'w':
        dx = 0;
        dy = delta;
        break;
    case 's':
        dx = 0;
        dy = -delta;
        break;
    case 'a':
        dx = -delta;
        dy = 0;
        break;
    case 'd':
        dx = delta;
        dy = 0;
        break;
    case 'g':
      // Teste: g liga e desliga a gravidade
      grav = cpSpaceGetGravity(space);
      if(grav.y == 0)
          grav.y = gravity.y;
      else
          grav.y = 0;
      printf("Gravity: %f\n",grav.y);
      cpSpaceSetGravity(space, grav);
      break;
  }
  if(dx != 0 || dy != 0) {
      pos = cpBodyGetPosition(robotBody);
      cpBodyApplyImpulseAtWorldPoint(robotBody, cpv(dx,dy), cpv(pos.x,pos.y));
      glutPostRedisplay();
  }
}

// Callback de mouse (se necessário)
void mouse(int button, int state, int x, int y)
{
    //printf("%d %d %d\n",button,x,y);
    //if(button == 3) {
    //    //
    //}
    //else if(button == 4) {
    //    //
    //}
    glutPostRedisplay();
}

//
// Funções para o debug draw da Chipmunk - não estão sendo usadas
//
const float PI180 = 3.1415926/180;
void DrawCircle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{
    glBegin(GL_LINE_LOOP);
    float x, y, inc=10*PI180;
    for(float ang=0; ang<2*M_PI; ang+=inc)
    {
        x = pos.x + cos(ang)*radius;
        y = pos.y + sin(ang)*radius;
        glVertex2f(x,y);
    }
    glEnd();
}

void DrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color, cpDataPointer data)
{
}

void DrawFatSegment(cpVect a, cpVect b, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{
    glColor3f(1,0,0);
    glBegin(GL_LINES);
    glVertex2f(a.x,a.y);
    glVertex2f(b.x,b.y);
    glEnd();
}

void DrawPolygon(int count, const cpVect *verts, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{
    printf("*** Polygon\n");
}

void DrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor color, cpDataPointer data)
{
    printf("** Dot\n");
}

// Caso ainda seja GLUT, e não FreeGLUT...
#ifndef __FREEGLUT_H__
void glutBitmapString( void* font, char *string )
{
    while(*string)
        glutBitmapCharacter(font, *string++);
}

#endif

