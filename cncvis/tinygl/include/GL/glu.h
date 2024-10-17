
#ifndef GLU_H
#define GLU_H
#ifdef __cplusplus
extern "C" {
#endif

#include "gl.h"

void gluPerspective(GLfloat fovy, GLfloat aspect,
				GLfloat zNear, GLfloat zFar);

void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
				GLfloat centerx, GLfloat centery, GLfloat centerz,
				GLfloat upx, GLfloat upy, GLfloat upz);

typedef struct
{
	int draw_style;
} GLUquadricObj;

#define GLU_LINE 0

GLUquadricObj *gluNewQuadric(void);
void gluQuadricDrawStyle(GLUquadricObj *obj, int style);

void gluSphere(GLUquadricObj *qobj,
				float radius,int slices,int stacks);
void gluCylinder(GLUquadricObj *qobj,
				GLdouble baseRadius, GLdouble topRadius, GLdouble height,
				GLint slices, GLint stacks);
void gluDisk(GLUquadricObj *qobj,
				GLdouble innerRadius, GLdouble outerRadius,
				GLint slices, GLint loops);

void drawTorus(float rc, int numc, float rt, int numt);

#ifdef __cplusplus
}
#endif
#endif /* GLU_H */
