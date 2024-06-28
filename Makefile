# Makefile para Linux e macOS

PROG = robot-football

FONTES = main.c opengl.c 

FONTES_SOIL = SOIL/SOIL.c SOIL/image_DXT.c SOIL/image_helper.c SOIL/stb_image_aug.c

FONTES_CM = chipmunk/chipmunk.c chipmunk/cpArbiter.c chipmunk/cpArray.c chipmunk/cpBBTree.c \
            chipmunk/cpBody.c chipmunk/cpCollision.c chipmunk/cpConstraint.c \
            chipmunk/cpDampedRotarySpring.c chipmunk/cpDampedSpring.c chipmunk/cpGearJoint.c \
            chipmunk/cpGrooveJoint.c chipmunk/cpHashSet.c chipmunk/cpHastySpace.c chipmunk/cpMarch.c \
            chipmunk/cpPinJoint.c chipmunk/cpPivotJoint.c chipmunk/cpPolyShape.c chipmunk/cpPolyline.c \
            chipmunk/cpRatchetJoint.c chipmunk/cpRobust.c chipmunk/cpRotaryLimitJoint.c \
            chipmunk/cpShape.c chipmunk/cpSimpleMotor.c chipmunk/cpSlideJoint.c chipmunk/cpSpace.c \
            chipmunk/cpSpaceComponent.c chipmunk/cpSpaceDebug.c chipmunk/cpSpaceHash.c \
            chipmunk/cpSpaceQuery.c chipmunk/cpSpaceStep.c chipmunk/cpSpatialIndex.c chipmunk/cpSweep1D.c

OBJETOS = $(FONTES:.c=.o)
OBJETOS_SOIL = $(FONTES_SOIL:.c=.o)
OBJETOS_CM = $(FONTES_CM:.c=.o)

CFLAGS = -Iinclude -Iinclude/chipmunk -Iinclude/SOIL -g -O3 -DGL_SILENCE_DEPRECATION # -Wall -g  # Todas as warnings, infos de debug
UNAME = $(shell uname)

all: $(TARGET)
	-@make $(UNAME)

Darwin: $(OBJETOS) $(OBJETOS_SOIL) $(OBJETOS_CM)
	gcc $(OBJETOS) $(OBJETOS_SOIL) $(OBJETOS_CM) -O3 -Wno-deprecated -framework OpenGL -framework Cocoa -framework GLUT -lm -o $(PROG)

Linux: $(OBJETOS) $(OBJETOS_SOIL) $(OBJETOS_CM)
	gcc $(OBJETOS) $(OBJETOS_SOIL) $(OBJETOS_CM) -O3 -lGL -lGLU -lglut -lm -o $(PROG)

clean:
	-@rm -f $(OBJETOS) $(OBJETOS_SOIL) $(OBJETOS_CM) $(PROG)
