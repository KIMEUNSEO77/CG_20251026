#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h> 
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "filetobuf.h"
#include "shaderMaker.h"
#include "obj_load.h"

#include <chrono>

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);

Mesh gTank;
float moveX = 0.0f; float moveZ = 0.0f;
bool middleRotatingY = false;
float angleY = 0.0f;

bool rotatingCameraZ = false;
float angleCameraZ = 0.0f;
int dirCameraZ = 1;   // 1: 양, -1: 음

bool rotatingCameraX = false;
float angleCameraX = 0.0f;
int dirCameraX = 1;   // 1: 양, -1: 음

bool rotatingCameraY = false;
float angleCameraY = 0.0f;

bool rotatingCameraCenterY = false;
float angleCameraCenterY = 0.0f;

bool rotatingBarel = false;  // 포신 회전
float angleBarel1 = 0.0f;    // 왼쪽 포신 각도
float angleBarel2 = 0.0f;    // 오른쪽 포신 각도

void Timer(int value)
{
	if (middleRotatingY) angleY += 1.0f;
	if (rotatingCameraZ) angleCameraZ += dirCameraZ * 2.0f;
	if (rotatingCameraX) angleCameraX += dirCameraX * 2.0f;
	if (rotatingCameraY) angleCameraY += 2.0f;
	if (rotatingCameraCenterY) angleCameraCenterY += 2.0f;
	if (rotatingBarel) 
	{
		angleBarel1 += 2.0f;
		angleBarel2 -= 2.0f;
	}

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}

void StopAllRotations()
{
	middleRotatingY = false;
	rotatingCameraZ = false;
	rotatingCameraX = false;
	rotatingCameraY = false;
	rotatingCameraCenterY = false;

	glutPostRedisplay();
}


