#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h> 
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "filetobuf.h"
#include "shaderMaker.h"
#include "obj_load.h"

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);

Mesh gTank;

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  // ±Ì¿Ã πˆ∆€ √ﬂ∞°
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(width, height);
	glutCreateWindow("Tesk_20");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST); // ±Ì¿Ã ≈◊Ω∫∆Æ »∞º∫»≠

	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	if (!LoadOBJ_PosNorm_Interleaved("unit_cube.obj", gTank)) {
		std::cerr << "Failed to load unit_cube.obj\n";
		return 1;
	}

	glutMainLoop();
	return 0;
}

void DrawCube(const Mesh& mesh, GLuint shaderProgram, const glm::mat4& model, const glm::vec3& color)
{
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

	GLint colorLoc = glGetUniformLocation(shaderProgram, "vColor");
	glUniform3fv(colorLoc, 1, &color[0]);

	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, mesh.count);
	glBindVertexArray(0);
}

GLvoid drawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 8.0f);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 vTransform = glm::mat4(1.0f);
	vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

	glm::mat4 pTransform = glm::mat4(1.0f);
	pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);
	
	// æ∆∑° ∏ˆ√º
	glm::mat4 bottomBody = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	bottomBody = glm::rotate(bottomBody, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	bottomBody = glm::rotate(bottomBody, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	bottomBody = glm::scale(bottomBody, glm::vec3(3.0f, 0.5f, 1.0f));
	DrawCube(gTank, shaderProgramID, bottomBody, glm::vec3(0.5f, 0.5f, 0.5f));
	// ¡ﬂæ” ∏ˆ√º
	glm::mat4 middleBody = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, 0.0f));
	middleBody = glm::rotate(middleBody, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	middleBody = glm::rotate(middleBody, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	middleBody = glm::scale(middleBody, glm::vec3(1.5f, 0.25f, 0.5f));
	DrawCube(gTank, shaderProgramID, middleBody, glm::vec3(0.8f, 0.8f, 0.8f));
	// øﬁ¬  ¿ß ∏ˆ√º
	glm::mat4 topBody1 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, 0.7f, 0.5f));
	topBody1 = glm::rotate(topBody1, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	topBody1 = glm::rotate(topBody1, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	topBody1 = glm::scale(topBody1, glm::vec3(0.75f, 0.5f, 0.5f));
	DrawCube(gTank, shaderProgramID, topBody1, glm::vec3(0.0f, 0.8f, 0.0f));
	// ø¿∏•¬  ¿ß ∏ˆ√º
	glm::mat4 topBody2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 0.7f, 0.5f));
	topBody2 = glm::rotate(topBody2, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	topBody2 = glm::rotate(topBody2, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	topBody2 = glm::scale(topBody2, glm::vec3(0.75f, 0.5f, 0.5f));
	DrawCube(gTank, shaderProgramID, topBody2, glm::vec3(0.0f, 0.8f, 0.0f));
	// øﬁ¬  ±Í¥Î
	glm::mat4 flag1 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, 1.3f, 0.5f));
	flag1 = glm::rotate(flag1, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	flag1 = glm::rotate(flag1, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	flag1 = glm::scale(flag1, glm::vec3(0.1f, 1.0f, 0.1f));
	DrawCube(gTank, shaderProgramID, flag1, glm::vec3(0.6f, 0.3f, 0.0f));
	// ø¿∏•¬  ±Í¥Î
	glm::mat4 flag2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 1.3f, 0.5f));
	flag2 = glm::rotate(flag2, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	flag2 = glm::rotate(flag2, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	flag2 = glm::scale(flag2, glm::vec3(0.1f, 1.0f, 0.1f));
	DrawCube(gTank, shaderProgramID, flag2, glm::vec3(0.6f, 0.3f, 0.0f));

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}