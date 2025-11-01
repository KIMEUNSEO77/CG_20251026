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
float moveX = 0.0f; float moveZ = 0.0f;
// ¡ﬂæ” ∏ˆ√º y√‡ »∏¿¸
bool middleRotatingY = false;
float angleY = 0.0f;

void Timer(int value)
{
	if (middleRotatingY) angleY += 1.0f;
	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 't': middleRotatingY = !middleRotatingY; break;
	case 'q': exit(0); break;
	}
}

void SpecialKeyboard(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP: moveZ -= 0.1f; break;
	case GLUT_KEY_DOWN: moveZ += 0.1f; break;
	case GLUT_KEY_LEFT: moveX -= 0.1f; break;
	case GLUT_KEY_RIGHT: moveX += 0.1f; break;
	}
	glutPostRedisplay();
}

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
	glEnable(GL_CULL_FACE);  // ¿∫∏È ¡¶∞≈ »∞º∫»≠

	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	if (!LoadOBJ_PosNorm_Interleaved("unit_cube.obj", gTank)) {
		std::cerr << "Failed to load unit_cube.obj\n";
		return 1;
	}

	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutTimerFunc(0, Timer, 0);

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

	float offsetY = moveZ * sin(glm::radians(-15.0f));
	//float offsetX = moveZ * sin(glm::radians(15.0f));

	// πŸ¥⁄
	glm::mat4 ground = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
	// ground = glm::translate(ground, glm::vec3(moveX, 0.0f, moveZ));
	//ground = glm::rotate(ground, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ground = glm::rotate(ground, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ground = glm::scale(ground, glm::vec3(100.0f, 0.05f, 100.0f)); // ≥–∞Ì æ„¿∫ πŸ¥⁄
	DrawCube(gTank, shaderProgramID, ground, glm::vec3(1.0f, 0.713f, 0.756f));
	
	// æ∆∑° ∏ˆ√º
	glm::mat4 bottomBody = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f + offsetY, 0.0f));
	bottomBody = glm::translate(bottomBody, glm::vec3(moveX, 0.0f, moveZ));
	bottomBody = glm::rotate(bottomBody, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	bottomBody = glm::rotate(bottomBody, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	bottomBody = glm::scale(bottomBody, glm::vec3(3.0f, 0.5f, 1.0f));
	DrawCube(gTank, shaderProgramID, bottomBody, glm::vec3(0.678f, 0.847f, 0.902f));

	// ¡ﬂæ” ∏ˆ√º
	glm::mat4 middleBody = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f + offsetY, 0.0f));
	middleBody = glm::translate(middleBody, glm::vec3(moveX, 0.0f, moveZ));
	middleBody = glm::rotate(middleBody, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	middleBody = glm::rotate(middleBody, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	middleBody = glm::rotate(middleBody, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	middleBody = glm::scale(middleBody, glm::vec3(1.5f, 0.25f, 0.5f));
	DrawCube(gTank, shaderProgramID, middleBody, glm::vec3(0.564f, 0.933f, 0.564f));

	// øﬁ¬  ¿ß ∏ˆ√º
	glm::mat4 topBody1 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, 0.7f + offsetY, 0.5f));
	topBody1 = glm::translate(topBody1, glm::vec3(moveX, 0.0f, moveZ));
	topBody1 = glm::rotate(topBody1, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	topBody1 = glm::rotate(topBody1, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	topBody1 = glm::rotate(topBody1, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	topBody1 = glm::scale(topBody1, glm::vec3(0.75f, 0.5f, 0.5f));
	DrawCube(gTank, shaderProgramID, topBody1, glm::vec3(0.784f, 0.635f, 0.784f));
	// ø¿∏•¬  ¿ß ∏ˆ√º
	glm::mat4 topBody2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 0.7f + offsetY, 0.5f));
	topBody2 = glm::translate(topBody2, glm::vec3(moveX, 0.0f, moveZ));
	topBody2 = glm::rotate(topBody2, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	topBody2 = glm::rotate(topBody2, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	topBody2 = glm::rotate(topBody2, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	topBody2 = glm::scale(topBody2, glm::vec3(0.75f, 0.5f, 0.5f));
	DrawCube(gTank, shaderProgramID, topBody2, glm::vec3(0.784f, 0.635f, 0.784f));
	// øﬁ¬  ±Í¥Î
	glm::mat4 flag1 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, 1.3f + offsetY, 0.5f));
	flag1 = glm::translate(flag1, glm::vec3(moveX, 0.0f, moveZ));
	flag1 = glm::rotate(flag1, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	flag1 = glm::rotate(flag1, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	flag1 = glm::scale(flag1, glm::vec3(0.1f, 1.0f, 0.1f));
	DrawCube(gTank, shaderProgramID, flag1, glm::vec3(1.0f, 0.7f, 0.3f));
	// ø¿∏•¬  ±Í¥Î
	glm::mat4 flag2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 1.3f + offsetY, 0.5f));
	flag2 = glm::translate(flag2, glm::vec3(moveX, 0.0f, moveZ));
	flag2 = glm::rotate(flag2, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	flag2 = glm::rotate(flag2, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	flag2 = glm::scale(flag2, glm::vec3(0.1f, 1.0f, 0.1f));
	DrawCube(gTank, shaderProgramID, flag2, glm::vec3(1.0f, 0.7f, 0.3f));
	// øﬁ¬  ∆˜Ω≈
	glm::mat4 barrel1 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, 0.5f + offsetY, 1.5f));
	barrel1 = glm::translate(barrel1, glm::vec3(moveX, 0.0f, moveZ));
	barrel1 = glm::rotate(barrel1, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	barrel1 = glm::rotate(barrel1, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	barrel1 = glm::scale(barrel1, glm::vec3(0.1f, 0.1f, 1.0f));
	DrawCube(gTank, shaderProgramID, barrel1, glm::vec3(0.5f, 0.0f, 0.5f));
	// ø¿∏•¬  ∆˜Ω≈
	glm::mat4 barrel2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 0.5f + offsetY, 1.5f));
	barrel2 = glm::translate(barrel2, glm::vec3(moveX, 0.0f, moveZ));
	barrel2 = glm::rotate(barrel2, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	barrel2 = glm::rotate(barrel2, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	barrel2 = glm::scale(barrel2, glm::vec3(0.1f, 0.1f, 1.0f));
	DrawCube(gTank, shaderProgramID, barrel2, glm::vec3(0.5f, 0.0f, 0.5f));

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}