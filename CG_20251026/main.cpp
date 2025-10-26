#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h> 

#include "filetobuf.h"
#include "shaderMaker.h"
#include "obj_load.h"

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);

Mesh gSphere;   // �߽��� ��

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		exit(0);
		break;
	}
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  // ���� ���� �߰�
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Tesk_19");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST); // ���� �׽�Ʈ Ȱ��ȭ
	glEnable(GL_CULL_FACE);  // ���� ���� Ȱ��ȭ

	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	// obj ���� �ε�
	if (!LoadOBJ_PosNorm_Interleaved("sphere.obj", gSphere)) 
	{
		std::cerr << "Failed to load sphere.obj\n";
		return 1;
	}

	glutKeyboardFunc(Keyboard);
	glutMainLoop();
	return 0;
}

// �� �׸��� �Լ�
void DrawSphere(const Mesh& mesh, GLuint shaderProgram, const glm::mat4& model, const glm::vec3& color)
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

	// �߽� ��
	glm::mat4 center = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	center = glm::scale(center, glm::vec3(2.0f, 2.0f, 2.0f));
	DrawSphere(gSphere, shaderProgramID, center, glm::vec3(0.8f, 0.8f, 0.8f));

	// ������ ��
	glm::mat4 m1 = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
	DrawSphere(gSphere, shaderProgramID, m1, glm::vec3(0.8f, 0.8f, 0.0f));

	// ���� �Ʒ� ��
	glm::mat4 m2 = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, -2.0f, 0.0f));
	DrawSphere(gSphere, shaderProgramID, m2, glm::vec3(0.0f, 0.8f, 0.8f));

	// ������ �Ʒ� ��
	glm::mat4 m3 = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -2.0f, 0.0f));
	DrawSphere(gSphere, shaderProgramID, m3, glm::vec3(0.8f, 0.0f, 0.8f));

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}