void Reset()
{
	middleRotatingY = false;
	angleY = 0.0f;
	rotatingCameraZ = false;
	angleCameraZ = 0.0f;
	rotatingCameraX = false;
	angleCameraX = 0.0f;
	rotatingCameraY = false;
	angleCameraY = 0.0f;
	rotatingCameraCenterY = false;
	angleCameraCenterY = 0.0f;
	moveX = 0.0f; moveZ = 0.0f;

	glutPostRedisplay();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 't': middleRotatingY = !middleRotatingY; break;
	case 'z': rotatingCameraZ = !rotatingCameraZ; dirCameraZ = 1; rotatingCameraX = false; rotatingCameraY = false; break;
	case 'Z': rotatingCameraZ = !rotatingCameraZ; dirCameraZ = -1; rotatingCameraX = false; rotatingCameraY = false; break;
	case 'x': rotatingCameraX = !rotatingCameraX; dirCameraX = 1; rotatingCameraZ = false; rotatingCameraY = false; break;
	case 'X': rotatingCameraX = !rotatingCameraX; dirCameraX = -1; rotatingCameraZ = false; rotatingCameraY = false; break;
	case 'y': rotatingCameraY = !rotatingCameraY; rotatingCameraX = false; rotatingCameraZ = false; break;
	case 'r': rotatingCameraCenterY = !rotatingCameraCenterY; rotatingCameraX = false; rotatingCameraZ = false; rotatingCameraY = false; break;
	case 'g': rotatingBarel = !rotatingBarel; break;
	case 'o': StopAllRotations(); break;
	case 'c': Reset(); break;
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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  // 깊이 버퍼 추가
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(width, height);
	glutCreateWindow("Tesk_20");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
	glEnable(GL_CULL_FACE);  // 은면 제거 활성화

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

	//glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 8.0f);
	//glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	//glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float radZ = glm::radians(angleCameraZ);
	float radX = glm::radians(angleCameraX);
	float radY = glm::radians(angleCameraY);
	float radiusCenterY = 8.0f;   // 공전 반지름
	float radCenterY = glm::radians(angleCameraCenterY);

	glm::vec3 cameraPos, cameraTarget, cameraUp;

	if (rotatingCameraCenterY) 
	{
		cameraPos = glm::vec3(
			radiusCenterY * sin(radCenterY),
			0.0f,
			radiusCenterY * cos(radCenterY)
		);
		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	else 
	{
		cameraPos = glm::vec3(0.0f, 0.0f, radiusCenterY);
		glm::vec3 cameraDirection = glm::vec3(
			-sin(radY) * cos(radX),
			-sin(radX),
			-cos(radY) * cos(radX)
		);
		cameraTarget = cameraPos + cameraDirection;
	}

		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 dir = glm::normalize(cameraTarget - cameraPos);
		glm::mat4 rollMat = glm::rotate(glm::mat4(1.0f), radZ, dir);
		cameraUp = glm::vec3(rollMat * glm::vec4(cameraUp, 0.0f));

	glm::mat4 vTransform = glm::mat4(1.0f);
	vTransform = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

	glm::mat4 pTransform = glm::mat4(1.0f);
	pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);

	float offsetY = moveZ * sin(glm::radians(-15.0f));

	// 바닥
	glm::mat4 ground = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
	ground = glm::rotate(ground, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ground = glm::scale(ground, glm::vec3(100.0f, 0.05f, 100.0f)); // 넓고 얇은 바닥
	DrawCube(gTank, shaderProgramID, ground, glm::vec3(1.0f, 0.713f, 0.756f));
	
    // 공통: 탱크의 월드 위치/기울기 (부모 변환)
	glm::mat4 M_tank = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, offsetY, 0.0f));
	M_tank = glm::translate(M_tank, glm::vec3(moveX, 0.0f, moveZ));
	M_tank = glm::rotate(M_tank, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	M_tank = glm::rotate(M_tank, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// 1) 아래 몸체: 부모만 따름
	glm::mat4 bottomBody = M_tank * glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 0.5f, 1.0f));
	DrawCube(gTank, shaderProgramID, bottomBody, glm::vec3(0.678f, 0.847f, 0.902f));

	// 2) 포탑(중앙 몸체) 베이스: 부모 + 포탑 위치 + 포탑 회전
	//    (여기에 달린 모든 자식은 angleY를 같이 받음)
	glm::mat4 M_turret = M_tank;
	M_turret = glm::translate(M_turret, glm::vec3(0.0f, 0.4f, 0.0f));
	M_turret = glm::rotate(M_turret, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));

	// 중앙 몸체
	glm::mat4 middleBody = M_turret * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 0.25f, 0.5f));
	DrawCube(gTank, shaderProgramID, middleBody, glm::vec3(0.564f, 0.933f, 0.564f));

	// 3) 상단 좌/우 몸체
	glm::mat4 M_top1 = M_turret * glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, 0.4f, 0.0f));
	glm::mat4 topBody1 = M_top1 * glm::scale(glm::mat4(1.0f), glm::vec3(0.75f, 0.5f, 0.5f));
	DrawCube(gTank, shaderProgramID, topBody1, glm::vec3(0.784f, 0.635f, 0.784f));

	glm::mat4 M_top2 = M_turret * glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 0.4f, 0.0f));
	glm::mat4 topBody2 = M_top2 * glm::scale(glm::mat4(1.0f), glm::vec3(0.75f, 0.5f, 0.5f));
	DrawCube(gTank, shaderProgramID, topBody2, glm::vec3(0.784f, 0.635f, 0.784f));

	// 4) 깃대: 각각 상단 몸체의 자식으로 (topBody 기준)
	glm::mat4 flag1 = M_top1
		* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.6f, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 1.0f, 0.1f));
	DrawCube(gTank, shaderProgramID, flag1, glm::vec3(1.0f, 0.7f, 0.3f));

	glm::mat4 flag2 = M_top2
		* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.6f, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 1.0f, 0.1f));
	DrawCube(gTank, shaderProgramID, flag2, glm::vec3(1.0f, 0.7f, 0.3f));

	// 5) 포신: 각각 상단 몸체의 자식으로
	glm::mat4 barrel1 = M_top1
		* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.2f, 0.5f))
		* glm::rotate(glm::mat4(1.0f), glm::radians(angleBarel1), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 1.0f));
	DrawCube(gTank, shaderProgramID, barrel1, glm::vec3(0.5f, 0.0f, 0.5f));

	glm::mat4 barrel2 = M_top2
		* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.2f, 0.5f))
		* glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 1.0f));
	DrawCube(gTank, shaderProgramID, barrel2, glm::vec3(0.5f, 0.0f, 0.5f));

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}