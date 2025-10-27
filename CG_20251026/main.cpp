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

Mesh gSphere;   // 중심의 구
float angle_1 = 0.0f;
float angle_2 = 0.0f;
float angle_3 = 0.0f;

bool orthoMode = false;  // false: 원근 투영, true: 직교 투영
bool wireMode = false;   // false: 솔리드 모드, true: 와이어 모드

float currentRadius = 3.0f;
float moonRadius = 1.0f;

float moveX = 0.0f;
float moveY = 0.0f;
float moveZ = 0.0f;
float moveStep = 0.1f;

float angleZ = 0.0f;
bool rotatingZ = false;  // z축 회전 여부
int dirZ = 1;        // z축 회전 방향 1: 양, -1: 음

void Timer(int value)
{
	angle_1 += 1.0f;
	angle_2 += 2.0f;
	angle_3 += 3.0f;

	if (rotatingZ) angleZ += dirZ * 2.0f;
	
	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}

// 와이어 모드
void SetWireMode(bool mode)
{
	if (mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glutPostRedisplay();
}

// 반지름 증가, 감소
void IncreaseRadius(float delta)
{
	currentRadius += delta;
	if (currentRadius <= 1.0f) currentRadius = 1.0f;
	if (currentRadius >= 5.0f) currentRadius = 5.0f;
	glutPostRedisplay();
}
void IncreaseMoonRadius(float delta)
{
	moonRadius += delta;
	if (moonRadius <= 0.4f) moonRadius = 0.4f;
	if (moonRadius >= 1.6f) moonRadius = 1.6f;
	glutPostRedisplay();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'p': orthoMode = !orthoMode; break;
	case 'm': wireMode = !wireMode;  SetWireMode(wireMode); break;
	case 'w': moveY += moveStep; glutPostRedisplay(); break;
	case 'a': moveX -= moveStep; glutPostRedisplay(); break;
	case 's':  moveY -= moveStep; glutPostRedisplay(); break;
	case 'd': moveX += moveStep; glutPostRedisplay(); break;
	case '+': moveZ += moveStep; glutPostRedisplay(); break;
	case '-': moveZ -= moveStep; glutPostRedisplay(); break;
	case 'y': IncreaseRadius(-0.5f); IncreaseMoonRadius(-0.2f); break;
	case 'Y': IncreaseRadius(0.5f); IncreaseMoonRadius(0.2f); break;
	case 'z': rotatingZ = !rotatingZ; dirZ = 1; break;
	case 'Z': dirZ = -1; break;

	case 'q':
		exit(0);
		break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  // 깊이 버퍼 추가
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Tesk_19");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
	glEnable(GL_CULL_FACE);  // 은면 제거 활성화

	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();
	InitOrbitMesh();

	// obj 파일 로드
	if (!LoadOBJ_PosNorm_Interleaved("sphere.obj", gSphere)) 
	{
		std::cerr << "Failed to load sphere.obj\n";
		return 1;
	}

	glutKeyboardFunc(Keyboard);
	glutTimerFunc(0, Timer, 0);

	glutMainLoop();
	return 0;
}

// 궤도 그리는 함수
void DrawOrbit(GLuint shaderProgramID,
	float rx, float rz,
	const glm::vec3& center,
	float angle,
	const glm::vec3& color)
{
	glm::mat4 M(1.0f);
	M = glm::translate(M, center);                         // 위치
	M = glm::rotate(M, glm::radians(angle), { 1,0,1 });  // x, z축 기울이기
	M = glm::rotate(M, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f)); // z축 회전
	M = glm::scale(M, glm::vec3(rx, 1.0f, rz));            // 타원 크기

	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &M[0][0]);

	GLint colorLoc = glGetUniformLocation(shaderProgramID, "vColor");
	if (colorLoc >= 0) glUniform3fv(colorLoc, 1, &color[0]);  // 선 색상(셰이더에 따라)

	glLineWidth(1.0f);
	glBindVertexArray(orbitVAO);
	glDrawArrays(GL_LINE_LOOP, 0, ORBIT_SEG);
	glBindVertexArray(0);
}

