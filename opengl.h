#ifndef MYOPENGL_H
#define MYOPENGL_H

#ifdef _WIN32
#include <windows.h>    // includes only in MSWindows not in UNIX
#include "gl/glut.h"
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <chipmunk.h>

#define LARGURA_JAN 1024
#define ALTURA_JAN 712

typedef enum {
    RIGHT,
    LEFT
} time;

// Abstração maior de uma struct quem contem metadados
typedef struct {
    cpVect resting_pos;
    time time;
} jogador_data;

// Definição dos parâmetros das funções de movimento
// (tipo do ponteiro de função)
typedef void (*bodyMotionFunc)(cpBody* body, void* data);

typedef struct
{
    GLuint tex;
    cpFloat radius;
    cpShape* shape;
    bodyMotionFunc func;
    jogador_data jogadorData; 
} UserData;

// Funções da interface gráfica e OpenGL
void init(int argc, char** argv);
GLuint loadImage(char* img);

#endif // MYOPENGL_H
