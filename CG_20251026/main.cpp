#include <iostream>
#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h> 
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "filetobuf.h"
#include "shaderMaker.h"

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Tesk_19");

	glewExperimental = GL_TRUE;
	glewInit();

	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}

GLvoid drawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}