// 구 그리는 함수
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
	if (orthoMode)
	{
		pTransform = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 0.1f, 100.0f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);
	}
	else
	{
		pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);
	}

	// 중심 구
	glm::mat4 centerM = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	centerM = glm::scale(centerM, glm::vec3(2.0f, 2.0f, 2.0f));
	DrawSphere(gSphere, shaderProgramID, centerM, glm::vec3(0.8f, 0.8f, 0.8f));

	// 궤도 그리기
	glm::vec3 center(0, 0, 0);
	center.x += moveX;
	center.y += moveY;
	center.z += moveZ;
	DrawOrbit(shaderProgramID, currentRadius, currentRadius, center, 2.0f, { 0.0f, 0.0f, 0.0f });
	DrawOrbit(shaderProgramID, currentRadius, currentRadius, center, 45.0f, { 0.0f, 0.0f, 0.0f });
	DrawOrbit(shaderProgramID, currentRadius, currentRadius, center, -45.0f, { 0.0f, 0.0f, 0.0f });

	glm::mat4 m1 = glm::translate(glm::mat4(1.0f), glm::vec3(center));
	m1 = glm::rotate(m1, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f)); // z축 회전 먼저
	m1 = glm::rotate(m1, glm::radians(angle_1), glm::vec3(0.0f, 1.0f, 0.0f));
	m1 = glm::translate(m1, glm::vec3(currentRadius, 0.0f, 0.0f));
	DrawSphere(gSphere, shaderProgramID, m1, glm::vec3(0.8f, 0.8f, 0.0f));

	glm::mat4 m2 = glm::translate(glm::mat4(1.0f), glm::vec3(center));
	m2 = glm::rotate(m2, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 1.0f));
	m2 = glm::rotate(m2, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f));
	m2 = glm::rotate(m2, glm::radians(angle_2), glm::vec3(0.0f, 1.0f, 0.0f));
	m2 = glm::translate(m2, glm::vec3(-currentRadius, 0.0f, 0.0f));
	DrawSphere(gSphere, shaderProgramID, m2, glm::vec3(0.0f, 0.8f, 0.8f));

	glm::mat4 m3 = glm::translate(glm::mat4(1.0f), glm::vec3(center));
	m3 = glm::rotate(m3, glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 1.0f));
	m3 = glm::rotate(m3, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f));
	m3 = glm::rotate(m3, glm::radians(angle_3), glm::vec3(0.0f, 1.0f, 0.0f));
	m3 = glm::translate(m3, glm::vec3(currentRadius, 0.0f, 0.0f));
	DrawSphere(gSphere, shaderProgramID, m3, glm::vec3(0.8f, 0.0f, 0.8f));

	glm::vec3 m1Center = glm::vec3(m1[3]);   // x, y, z 위치 추출
	glm::vec3 m2Center = glm::vec3(m2[3]);
	glm::vec3 m3Center = glm::vec3(m3[3]);
	// 작은 구 궤도 그리기
	DrawOrbit(shaderProgramID, moonRadius, moonRadius, m1Center, 2.0, { 0.0f, 0.0f, 0.0f });
	DrawOrbit(shaderProgramID, moonRadius, moonRadius, m2Center, 45.0f, { 0.0f, 0.0f, 0.0f });
	DrawOrbit(shaderProgramID, moonRadius, moonRadius, m3Center, -45.0f, { 0.0f, 0.0f, 0.0f });

	glm::mat4 d1 = glm::mat4(1.0f);
	d1 = glm::translate(d1, m1Center);
	d1 = glm::rotate(d1, glm::radians(angle_3), glm::vec3(0.0f, 1.0f, 0.0f));
	d1 = glm::rotate(d1, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f));
	d1 = glm::translate(d1, glm::vec3(moonRadius, 0.0f, 0.0f));
	d1 = glm::scale(d1, glm::vec3(0.3f, 0.3f, 0.3f));
	DrawSphere(gSphere, shaderProgramID, d1, glm::vec3(1.0f, 1.0f, 0.0f));

	glm::mat4 d2 = glm::mat4(1.0f);
	d2 = glm::translate(d2, m2Center);
	d2 = glm::rotate(d2, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 1.0f));
	d2 = glm::rotate(d2, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f));
	d2 = glm::rotate(d2, glm::radians(angle_3), glm::vec3(0.0f, 1.0f, 0.0f));
	d2 = glm::translate(d2, glm::vec3(-moonRadius, 0.0f, 0.0f));
	d2 = glm::scale(d2, glm::vec3(0.3f, 0.3f, 0.3f));
	DrawSphere(gSphere, shaderProgramID, d2, glm::vec3(0.0f, 0.5f, 0.5f));

	glm::mat4 d3 = glm::mat4(1.0f);
	d3 = glm::translate(d3, m3Center);
	d3 = glm::rotate(d3, glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 1.0f));
	d3 = glm::rotate(d3, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f));
	d3 = glm::rotate(d3, glm::radians(angle_3), glm::vec3(0.0f, 1.0f, 0.0f));
	d3 = glm::translate(d3, glm::vec3(moonRadius, 0.0f, 0.0f));
	d3 = glm::scale(d3, glm::vec3(0.3f, 0.3f, 0.3f));
	DrawSphere(gSphere, shaderProgramID, d3, glm::vec3(0.5f, 0.0f, 0.5f));


	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}