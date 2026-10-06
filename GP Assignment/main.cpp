#include <d3dx9math.h>
#include <Windows.h>
#include <hidusage.h>
#include <gl/GL.h>
#include <math.h>
#include <iostream>
#include <GL/glu.h>
#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "GLU32.lib")
#pragma comment (lib,"d3dx9.lib")
#define WINDOW_TITLE "OpenGL Window"

using namespace std;

// ----- Camera Viewport ----- //CHANGE WINDOW SIZE HERE ADD VARIABLE
struct Camera {
	D3DXVECTOR3 position;
	float yaw, pitch;
};
void Camera_Look(Camera& c, float yawInput, float pitchInput) {
	c.yaw += yawInput;
	c.pitch += pitchInput;

	if (c.pitch > 0.5f * 3.142f) {
		c.pitch = 0.5f * 3.142f - 0.1f;
	}
	else if (c.pitch < -0.5f * 3.142f) {
		c.pitch = -0.5f * 3.142f + 0.1f;
	}
}
void Camera_Move(Camera& c, int xInput, int yInput, int zInput, float rate) {
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);

	D3DXMatrixRotationX(&mat, c.pitch);
	D3DXMatrixRotationY(&mat, -c.yaw);

	D3DXVECTOR3 input = { (FLOAT)xInput, (FLOAT)yInput, (FLOAT)zInput };

	D3DXVec3TransformCoord(&input, &input, &mat);

	input *= rate;

	c.position += input;
}
Camera camera = {
};
int environment3DCamCTRL = 0;
void Environment3DCam() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
		60.0,             // Field of view angle in y-direction
		(GLfloat)800 / 600, // Aspect ratio of the viewport
		0.001,              // Near clipping plane - Ignore objects closer than ?
		300.0             // Far clipping plane - Ignore objects farther than ?
	);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(camera.pitch * 180 / 3.142f, 1, 0, 0);
	glRotatef(camera.yaw * 180 / 3.142f, 0, 1, 0);
	glTranslatef(-camera.position.x, -camera.position.y, -camera.position.z);
	glTranslatef(-0.0f, -0.0f, -3.0f);
}

float x = 0;
float y = 0;
float z = 0.0f;
float xRotate = 0;
float yRotate = 0;
float zRotate = 0;
float radius = 0;

float rotateSpeed = 0.01f;

const char* texturePaths[] = {
	"metal-black.bmp",
	"metal-yellow-rusty.bmp",
	"glass.bmp",
};
#define TEXTURE_COUNT (sizeof(texturePaths) / sizeof(const char*))
GLuint textures[TEXTURE_COUNT];

#define METAL_BLACK textures[0]
#define METAL_YELLOW_RUSTY textures[1]
#define GLASS textures[2]

GLUquadricObj* quad;
// Initialization Func
void GL3DInitialization() {
	quad = gluNewQuadric();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);	//Defaulted, Ensure Close Obj properly obscure far obj

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glGenTextures(TEXTURE_COUNT, textures);
	for (int i = 0; i < TEXTURE_COUNT; i++) {
		BITMAP BMP;
		HBITMAP hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL), texturePaths[i], IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
		GetObject(hBMP, sizeof(BMP), &BMP);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);
		DeleteObject(hBMP);
	}

	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
}

// Typedef Structs
typedef struct SphereInfo {
	GLenum drawStyle;  // GLU_FILL or GLU_LINE
	float r, g, b;
	float radius;
	int slices;
	int stacks;
} SphereInfo;
typedef struct CylinderInfo {
	GLenum drawStyle;  // GLU_FILL or GLU_LINE
	float r, g, b;
	float radius;
	float height;
	int slices;
	int stacks;
} CylinderInfo;
typedef struct ConeInfo {
	GLenum drawStyle;  // GLU_FILL or GLU_LINE
	float r, g, b;
	float radius;
	float height;
	int slices;
	int stacks;
} ConeInfo;

// Draw Func
void DrawCube(float width, float height, float depth)
{
	float w = width / 2;
	float h = height / 2;
	float d = depth / 2;

	glBegin(GL_QUADS);

	float ambientAndDiffuse[] = { 1, 1, 1, 1 };
	glMaterialfv(GL_FRONT_FACE, GL_AMBIENT_AND_DIFFUSE, ambientAndDiffuse);

	// Front face
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0); glVertex3f(-w, -h, d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, d);
	glTexCoord2f(1, 1); glVertex3f(w, h, d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, d);
	// Back face
	glNormal3f(0, 0, -1);
	glTexCoord2f(0, 0); glVertex3f(-w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(-w, h, -d);
	glTexCoord2f(1, 1); glVertex3f(w, h, -d);
	glTexCoord2f(0, 1); glVertex3f(w, -h, -d);
	// Left face
	glNormal3f(0, -1, 0);
	glTexCoord2f(0, 0); glVertex3f(-w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(-w, -h, d);
	glTexCoord2f(1, 1); glVertex3f(-w, h, d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, -d);
	// Right face
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 0); glVertex3f(w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(w, h, -d);
	glTexCoord2f(1, 1); glVertex3f(w, h, d);
	glTexCoord2f(0, 1); glVertex3f(w, -h, d);
	// Top face
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(-w, h, -d);
	glTexCoord2f(1, 0); glVertex3f(-w, h, d);
	glTexCoord2f(1, 1); glVertex3f(w, h, d);
	glTexCoord2f(0, 1); glVertex3f(w, h, -d);
	// Bottom face
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(-w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, -d);
	glTexCoord2f(1, 1); glVertex3f(w, -h, d);
	glTexCoord2f(0, 1); glVertex3f(-w, -h, d);
	glEnd();
}
void DrawSphere(const SphereInfo* sphere)
{

	gluQuadricNormals(quad, GLU_SMOOTH);
	gluQuadricTexture(quad, GL_FRONT_FACE);
	gluQuadricDrawStyle(quad, sphere->drawStyle);

	glColor3f(sphere->r, sphere->g, sphere->b);

	gluSphere(quad,
		sphere->radius,
		sphere->slices,
		sphere->stacks);

}
void DrawCylinder(const CylinderInfo* cylinder)
{
	gluQuadricNormals(quad, GLU_SMOOTH);
	gluQuadricTexture(quad, GL_FRONT_FACE);
	gluQuadricDrawStyle(quad, cylinder->drawStyle);

	glColor3f(cylinder->r, cylinder->g, cylinder->b);

	gluCylinder(quad,
		cylinder->radius,
		cylinder->radius,
		cylinder->height,
		cylinder->slices,
		cylinder->stacks);

	// TOP
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, cylinder->height);
	gluDisk(quad, 0, cylinder->radius, cylinder->slices, cylinder->stacks);
	glPopMatrix();

	// BOT
	glPushMatrix();
	gluDisk(quad, 0, cylinder->radius, cylinder->slices, cylinder->stacks);
	glPopMatrix();

}
void DrawCone(const ConeInfo* cone)
{
	gluQuadricNormals(quad, GLU_SMOOTH);
	gluQuadricTexture(quad, GL_FRONT_FACE);
	gluQuadricDrawStyle(quad, cone->drawStyle);

	glColor3f(cone->r, cone->g, cone->b);

	gluCylinder(quad,
		cone->radius,
		0.0f,             // tip
		cone->height,
		cone->slices,
		cone->stacks);

	// CAP
	glPushMatrix();
	gluDisk(quad, 0.0f, cone->radius, cone->slices, cone->stacks);
	glPopMatrix();

}
void DrawTrapeziumQuad(float ht, float hb)
{
	float topY = 0.5f;
	float bottomY = -0.5f;
	float thickness = 0.3f;

	glBegin(GL_QUADS);

	float ambientAndDiffuse[] = { 1, 1, 1, 1 };
	glMaterialfv(GL_FRONT_FACE, GL_AMBIENT_AND_DIFFUSE, ambientAndDiffuse);

	// ================= FRONT FACE =================
	// Visible trapezium from forward POV
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0);  glVertex3f(-ht, topY, 0.0f);
	glTexCoord2f(1, 0);  glVertex3f(ht, topY, 0.0f);
	glTexCoord2f(1, 1);  glVertex3f(hb, bottomY, 0.0f);
	glTexCoord2f(0, 1);  glVertex3f(-hb, bottomY, 0.0f);

	// ================= BACK FACE =================
	glNormal3f(0, 0, -1);
	glTexCoord2f(0, 0);  glVertex3f(-ht, topY, -thickness);
	glTexCoord2f(1, 0);  glVertex3f(-hb, bottomY, -thickness);
	glTexCoord2f(1, 1);  glVertex3f(hb, bottomY, -thickness);
	glTexCoord2f(0, 1);  glVertex3f(ht, topY, -thickness);

	// ================= LEFT FACE =================
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0, 0);  glVertex3f(-ht, topY, 0.0f);
	glTexCoord2f(1, 0);  glVertex3f(-ht, topY, -thickness);
	glTexCoord2f(1, 1);  glVertex3f(-hb, bottomY, -thickness);
	glTexCoord2f(0, 1);  glVertex3f(-hb, bottomY, 0.0f);

	// ================= RIGHT FACE =================
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 0);  glVertex3f(ht, topY, 0.0f);
	glTexCoord2f(1, 0);  glVertex3f(hb, bottomY, 0.0f);
	glTexCoord2f(1, 1);  glVertex3f(hb, bottomY, -thickness);
	glTexCoord2f(0, 1);  glVertex3f(ht, topY, -thickness);

	// ================= TOP FACE =================
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 0);  glVertex3f(-ht, topY, 0.0f);
	glTexCoord2f(1, 0);  glVertex3f(ht, topY, 0.0f);
	glTexCoord2f(1, 1);  glVertex3f(ht, topY, -thickness);
	glTexCoord2f(0, 1);  glVertex3f(-ht, topY, -thickness);

	// ================= BOTTOM FACE =================
	glNormal3f(0, -1, 0);
	glTexCoord2f(0, 0);  glVertex3f(-hb, bottomY, 0.0f);
	glTexCoord2f(1, 0);  glVertex3f(hb, bottomY, 0.0f);
	glTexCoord2f(1, 1);  glVertex3f(hb, bottomY, -thickness);
	glTexCoord2f(0, 1);  glVertex3f(-hb, bottomY, -thickness);

	glEnd();
}

void DrawRing(float radius, float thickness, int segments)
{
	glBegin(GL_QUAD_STRIP);

	float ambientAndDiffuse[] = { 1, 1, 1, 1 };
	glMaterialfv(GL_FRONT_FACE, GL_AMBIENT_AND_DIFFUSE, ambientAndDiffuse);

	for (int i = 0; i <= segments; ++i)
	{
		float angle = i * 2.0f * 3.1415926f / segments;
		float x = cos(angle) * radius;
		float y = sin(angle) * radius;

		glVertex3f(x, y, thickness / 2);
		glVertex3f(x, y, -thickness / 2);
	}
	glEnd();
}
void DrawOctagon3D(float radius = 1.0f, float depth = 0.3f)
{
	const int sides = 8;
	float angleStep = 2.0f * 3.1415926f / sides;

	glBegin(GL_QUADS);

	float ambientAndDiffuse[] = { 1, 1, 1, 1 };
	glMaterialfv(GL_FRONT_FACE, GL_AMBIENT_AND_DIFFUSE, ambientAndDiffuse);

	for (int i = 0; i < sides; i++)
	{
		float a0 = i * angleStep;
		float a1 = (i + 1) * angleStep;

		float x0 = radius * cos(a0);
		float y0 = radius * sin(a0);

		float x1 = radius * cos(a1);
		float y1 = radius * sin(a1);

		// side walls
		glColor3f(0.6f, 0.1f, 0.1f);
		glVertex3f(x0, y0, depth);
		glVertex3f(x1, y1, depth);
		glVertex3f(x1, y1, -depth);
		glVertex3f(x0, y0, -depth);
	}
	glEnd();

	// top
	glBegin(GL_POLYGON);
	glMaterialfv(GL_FRONT_FACE, GL_AMBIENT_AND_DIFFUSE, ambientAndDiffuse);
	glColor3f(0.8f, 0.2f, 0.2f);
	for (int i = 0; i < sides; i++)
	{
		float a = i * angleStep;
		glVertex3f(radius * cos(a), radius * sin(a), depth);
	}
	glEnd();

	// bottom
	glBegin(GL_POLYGON);
	glMaterialfv(GL_FRONT_FACE, GL_AMBIENT_AND_DIFFUSE, ambientAndDiffuse);
	glColor3f(0.3f, 0.05f, 0.05f);
	for (int i = 0; i < sides; i++)
	{
		float a = i * angleStep;
		glVertex3f(radius * cos(a), radius * sin(a), -depth);
	}
	glEnd();
}
void DrawGyroscope()
{
	glPushMatrix();

	// Base stand (cube)
	glColor3f(0.3f, 0.3f, 0.3f);
	glPushMatrix();
	glScalef(0.1f, 0.1f, 0.5f); // width, height, depth
	DrawCube(1.0f, 1.0f, 1.0f);
	glPopMatrix();

	// Outer gimbal (yaw)
	static float yaw = 0;
	yaw += 0.1f;
	if (yaw > 360) yaw -= 360;
	glRotatef(yaw, 0, 1, 0);
	glColor3f(1, 0, 0);
	DrawRing(1.0f, 0.05f, 64);

	// Middle gimbal (pitch)
	static float pitch = 0;
	pitch += 0.1f;
	if (pitch > 360) pitch -= 360;
	glRotatef(pitch, 1, 0, 0);
	glColor3f(0, 1, 0);
	DrawRing(0.7f, 0.05f, 64);

	// Inner gimbal (roll)
	static float roll = 0;
	roll += 0.1f;
	if (roll > 360) roll -= 360;
	glRotatef(roll, 0, 0, 1);
	glColor3f(0, 0, 1);
	DrawRing(0.5f, 0.1f, 64);

	glPopMatrix();
}

float previousTime = 0;

float gyroYaw = 0;
float gyroPitch = 0;
float gyroRoll = 0;

// Assg Helper Tools
int movePosCTRL = 0;
int shapeChangeCTRL = 0;

// UNUSED ------------------>
/*
	//COCKPILOT VIEW

void DrawRobotUpperLeftLegSkeleton() {
	glLoadIdentity();
	glRotatef(90.0f, 1.0f, 0, 0.0f);
	glTranslatef(0.0f, -1.1f, 0.5f);
	//movePart();
	RotateControl(upperLeftLegRotX, upperLeftLegRotY, upperLeftLegRotZ);

	DrawCone(&UpperLeg);
}*/
/*
	{
		//REMOVE A PART
		GLdouble eq[4] = { 0.0, 1.0, 0.0, -0. };	//Ax + By + Cz + D = 0	XYZ IS ANGLE HOW TO BE CUT
		// Equation: 0*x + 1*y + 0*z + (-0.5) <= 0 ? y <= 0.5 survives

		glEnable(GL_CLIP_PLANE0);
		glClipPlane(GL_CLIP_PLANE0, eq);

		//ADDBACK
		glDisable(GL_CLIP_PLANE0);*/
		//void Gyroscope(float yawSpeed, float pitchSpeed, float rollSpeed)
		//{
		//	// --- REALISTIC PHYSICS: use elapsed time ---
		//	float currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f; // to seconds
		//	float deltaTime = currentTime - previousTime;
		//	previousTime = currentTime;
		//
		//	// --- angular velocity (deg/s) * time = angle change ---
		//	gyroYaw += yawSpeed * deltaTime;
		//	gyroPitch += pitchSpeed * deltaTime;
		//	gyroRoll += rollSpeed * deltaTime;
		//
		//	// --- normalize ---
		//	if (gyroYaw > 360) gyroYaw -= 360;
		//	if (gyroPitch > 360) gyroPitch -= 360;
		//	if (gyroRoll > 360) gyroRoll -= 360;
		//
		//	// --- apply the rotations ---
		//	glRotatef(gyroYaw, 0, 1, 0);
		//	glRotatef(gyroPitch, 1, 0, 0);
		//	glRotatef(gyroRoll, 0, 0, 1);
		//}
SphereInfo JointSmall = { GLU_FILL, 0.75f, 0.75f, 0.75f, 0.16f, 20, 20 };
SphereInfo JointMedium = { GLU_FILL, 0.75f, 0.75f, 0.75f, 0.22f, 20, 20 };
SphereInfo JointLarge = { GLU_FILL, 0.75f, 0.75f, 0.75f, 0.30f, 20, 20 };
// <----------------- UN USED

// -- HALL OF FAME Tools UwU --
void MovePart() {
	switch (movePosCTRL) {
	case 1:
		xRotate += 1.0f;
		break;
	case 2:
		xRotate -= 1.0f;
		break;
	case 3:
		yRotate += 1.0f;
		break;
	case 4:
		yRotate -= 1.0f;
		break;
	case 5:
		zRotate += 1.0f;
		break;
	case 6:
		zRotate -= 1.0f;
		break;
	case 7:
		x += 0.1f;
		break;
	case 8:
		x -= 0.1f;
		break;
	case 9:
		y += 0.1f;
		break;
	case 10:
		y -= 0.1f;
		break;
	case 11:
		z += 0.1f;
		break;
	case 12:
		z -= 0.1f;
		break;
	default:
		break;
	}
	movePosCTRL = 0;

	glTranslatef(x, y, z);

	glRotatef(xRotate, 1, 0, 0);
	glRotatef(yRotate, 0, 1, 0);
	glRotatef(zRotate, 0, 0, 1);
//	cout << "X Pos:" << x << "	Y Pos:" << y << "Z Pos:" << z << endl;
//	cout << "X Rot:" << xRotate << "	Y Rot:" << yRotate << " Z Rot:" << zRotate << endl << endl;
}
void DrawPosSphere() {
	glPushMatrix();
	MovePart();
	glTranslatef(0.0f, 0.0f, 0.0f);
	DrawSphere(&JointSmall);
	glPopMatrix();
}
void ChangeCylinderShape(CylinderInfo* cylinder)
{
	switch (shapeChangeCTRL) {
	case 1:
		cylinder->radius += 0.1;
		break;
	case 2:
		cylinder->radius -= 0.1;
		break;
	case 3:
		cylinder->height += 0.1;
		break;
	case 4:
		cylinder->height -= 0.1;
		break;
	case 5:
		cylinder->slices += 0.1;
		break;
	case 6:
		cylinder->slices -= 0.1;
		break;
	case 7:
		cylinder->stacks += 0.1;
		break;
	case 8:
		cylinder->stacks -= 0.1;
		break;
	}
	shapeChangeCTRL = 0;
//	cout << "Radius:" << cylinder->radius << "	Height:" << cylinder->height << "	Slices:" << cylinder->slices << "	Stacks:" << cylinder->stacks << endl << endl;
}
void ChangeConeShape(ConeInfo* cone)
{
	switch (shapeChangeCTRL) {
	case 1:
		cone->radius += 0.1;
		break;
	case 2:
		cone->radius -= 0.1;
		break;
	case 3:
		cone->height += 0.1;
		break;
	case 4:
		cone->height -= 0.1;
		break;
	case 5:
		cone->slices += 0.1;
		break;
	case 6:
		cone->slices -= 0.1;
		break;
	case 7:
		cone->stacks += 0.1;
		break;
	case 8:
		cone->stacks -= 0.1;
		break;
	}
	shapeChangeCTRL = 0;
//	cout << "Radius:" << cone->radius << "	Height:" << cone->height << "	Slices:" << cone->slices << "	Stacks:" << cone->stacks << endl << endl;
}
void ChangeSphereShape(SphereInfo* sphere)
{
	switch (shapeChangeCTRL) {
	case 1:
		sphere->radius += 0.1;
		break;
	case 2:
		sphere->radius -= 0.1;
		break;
	case 5:
		sphere->slices += 0.1;
		break;
	case 6:
		sphere->slices -= 0.1;
		break;
	case 7:
		sphere->stacks += 0.1;
		break;
	case 8:
		sphere->stacks -= 0.1;
		break;
	}
	shapeChangeCTRL = 0;
//	cout << "Radius:" << sphere->radius << "	Slices:" << sphere->slices << "	Stacks:" << sphere->stacks << endl << endl;
}
void ChangeCubeShape() {

}
// Robot Skin
// Hip

// --------------------------------------- V  Shape Variable Declaration Here V ----------------------------------------------------------------------//

// Left Leg
CylinderInfo upperLeftLegCylinder = {
	GLU_FILL,     // drawStyle
	0.1f, 0.1f, 0.1f,
	0.65f,         // radius
	3.4f,         // height
	30,           // slices
	30            // stacks
};
CylinderInfo upperLeftLegGuard = {
	GLU_FILL,     // drawStyle
	0.19f, 0.19f, 0.19f,
	0.75f,         // radius
	2.4f,         // height
	30,           // slices
	30            // stacks
};

CylinderInfo upperLeftFootCylinder = {
	GLU_FILL,     // drawStyle
	0.5f, 0.5f, 0.0f,
	0.6f,         // radius
	0.9f,         // height
	30,           // slices
	30            // stacks
};


// Right Leg
CylinderInfo upperRightLegCylinder = {
	GLU_FILL,     // drawStyle
	0.1f, 0.1f, 0.1f,
	0.65f,         // radius
	3.4f,         // height
	30,           // slices
	30            // stacks
};
CylinderInfo upperRightLegGuard = {
	GLU_FILL,     // drawStyle
	0.22f, 0.22f, 0.22f,
	0.75f,         // radius
	2.4f,         // height
	30,           // slices
	30            // stacks
};

CylinderInfo legArmorRims = {
	GLU_FILL,     // drawStyle
	0.6f, 0.6f, 0.0f,
	0.8f,         // radius
	0.3f,         // height
	30,           // slices
	30            // stacks
};

CylinderInfo upperRightFootCylinder = {
	GLU_FILL,     // drawStyle
	0.5f, 0.5f, 0.0f,
	0.6f,         // radius
	0.9f,         // height
	30,           // slices
	30            // stacks
};

// Body
CylinderInfo tankPiece = {
	GLU_FILL,     // drawStyle
	0.35f, 0.35f, 0.35f,
	0.45f,         // radius
	3.4f,         // height
	30,           // slices
	30            // stacks
};

CylinderInfo tankRimPiece = {
	GLU_FILL,     // drawStyle
	0.25f, 0.25f, 0.25f,
	0.5f,         // radius
	0.3f,         // height
	30,           // slices
	30            // stacks
};

SphereInfo tankHead = { GLU_FILL, 0.15f, 0.15f, 0.15f, 0.43f, 20, 20 };
SphereInfo smallPiece = { GLU_FILL, 0.15f, 0.15f, 0.15f, 0.15f, 20, 20 };
SphereInfo smallPieceB = { GLU_FILL, 0.9f, 0.9f, 0.15f, 0.15f, 20, 20 };

// Left Arm
CylinderInfo LeftShoulderArmBase = {
	GLU_FILL,     // drawStyle
	0.85f, 0.85f, 0.0f,
	0.6f,         // radius
	1.5f,         // height
	30,           // slices
	30            // stacks
};

CylinderInfo LeftShoulderWide = {
	GLU_FILL,     // drawStyle
	0.75f, 0.75f, 0.0f,
	0.7f,         // radius
	0.6f,         // height
	30,           // slices
	30            // stacks
};

CylinderInfo LeftUpperArmBase = {
	GLU_FILL,     // drawStyle
	0.15f, 0.15f, 0.15f,
	0.55f,         // radius
	3.2f,         // height
	30,           // slices
	30            // stacks
};

// Right Arm
CylinderInfo RightShoulderArmBase = {
	GLU_FILL,     // drawStyle
	0.9f, 0.9f, 0.0f,
	0.6f,         // radius
	1.5f,         // height
	30,           // slices
	30            // stacks
};
CylinderInfo RightShoulderWide = {
	GLU_FILL,     // drawStyle
	0.75f, 0.75f, 0.0f,
	0.7f,         // radius
	0.6f,         // height
	30,           // slices
	30            // stacks
};
CylinderInfo RightUpperArmBase = {
	GLU_FILL,     // drawStyle
	0.15f, 0.15f, 0.15f,
	0.55f,         // radius
	3.2f,         // height
	30,           // slices
	30            // stacks
};


CylinderInfo lowerRightArmBase = {
	GLU_FILL,     // drawStyle
	0.35f, 0.35f, 0.35f,
	0.5f,         // radius
	4.1f,         // height
	30,           // slices
	30            // stacks
};
CylinderInfo lowerRightArmTopDrill = {
	GLU_FILL,     // drawStyle
	0.6f, 0.6f, 0.0f,
	0.9f,         // radius
	0.6f,         // height
	8,           // slices
	8            // stacks
};
CylinderInfo lowerRightArmTopOuterBase = {
	GLU_FILL,     // drawStyle
	0.6f, 0.6f, 0.0f,
	0.8f,         // radius
	0.6f,         // height
	8,           // slices
	8            // stacks
};
CylinderInfo lowerRightArmShellTopMid = {
	GLU_FILL,     // drawStyle
	0.75f, 0.75f, 0.0f,
	1.1f,         // radius
	1.6f,         // height
	8,           // slices
	8            // stacks
};
CylinderInfo lowerRightArmShellBot = {
	GLU_FILL,     // drawStyle
	0.0f, 1.0f, 0.0f,
	0.8f,         // radius
	0.6f,         // height
	8,           // slices
	8            // stacks
};

// Head + Neck
SphereInfo headPiece = { GLU_FILL, 0.75f, 0.75f, 0.75f, 0.3f, 20, 20 };
CylinderInfo skullPiece = {
	GLU_FILL,     // drawStyle
	0.80f, 0.80f, 0.0f,
	0.7f,         // radius
	1.8f,         // height
	30,           // slices
	30            // stacks
};

CylinderInfo neckPiece = {
	GLU_FILL,     // drawStyle
	0.0f, 0.0f, 0.0f,
	0.45f,         // radius
	0.7f,         // height
	30,           // slices
	30            // stacks
};
CylinderInfo lowerNeckPiece = {
	GLU_FILL,     // drawStyle
	0.5f, 0.5f, 0.0f,
	0.6f,         // radius
	0.3f,         // height
	30,           // slices
	30            // stacks
};

CylinderInfo helmetConnectorPiece = {
	GLU_FILL,     // drawStyle
	0.3f, 0.3f, 0.0f,
	0.2f,         // radius
	1.6f,         // height
	30,           // slices
	30            // stacks
};


//------------------------------------------------------------------------------------------------------------------------------------//
void DrawHalfSphere(float radius, int slices, int stacks)
{
	for (int i = 0; i < stacks / 2; i++)   // half sphere
	{
		float lat0 = 3.142 * (-0.5f + (float)i / stacks);
		float lat1 = 3.142 * (-0.5f + (float)(i + 1) / stacks);

		float z0 = sin(lat0);
		float zr0 = cos(lat0);

		float z1 = sin(lat1);
		float zr1 = cos(lat1);

		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j <= slices; j++)
		{
			float lng = 2 * 3.142 * (float)j / slices;
			float x = cos(lng);
			float y = sin(lng);

			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(radius * x * zr0, radius * y * zr0, radius * z0);

			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(radius * x * zr1, radius * y * zr1, radius * z1);
		}
		glEnd();
	}
}
void DrawFinger(GLUquadric* quad)
{
	// Finger body
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	gluCylinder(quad, 0.08f, 0.07f, 0.35f, 12, 3);

	// Finger tip (cone)
	glTranslatef(0.0f, 0.0f, 0.35f);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	gluCylinder(quad, 0.07f, 0.0f, 0.1f, 12, 2);
}

// Editable finger positions
float finger1Pos[3] = { -0.35f, 0.0f, 0.45f };
float finger2Pos[3] = { -0.12f, 0.0f, 0.48f };
float finger3Pos[3] = { 0.12f, 0.0f, 0.48f };
float finger4Pos[3] = { 0.35f, 0.0f, 0.45f };
float thumbPos[3] = { -0.55f,-0.15f, 0.25f };

void DrawRightArm()
{
	glPushMatrix();

	// =====================
	// PALM
	// =====================
	glPushMatrix();
	glScalef(1.3f, 1.3f, 2.4f);
	glRotatef(180, 0, 1, 0);
	glColor3f(0, 0, 0);
	MovePart();
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawHalfSphere(0.5f, 24, 24);
	glRotatef(-180, 0, 1, 0);
	glColor3f(1, 1, 0);

	// =====================
	// FINGER 1
	// =====================
	glPushMatrix();
	glTranslatef(finger1Pos[0], finger1Pos[1], finger1Pos[2]);
	glRotatef(-20.0f, 1, 0, 0);
	DrawFinger(quad);
	glPopMatrix();

	// =====================
	// FINGER 2
	// =====================
	glPushMatrix();
	glTranslatef(finger2Pos[0], finger2Pos[1], finger2Pos[2]);
	glRotatef(-18.0f, 1, 0, 0);
	DrawFinger(quad);
	glPopMatrix();

	// =====================
	// FINGER 3
	// =====================
	glPushMatrix();
	glTranslatef(finger3Pos[0], finger3Pos[1], finger3Pos[2]);
	glRotatef(-18.0f, 1, 0, 0);
	DrawFinger(quad);
	glPopMatrix();

	// =====================
	// FINGER 4
	// =====================
	glPushMatrix();
	glTranslatef(finger4Pos[0], finger4Pos[1], finger4Pos[2]);
	glRotatef(-20.0f, 1, 0, 0);
	DrawFinger(quad);
	glPopMatrix();

	// =====================
	// THUMB
	// =====================
	glPushMatrix();
	glTranslatef(thumbPos[0], thumbPos[1], thumbPos[2]);
	glRotatef(-45.0f, 0, 1, 0);
	DrawFinger(quad);
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();

}


float clawAngle = 30.0;
// EXCAVATOR BUCKET
void DrawHalfCylinder(float radius, float height, int segments) {
	// Draw the curved side surface (using quads)
	float angleStep = 180.0f / segments;  // Divide the 180 degrees into smaller segments

	// Draw the curved surface of the half-cylinder
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= segments; ++i) {
		float angle = i * angleStep;

		// Calculate X and Y positions for the current angle on the curved surface
		float x1 = radius * cos(angle * 3.142 / 180.0f);
		float y1 = radius * sin(angle * 3.142 / 180.0f);
		float x2 = radius * cos((angle + angleStep) * 3.142 / 180.0f);
		float y2 = radius * sin((angle + angleStep) * 3.142 / 180.0f);

		// Draw two vertices for each quad (the current and the next angle)
		glVertex3f(x1, y1, 0.0f); // Bottom vertex
		glVertex3f(x1, y1, height); // Top vertex
		glVertex3f(x2, y2, 0.0f); // Bottom vertex
		glVertex3f(x2, y2, height); // Top vertex
	}
	glEnd();

	// Draw the top half-circle
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, height);  // Center of the top face
	for (int i = 0; i <= segments; ++i) {
		float angle = i * angleStep;
		float x = radius * cos(angle * 3.142 / 180.0f);
		float y = radius * sin(angle * 3.142 / 180.0f);
		glVertex3f(x, y, height);
	}
	glEnd();

	// Draw the bottom half-circle (center part is open, so no bottom face)
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, 0.0f);  // Center of the bottom face
	for (int i = 0; i <= segments; ++i) {
		float angle = i * angleStep;
		float x = radius * cos(angle * 3.142 / 180.0f);
		float y = radius * sin(angle * 3.142 / 180.0f);
		glVertex3f(x, y, 0.0f);
	}
	glEnd();
}

void DrawExcavatorBucket()
{
	glPushMatrix();
	glTranslatef(0.0f, -0.22f, 0.35f);
	glColor3f(0.65, 0.65, 0);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.5, 0.5, 0.01);
	glRotatef(74, 1, 0, 0);
	glTranslatef(0.01f, -0.30f, 0.24f);
	DrawCube(0.5, 0.5, 0.01);
	glRotatef(20, 1, 0, 0);
	glTranslatef(0, -0.48f, 0.08);
	DrawCube(0.5, 0.5, 0.01);
	glRotatef(90, 0, 1, 0);
	glRotatef(30, 0, 0, 1);
	glTranslatef(0.5, 0.14f, -0.35);
	glColor3f(0.35, 0.35, 0.35);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawHalfCylinder(0.4f, 0.7f, 30);
	glColor3f(0.5, 0.5, 0.5);
	glRotatef(-90, 0, 1, 0);
	glTranslatef(0.06, -0.02f, -0.38);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.11, 0.3, 0.01);
	glTranslatef(0.2, 0, 0);
	DrawCube(0.11, 0.3, 0.01);
	glTranslatef(0.2, 0, 0);
	DrawCube(0.11, 0.3, 0.01);
	glTranslatef(0.15, 0, 0);
	DrawCube(0.11, 0.3, 0.01);
	glColor3f(0.65, 0.65, 0);

	glPopMatrix();
}

// RIGHT ARM
void DrawLeftArm()
{
	glPushMatrix();

	// ---------- ARM EXTENSION ----------
	glRotatef(90, 1, 0, 0);
	glColor3f(0.15, 0.15, 0.15);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	gluCylinder(quad, 0.14f, 0.14f, 1.55f, 16, 4);

	// ---------- FOREARM ----------
	glTranslatef(0.0f, 0.0f, 0.35f);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	gluCylinder(quad, 0.12f, 0.1f, 0.6f, 16, 4);

	// ---------- CLAW BASE ----------
	glTranslatef(0.0f, 0.0f, 0.6f);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	gluCylinder(quad, 0.1f, 0.1f, 0.15f, 16, 4);
	glTranslatef(0.0f, 0.0f, 0.4f);


	// ---------- LEFT BUCKET ----------
	glPushMatrix();
	glTranslatef(0.0f, 0.12f, 0.15f);
	//MovePart();
	glRotatef(clawAngle, 1, 0, 0);
	//glRotatef(-clawAngle, 1, 0, 0);
	MovePart();
	DrawExcavatorBucket();
	glPopMatrix();

	// ---------- RIGHT BUCKET ----------
	glPushMatrix();
	glTranslatef(0.0f, -0.12f, 0.15f);
	glRotatef(-clawAngle, 1, 0, 0);
	glRotatef(180, 0, 0, 1);
	DrawExcavatorBucket();

	glPopMatrix();

	glPopMatrix();
}

//---------------------------------------------  V Section to Apply Variables Here  V -----------------------------------------------------//

// Hip
void DrawRobotHip()
{

	glPushMatrix();
	glTranslatef(0.6, 0.15, 0);
	glColor3f(0.7, 0.7, 0.0);
	glScalef(4.4, 1.6, 2.4);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawTrapeziumQuad(0.55, 0.75);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.4, 0.1, 0);
	glColor3f(0.4, 0.4, 0.0);
	glScalef(4.2, 1.1, 2.2);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawTrapeziumQuad(0.45, 0.6);
	glPopMatrix();
}

// Left Leg
void DrawRobotUpperLeftLeg() {
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&upperLeftLegCylinder);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 0.4);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&upperLeftLegGuard);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 0.35);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&legArmorRims);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 1.5);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&legArmorRims);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 2.65);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&legArmorRims);
	glPopMatrix();
}
void DrawRobotLowerLeftLeg() {
	glPushMatrix();
	glTranslatef(0, 0, 1.4);
	glColor3f(0.7, 0.69, 0.0);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(1.4, 1.4, 2.5);
	glPopMatrix();
}
void DrawRobotLeftFoot() {
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&upperLeftFootCylinder);
	glPopMatrix();
}
void DrawRobotLeftToes() {
	glPushMatrix();
	glTranslatef(-0.3, -0.2, 0.3);
	glColor3f(0.6, 0.6, 0.0);
	glRotatef(-10, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.5, 0.6, 1.8);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.2, -0.2, 0.3);
	glColor3f(0.1, 0.1, 0.1);
	glRotatef(5, 0, 1, 0);
	DrawCube(0.5, 0.6, 1.8);
	glPopMatrix();
}



// Right Leg
void DrawRobotUpperRightLeg() {
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&upperRightLegCylinder);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 0.4);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&upperRightLegGuard);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 0.75);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&legArmorRims);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 2.15);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&legArmorRims);
	glPopMatrix();
}
void DrawRobotLowerRightLeg() {
	glPushMatrix();
	glTranslatef(0, 0, 1.4);
	glColor3f(0.7, 0.7, 0.0);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(1.4, 1.4, 2.5);
	glPopMatrix();
}
void DrawRobotRightFoot() {
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&upperRightFootCylinder);
	glPopMatrix();
}
void DrawRobotRightToes() {
	glPushMatrix();
	glTranslatef(-0.3, 0.1, 0.3);
	glColor3f(0.6, 0.6, 0.0);
	glRotatef(-10, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.5, 0.6, 1.8);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.2, 0.1, 0.3);
	glColor3f(0.1, 0.1, 0.1);
	glRotatef(5, 0, 1, 0);
	DrawCube(0.5, 0.6, 1.8);
	glPopMatrix();
}

//Torso
void DrawRobotLowerTorso() {
	glPushMatrix();
	glTranslatef(-0.65, 0.0, 0.1);
	glColor3f(0.7, 0.7, 0.0);
	glScalef(4.2, 2.0, 1.8);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawTrapeziumQuad(0.6, 0.8);
	glPopMatrix();
}
void DrawRobotUpperTorso() {
	//Main Torso(Black)
	glPushMatrix();
	glTranslatef(0, 0, 1.5);
	glColor3f(0.1, 0.1, 0.1);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(1.3, 2.7, 3);
	glPopMatrix();

	//Armor Torso (Dark GRey)
	glPushMatrix();
	glTranslatef(-0.1, 0, 0.5);
	glColor3f(0.2, 0.2, 0.2);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(1.3, 3.1, 0.9);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.45, 0, 0.5);
	glColor3f(0.45, 0.45, 0.45);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.7, 2.7, 0.6);
	glPopMatrix();


	//Armor Torso (Grey Part)
	glPushMatrix();
	glTranslatef(-0.35, 0, 2.35);
	glColor3f(0.25, 0.25, 0.25);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.8, 3.1, 1.3);
	glPopMatrix();

	//Armor Torso (Light Grey Part)
	glPushMatrix();
	glTranslatef(-0.45, -0.7, 2.35);
	glColor3f(0.45, 0.45, 0.45);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.7, 1.1, 1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.45, 0.7, 2.35);
	glColor3f(0.45, 0.45, 0.45);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.7, 1.1, 1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.35, 1.4, 1.9);
	glColor3f(0.45, 0.45, 0.45);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.45, 0.4, 1.8);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.35, -1.4, 1.9);
	glColor3f(0.45, 0.45, 0.45);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.45, 0.4, 1.8);
	glPopMatrix();

	//Armor Torso Lower Section 
	glPushMatrix();
	glTranslatef(-0.35, 0, 1.35);
	glColor3f(0.25, 0.25, 0.25);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.8, 3.1, 0.9);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.45, 0, 1.35);
	glColor3f(0.45, 0.45, 0.45);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.7, 2.7, 0.6);
	glPopMatrix();

	// Gas Tank Looking (Left)
	glPushMatrix();
	glTranslatef(1.3, 0.65, 0.5);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&tankPiece);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, 0.65, 0.3);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&tankRimPiece);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, 0.65, 3.3);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&tankRimPiece);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, 0.65, 3.65);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawSphere(&tankHead);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, 0.92, 2.2);
	glColor3f(0.2, 0.2, 0.2);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.4, 0.4, 1.6);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, 0.96, 2.2);
	glColor3f(0.3, 0.3, 0.3);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.3, 0.4, 1.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, 1.05, 1.2);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawSphere(&smallPiece);
	glPopMatrix();


	// Gas Tank Looking (Right)
	glPushMatrix();
	glTranslatef(1.3, -0.65, 0.5);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&tankPiece);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, -0.65, 0.3);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&tankRimPiece);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, -0.65, 3.3);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&tankRimPiece);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, -0.65, 3.65);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawSphere(&tankHead);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, -0.92, 2.2);
	glColor3f(0.2, 0.2, 0.2);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.4, 0.4, 1.6);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, -0.96, 2.2);
	glColor3f(0.3, 0.3, 0.3);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.3, 0.4, 1.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, -1.05, 1.2);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawSphere(&smallPiece);
	glPopMatrix();

	//Connecting Tank Plate
	glPushMatrix();
	glTranslatef(1.3, 0, 1.3);
	glColor3f(0.2, 0.2, 0.2);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.5, 0.6, 0.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, 0, 2.7);
	glColor3f(0.2, 0.2, 0.2);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.5, 0.6, 0.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.0, 0, 2.0);
	glColor3f(0.2, 0.2, 0.2);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.8, 0.4, 1.4);
	glPopMatrix();


}
void DrawRobotShoulderGirdle() {
	//Trapezium Looking ahhh part
	glPushMatrix();
	glTranslatef(0.6, 0, 0.7);
	glColor3f(0.7, 0.7, 0.0);
	glScalef(4.1, 2.2, 1.5);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawTrapeziumQuad(0.5, 0.8);
	glPopMatrix();

	//Decoration (Badge)
	glPushMatrix();
	glTranslatef(-0.4, 0.65, 0.7);
	glColor3f(0.85, 0.85, 0.1);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.5, 1.0, 0.5);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-0.5, 0.65, 0.7);
	glColor3f(0.55, 0.55, 0.1);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.4, 0.8, 0.35);
	glPopMatrix();


	//Light Looking Spheres
	glPushMatrix();
	glTranslatef(-0.65, -0.65, 0.9);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawSphere(&smallPiece);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.65, -1.0, 0.9);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawSphere(&smallPieceB);
	glPopMatrix();

}

// Left Arm
void DrawRobotLeftShoulder() {
	glPushMatrix();
	glTranslatef(0, 0, -0.2);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&LeftShoulderArmBase);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 0.8);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&LeftShoulderWide);
	glPopMatrix();
}
void DrawRobotUpperLeftArm() {
	glPushMatrix();
	glTranslatef(0, 0, -0.2);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&LeftUpperArmBase);
	glPopMatrix();
}
void DrawRobotLowerLeftArm() {
	glPushMatrix();
	glScalef(3, 2, 3);
	glTranslatef(0, 0, -0.6);
	glRotatef(-90, 1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawLeftArm();
	glPopMatrix();
}




// Right Arm
void DrawRobotRightShoulder() {
	glPushMatrix();
	glTranslatef(0, 0, -0.2);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&RightShoulderArmBase);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 0.8);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&RightShoulderWide);
	glPopMatrix();
}
void DrawRobotUpperRightArm() {
	glPushMatrix();
	glTranslatef(0, 0, -0.2);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&RightUpperArmBase);
	glPopMatrix();
}
void DrawRobotLowerRightArm()
{
	glPushMatrix();
	glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.8f, -1.8f);

	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&lowerRightArmBase);
	glPopMatrix();

	glPushMatrix();
	glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.8f, -1.9f);
	//ChangeCylinderShape(&lowerRightArmTopDrill);
	//MovePart();
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&lowerRightArmTopDrill);
	glPopMatrix();

	glPushMatrix();
	glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.8f, -0.6f);
	//ChangeCylinderShape(&lowerRightArmTopOuterBase);
	//MovePart();
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&lowerRightArmTopOuterBase);
	glPopMatrix();

	glPushMatrix();
	glRotatef(25.0f, 1.0f, 0.0f, 0.0f); // Keep the main rotation intact
	glTranslatef(0.0f, 0.8f, -1.9f); // Starting translation
	glPopMatrix();


	glPushMatrix();
	glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.1f, 0.8f, -0.1f);
	//ChangeCylinderShape(&lowerRightArmShellTopMid);
	//MovePart();
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&lowerRightArmShellTopMid);
	glPopMatrix();

	glPushMatrix();
	glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.1f, 0.8f, -0.1f);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	ChangeCylinderShape(&lowerRightArmShellBot);
	DrawCylinder(&lowerRightArmShellBot);
	glPopMatrix();

	glPushMatrix();
	glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(-0.1f, 0.7f, 2.1f);
	DrawRightArm();
	glPopMatrix();
}


// Neck + Head
void DrawRobotNeck() {
	//Main Neck
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&neckPiece);
	glPopMatrix();

	//Neck piece to torso
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&lowerNeckPiece);
	glPopMatrix();
}
void DrawRobotHead() {
	// Face Piece (Half Sphere looking ahhh)
	glPushMatrix();
	GLdouble plane[] = { -1.0, 0.0, 0.0, 0.0 };

	glPushMatrix();

	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, plane);

	glTranslatef(-0.2, 0, 0.9);
	glScalef(2.2, 2.6, 2.6);
	glBindTexture(GL_TEXTURE_2D, GLASS);
	DrawSphere(&headPiece);

	glDisable(GL_CLIP_PLANE0);

	glPopMatrix();
	glPopMatrix();

	// Head Piece 
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCylinder(&skullPiece);
	glPopMatrix();

	//Left Face Plate
	glPushMatrix();
	glTranslatef(0.13, 0.84, 0.8);
	glColor3f(0.5, 0.5, 0.0);
	glRotatef(-7, 0, 0, 1);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(1.1, 0.3, 1.6);
	glPopMatrix();

	//Right face plate
	glPushMatrix();
	glTranslatef(0.13, -0.84, 0.8);
	glColor3f(0.5, 0.5, 0.0);
	glRotatef(7, 0, 0, 1);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(1.1, 0.3, 1.6);
	glPopMatrix();

	//Rear Head Plate
	glPushMatrix();
	glTranslatef(0.73, 0, 0.8);
	glColor3f(0.45, 0.45, 0.0);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(0.3, 1.5, 1.6);
	glPopMatrix();

	//Extra Rear Head Layer
	glPushMatrix();
	glTranslatef(0.85, 0, 0.73);
	glColor3f(0.4, 0.4, 0.0);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.3, 1.25, 1.4);
	glPopMatrix();

	//Cylinder connector for plates
	glPushMatrix();
	glTranslatef(0.7, 0.8, 0);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&helmetConnectorPiece);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.7, -0.8, 0);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCylinder(&helmetConnectorPiece);
	glPopMatrix();

	//Top Connecting Piece
	glPushMatrix();
	glTranslatef(0.625, 0, 1.65);
	glColor3f(0.65, 0.7, 0.0);
	glRotatef(-30.0, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(0.3, 1.5, 0.4);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.025, -0.7, 1.65);
	glColor3f(0.65, 0.7, 0.0);
	glRotatef(-30.0, 1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(1.55, 0.3, 0.4);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.025, 0.7, 1.65);
	glColor3f(0.65, 0.7, 0.0);
	glRotatef(30.0, 1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
	DrawCube(1.55, 0.3, 0.4);
	glPopMatrix();

	//Top Helmet Piece
	glPushMatrix();
	glTranslatef(0.0, 0.0, 1.8);
	glColor3f(0.65, 0.7, 0.0);
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawCube(1.5, 1.4, 0.4);
	glPopMatrix();

}

//--------------------------------------------------------------------------------------------------------------------------------------//

//Bones Struct
// -----  Head	-----
ConeInfo Head = {
	GLU_FILL,
	0.0f, 1.0f, 0.7f,
	0.4f,
	1.8f,
	30, 30
};
ConeInfo Neck = {
	GLU_FILL,
	0.7f, 0.7f, 0.7f,
	0.25f,
	0.7f,
	20, 20
};

// -----  Arms	-----
ConeInfo Shoulder = {
	GLU_FILL,
	1.0f, 0.7f, 0.7f,
	0.4f,
	1.3f,
	30, 30
};
ConeInfo UpperArm = {
	GLU_FILL,
	0.7f, 0.7f, 0.7f,
	0.33f,
	2.7f,
	30, 30
};
ConeInfo LowerArm = {
	GLU_FILL,
	0.0f, 0.0f, 1.0f,
	0.25f,
	2.55f,
	30, 30
};

// -----  Shoulder Gridle	-----	(Neck + Arms + Torso Connector)
ConeInfo ShoulderGirdle = {
	GLU_FILL,
	1.0f, 0.0f, 0.0f,
	0.4f,
	1.4f,
	24, 24
};
ConeInfo ShoulderGirdleSide = {
	GLU_FILL,
	0.7f, 0.7f, 0.7f,
	0.4f,
	2.0f,
	30, 30
};

// ----- Torso  -----
ConeInfo UpperTorso = {
	GLU_FILL,
	0.0f, 0.0f, 1.0f,
	0.40f,
	3.2f,
	30, 30
};
ConeInfo LowerTorso = {
	GLU_FILL,
	0.0f, 1.0f, 0.0f,
	0.40f,
	1.00f,
	30, 30
};
// -----  Hip	-----
ConeInfo HipSkeleton = {
	GLU_FILL,
	0.7f, 0.7f, 0.7f,
	0.25f,
	1.2f,
	30, 30
};
ConeInfo HipSkeletonConnectLowerTorso = {
	GLU_FILL,
	0.7f, 0.7f, 0.7f,
	0.25f,
	1.0f,
	30, 30
};

// -----  Legs	-----
ConeInfo UpperLeg = {
	GLU_FILL,
	0.6f, 0.6f, 0.9f,
	0.30f,
	2.9f,
	30, 30
};
ConeInfo LowerLeg = {
	GLU_FILL,
	0.45f, 0.45f, 0.45f,
	0.25f,              // radius
	2.5f,               // height
	30, 30
};
ConeInfo Foot = {
	GLU_FILL,
	0.7f, 0.7f, 0.9f,   // light color like legs
	0.22f,              // radius (slightly smaller than lower leg)
	1.0f,               // height
	30, 30
};
ConeInfo Toes = {
	GLU_FILL,
	1.0f, 0.0f, 0.0f,
	0.15f,              // radius
	0.8f,               // height (toes segment is short)
	30, 30
};

void RotateControl(float x, float y, float z) {
	glRotatef(x, 1, 0, 0);
	glRotatef(y, 0, 1, 0);
	glRotatef(z, 0, 0, 1);
}

struct Move {
	int action;
	float rotX, rotY, rotZ;      // current rotations
	float minX, maxX;
	float minY, maxY;
	float minZ, maxZ;
	float moveXAmt, moveYAmt, moveZAmt;
};
// Movement Bone Struct

// Hip Move Struct
Move hipMove = {
	0,
	0, 0, 0,
	-45.0f, 60.0f,
	-30.0f, 30.0f,
	-25.0f, 25.0f,
	1.0f, 1.0f, 1.0f
};
//Left Leg Move Struct
Move upperLeftLegMove = {
	0,
	0, 0, 0,
	-25.0f, 45.0f,
	-95.0f, 48.0f,
	-10.0f, 25.0f,
	1.0f, 1.0f, 1.0f
};
Move lowerLeftLegMove = {
	0,
	0, 0, 0,
	-5.0f, 5.0f,
	-5.0f, 130.0f,
	-5.0f, 10.0f,
	1.0f, 1.0f, 1.0f
};
Move footLeftMove = {
	0,
	0, 0, 0,
	-25.0f, 30.0f,
	-10.0f, 15.0f,
	-2.0f, 5.0f,
	1.0f, 1.0f, 1.0f
};
Move toesLeftMove = {
	0,
	0, 0, 0,
	-30.0f, 25.0f,
	-10.0f, 15.0f,
	2.0f, -5.0f,
	1.0f, 1.0f, 1.0f
};
//Right Leg Move Struct
Move upperRightLegMove = {
	0,
	0, 0, 0,
	-25.0f, 45.0f,
	-48.0f, 95.0f,
	-25.0f, 10.0f,
	1.0f, 1.0f, 1.0f
};
Move lowerRightLegMove = {
	0,
	0, 0, 0,
	-5.0f, 5.0f,
	-130.0f, 5.0f,
	-10.0f, 5.0f,
	1.0f, 1.0f, 1.0f
};
Move footRightMove = {
	0,
	0, 0, 0,
	-25.0f, 30.0f,
	-15.0f, 10.0f,
	-5.0f, 2.0f,
	1.0f, 1.0f, 1.0f
};
Move toesRightMove = {
	0,
	0, 0, 0,
	-30.0f, 25.0f,
	-15.0f, 10.0f,
	-5.0f, 2.0f,
	1.0f, 1.0f, 1.0f
};

// Body Move Struct
Move lowerTorsoMove = {
	0,
	0, 0, 0,
	-30.0f, 30.0f,
	-60.0f, 20.0f,
	-30.0f, 30.0f,
	1.0f, 1.0f, 1.0f
};
Move upperTorsoMove = {
	0,
	0, 0, 0,
	-15.0f, 15.0f,
	-10.0f, 10.0f,
	-30.0f, 30.0f,
	1.0f, 1.0f, 1.0f
};
Move shoulderGridle = {
	0,
	0, 0, 0,
	-5.0f, 5.0f,
	-5.0f, 5.0f,
	-10.0f, 10.0f,
	1.0f, 1.0f, 1.0f
};

// Left Arm Move Struct
Move shoulderLeftMove = {
	0,
	0, 0, 0,
	-15.0f, 20.0f,
	-10.0f, 15.0f,
	-5.0f, 10.0f,
	1.0f, 1.0f, 1.0f
};
Move upperLeftArmMove = {
	0,
	0, 0, 0,
	-150.0f, 20.0f,
	-140.0f, 50.0f,
	-160.0f, 20.0f,
	1.0f, 1.0f, 1.0f
};
Move lowerLeftArmMove = {
	0,
	0, 0, 0,
	-3.0f, 150.0f,
	-3.0f, 3.0f,
	-5.0f, 180.0f,
	1.0f, 1.0f, 1.0f
};

// Right Arm Move Struct
Move shoulderRightMove = {
	0,
	0, 0, 0,
	-15.0f, 20.0f,
	-15.0f, 10.0f,
	-10.0f, 5.0f,
	1.0f, 1.0f, 1.0f
};
Move upperRightArmMove = {
	0,
	0, 0, 0,
	-150.0f, 20.0f,
	-50.0f, 140.0f,
	-20.0f, 160.0f,
	1.0f, 1.0f, 1.0f
};
Move lowerRightArmMove = {
	0,
	0, 0, 0,
	-3.0f, 150.0f,
	-3.0f, 3.0f,
	-180.0f, 5.0f,
	1.0f, 1.0f, 1.0f
};

// Neck + Head Move Struct
Move neckMove = {
	0,
	0, 0, 0,
	-35.0f, 35.0f,
	-80.0f, 40.0f,
	-45.0f, 45.0f,
	1.0f, 1.0f, 1.0f
};
Move headMove = {
	0,
	0, 0, 0,
	-25.0f, 25.0f,
	-65.0f, 30.0f,
	-10.0f, 10.0f,
	1.0f, 1.0f, 1.0f
};

// Joints Array
Move* joints[] = {
	// 1 Hip
	&hipMove,                // 1

	// 2–5 Left Leg
	&upperLeftLegMove,       // 2
	&lowerLeftLegMove,       // 3
	&footLeftMove,           // 4
	&toesLeftMove,           // 5

	// 6–9 Right Leg
	&upperRightLegMove,      // 6
	&lowerRightLegMove,      // 7
	&footRightMove,          // 8
	&toesRightMove,          // 9

	// 10–11 Torso
	&lowerTorsoMove,         // 10
	&upperTorsoMove,         // 11

	// 12 Shoulder Girdle
	&shoulderGridle,         // 12

	// 13–15 Left Arm
	&shoulderLeftMove,       // 13
	&upperLeftArmMove,       // 14
	&lowerLeftArmMove,       // 15

	// 16–18 Right Arm
	&shoulderRightMove,      // 16
	&upperRightArmMove,      // 17
	&lowerRightArmMove,      // 18

	// 19–20 Neck + Head
	&neckMove,               // 19
	&headMove                // 20
};

// ----- Hip -----
void DrawRobotHipSkeleton() {
	RotateControl(hipMove.rotX, hipMove.rotY, hipMove.rotZ);
	glRotatef(270, 1, 0, 0);
	//Mid
	DrawCone(&HipSkeletonConnectLowerTorso);
	//Left
	glRotatef(115, 1, 0, 0);
	DrawCone(&HipSkeleton);
	//Right
	glRotatef(130, 1, 0, 0);
	DrawCone(&HipSkeleton);
	glRotatef(-155, 1, 0, 0);	//DO NOT REMOVE
	glBindTexture(GL_TEXTURE_2D, METAL_YELLOW_RUSTY);
	DrawRobotHip();
	glBindTexture(GL_TEXTURE_2D, METAL_BLACK);
}

// ----- Left Leg -----
void DrawRobotUpperLeftLegSkeleton() {
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -1.1f, 0.5f);
	RotateControl(upperLeftLegMove.rotX, upperLeftLegMove.rotY, upperLeftLegMove.rotZ);
	DrawCone(&UpperLeg);
	DrawRobotUpperLeftLeg();
}
void DrawRobotLowerLeftLegSkeleton() {
	glTranslatef(0.0f, 0.0f, 2.9f);
	RotateControl(lowerLeftLegMove.rotX, lowerLeftLegMove.rotY, lowerLeftLegMove.rotZ);
	DrawCone(&LowerLeg);
	DrawRobotLowerLeftLeg();
}
void DrawRobotLeftFootSkeleton() {
	glRotatef(-5.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.2f, 0.0f, 2.5f);
	RotateControl(footLeftMove.rotX, footLeftMove.rotY, footLeftMove.rotZ);
	DrawCone(&Foot);
	DrawRobotLeftFoot();
}
void DrawRobotLeftToesSkeleton() {
	glRotatef(-95.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(265.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, -1.0f, 0.0f);
	RotateControl(toesLeftMove.rotX, toesLeftMove.rotY, toesLeftMove.rotZ);
	DrawCone(&Toes);
	DrawRobotLeftToes();
}

// ----- Right Leg -----
void DrawRobotUpperRightLegSkeleton() {
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 1.05f, 0.5f);
	RotateControl(upperRightLegMove.rotX, upperRightLegMove.rotY, upperRightLegMove.rotZ);
	DrawCone(&UpperLeg);
	DrawRobotUpperRightLeg();
}
void DrawRobotLowerRightLegSkeleton() {
	glTranslatef(0.0f, 0.0f, 2.9f);
	RotateControl(lowerRightLegMove.rotX, lowerRightLegMove.rotY, lowerRightLegMove.rotZ);
	DrawCone(&LowerLeg);
	DrawRobotLowerRightLeg();
}
void DrawRobotRightFootSkeleton() {
	glTranslatef(0.0f, 0.0f, 2.5f);
	glRotatef(-5.0f, 0.0f, 1.0f, 0.0f);
	RotateControl(lowerRightLegMove.rotX, lowerRightLegMove.rotY, lowerRightLegMove.rotZ);
	DrawCone(&Foot);
	DrawRobotRightFoot();
}
void DrawRobotRightToesSkeleton() {
	glRotatef(-5.0f, 0.0f, 1.0f, 0.0f);	//Inversed
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);		//Inversed
	glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 1.05f, -0.1f);
	RotateControl(toesRightMove.rotX, toesRightMove.rotY, toesRightMove.rotZ);
	DrawCone(&Toes);
	DrawRobotRightToes();
}

// ----- Torso -----
void DrawRobotLowerTorsoSkeleton() {
	glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 1.0f);
	RotateControl(lowerTorsoMove.rotX, lowerTorsoMove.rotY, lowerTorsoMove.rotZ);
	DrawCone(&LowerTorso);
	DrawRobotLowerTorso();
}
void DrawRobotUpperTorsoSkeleton() {
	glTranslatef(0.0f, 0.0f, 1.0f);
	RotateControl(upperTorsoMove.rotX, upperTorsoMove.rotY, upperTorsoMove.rotZ);
	DrawCone(&UpperTorso);
	DrawRobotUpperTorso();
}

// ----- Shoulder Girdle ----- 
void DrawRobotShoulderGirdleSkeleton() {
	//Mid
	glTranslatef(0.0f, 0.0f, 3.0f);
	RotateControl(shoulderGridle.rotX, shoulderGridle.rotY, shoulderGridle.rotZ);

	DrawCone(&ShoulderGirdle);
	//Left
	glRotatef(314.0, 1.0f, 0.0f, 0.0f);
	DrawCone(&ShoulderGirdleSide);
	//Right
	glRotatef(91.0, 1.0f, 0.0f, 0.0f);
	DrawCone(&ShoulderGirdleSide);
	glRotatef(315.0, 1.0f, 0.0f, 0.0f);
	DrawRobotShoulderGirdle();
	glRotatef(-315.0, 1.0f, 0.0f, 0.0f);	// DO NOT REMOVE
}

// ----- Left Arm -----
void DrawRobotLeftShoulderSkeleton() {
	glRotatef(-155.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -1.6f, 0.8f);
	RotateControl(shoulderLeftMove.rotX, shoulderLeftMove.rotY, shoulderLeftMove.rotZ);
	DrawCone(&Shoulder);
	DrawRobotLeftShoulder();
}
void DrawRobotUpperLeftArmSkeleton() {
	glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.45f, 1.2f);
	RotateControl(upperLeftArmMove.rotX, upperLeftArmMove.rotY, upperLeftArmMove.rotZ);
	DrawCone(&UpperArm);
	DrawRobotUpperLeftArm();
}
void DrawRobotLowerLeftArmSkeleton() {
	glTranslatef(0.0f, 0.05f, 2.6f);
	RotateControl(lowerLeftArmMove.rotX, lowerLeftArmMove.rotY, lowerLeftArmMove.rotZ);
	DrawCone(&LowerArm);
	DrawRobotLowerLeftArm();
}

// ----- Right Arm -----
void DrawRobotRightShoulderSkeleton() {
	glRotatef(-295.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 1.7f, 0.8f);
	RotateControl(shoulderRightMove.rotX, shoulderRightMove.rotY, shoulderRightMove.rotZ);
	DrawCone(&Shoulder);
	DrawRobotRightShoulder();
}
void DrawRobotUpperRightArmSkeleton() {
	glRotatef(-20.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -0.45f, 1.2f);
	RotateControl(upperRightArmMove.rotX, upperRightArmMove.rotY, upperRightArmMove.rotZ);
	DrawCone(&UpperArm);
	DrawRobotUpperRightArm();
}
void DrawRobotLowerRightArmSkeleton() {
	glTranslatef(0.0f, 0.0f, 2.6f);
	RotateControl(lowerRightArmMove.rotX, lowerRightArmMove.rotY, lowerRightArmMove.rotZ);
	DrawCone(&LowerArm);
	DrawRobotLowerRightArm();
}

// ----- Head -----
void DrawRobotNeckSkeleton() {
	glRotatef(-45.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 1.4f);
	RotateControl(neckMove.rotX, neckMove.rotY, neckMove.rotZ);
	DrawCone(&Neck);
	DrawRobotNeck();
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -6.35f, 0.0f);
}
void DrawRobotHeadSkeleton() {
	glTranslatef(0.0f, 7.0f, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	RotateControl(headMove.rotX, headMove.rotY, headMove.rotZ);
	DrawCone(&Head);
	DrawRobotHead();
	DrawRobotHead();
}

// Grouped Up Full Robot Skeleton
void LeftLegSkeletonHierachy() {
	DrawRobotUpperLeftLegSkeleton();

	glPushMatrix();
	DrawRobotLowerLeftLegSkeleton();

	glPushMatrix();
	DrawRobotLeftFootSkeleton();

	glPushMatrix();
	DrawRobotLeftToesSkeleton();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}
void RightLegSkeletonHierachy() {
	DrawRobotUpperRightLegSkeleton();

	glPushMatrix();
	DrawRobotLowerRightLegSkeleton();

	glPushMatrix();
	DrawRobotRightFootSkeleton();

	glPushMatrix();
	DrawRobotRightToesSkeleton();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}
void LeftArmSkeletonHierachy() {
	DrawRobotLeftShoulderSkeleton();
	glPushMatrix();
	DrawRobotUpperLeftArmSkeleton();

	glPushMatrix();
	DrawRobotLowerLeftArmSkeleton();
	glPopMatrix();
	glPopMatrix();
}
void RightArmSkeletonHierachy() {
	DrawRobotRightShoulderSkeleton();
	glPushMatrix();
	DrawRobotUpperRightArmSkeleton();

	glPushMatrix();
	DrawRobotLowerRightArmSkeleton();
	glPopMatrix();
	glPopMatrix();
}
void AllUpperBodySkeletonHierachy() {
	DrawRobotLowerTorsoSkeleton();
	glPushMatrix();
	DrawRobotUpperTorsoSkeleton();

	glPushMatrix();
	DrawRobotShoulderGirdleSkeleton();

	// Left Arm
	glPushMatrix();
	LeftArmSkeletonHierachy();
	glPopMatrix();

	// Right Arm
	glPushMatrix();
	RightArmSkeletonHierachy();
	glPopMatrix();

	// Neck and Head
	glPushMatrix();
	DrawRobotNeckSkeleton();

	glPushMatrix();
	DrawRobotHeadSkeleton();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}
void DrawFULLRobot() {

	float ambient[] = { 1, 1, 1, 1 };
	float diffuse[] = { 1, 1, 1, 1 };
	float pos[] = {camera.position.x, camera.position.y, camera.position.z, 1};

	glPushMatrix();
	// Repos
	glRotatef(90, 0, 1, 0);
	glTranslatef(10, 0, 0);

	// Hip
	DrawRobotHipSkeleton();
	//Left Leg
	glPushMatrix();
	LeftLegSkeletonHierachy();
	glPopMatrix();
	// Right Leg
	glPushMatrix();
	RightLegSkeletonHierachy();
	glPopMatrix();
	// Body
	glPushMatrix();
	AllUpperBodySkeletonHierachy();
	glPopMatrix();
	glPopMatrix();
}


// ----- Interactive features -----
int robotCTRL = 0;

void Interactor(Move& J)
{
	switch (J.action) {

	case 1: // X ROTATE +
		J.rotX += J.moveXAmt;
		if (J.rotX > J.maxX) J.rotX = J.maxX;
		J.action = 0;
		break;

	case 2: // X ROTATE -
		J.rotX -= J.moveXAmt;
		if (J.rotX < J.minX) J.rotX = J.minX;
		J.action = 0;
		break;

	case 3: // Y ROTATE +
		J.rotY += J.moveYAmt;
		if (J.rotY > J.maxY) J.rotY = J.maxY;
		J.action = 0;
		break;

	case 4: // Y ROTATE -
		J.rotY -= J.moveYAmt;
		if (J.rotY < J.minY) J.rotY = J.minY;
		J.action = 0;
		break;

	case 5: // Z ROTATE +
		J.rotZ += J.moveZAmt;
		if (J.rotZ > J.maxZ) J.rotZ = J.maxZ;
		J.action = 0;
		break;

	case 6: // Z ROTATE -
		J.rotZ -= J.moveZAmt;
		if (J.rotZ < J.minZ) J.rotZ = J.minZ;
		J.action = 0;
		break;

	case 7: // RESET
		J.rotX = 0.0f;
		J.rotY = 0.0f;
		J.rotZ = 0.0f;
		J.action = 0;
		break;

	case 8: // Back to OtherCTRL
		J.action = 0;
		robotCTRL = 0;
		break;
	}
}
void ResetAllJoints()
{
	int count = sizeof(joints) / sizeof(joints[0]);

	for (int i = 0; i < count; i++) {
		joints[i]->action = 7;   // Tell Interactor to reset
		Interactor(*joints[i]);  // Perform the reset
	}
}

// Grouped Up Interactive features
void RobotInteractiveFeatures() {
	int index = robotCTRL - 1;
	int count = sizeof(joints) / sizeof(joints[0]);
	if (index < 0 || index >= count) return;

	Interactor(*joints[index]);

	switch (robotCTRL) {
	case 1:  Interactor(hipMove); break;

	case 2:  Interactor(upperLeftLegMove); break;
	case 3:  Interactor(lowerLeftLegMove); break;
	case 4:  Interactor(footLeftMove); break;
	case 5:  Interactor(toesLeftMove); break;

	case 6:  Interactor(upperRightLegMove); break;
	case 7:  Interactor(lowerRightLegMove); break;
	case 8:  Interactor(footRightMove); break;
	case 9:  Interactor(toesRightMove); break;

	case 10: Interactor(lowerTorsoMove); break;
	case 11: Interactor(upperTorsoMove); break;

	case 12: Interactor(shoulderGridle); break;

	case 13: Interactor(shoulderLeftMove); break;
	case 14: Interactor(upperLeftArmMove); break;
	case 15: Interactor(lowerLeftArmMove); break;

	case 16: Interactor(shoulderRightMove); break;
	case 17: Interactor(upperRightArmMove); break;
	case 18: Interactor(lowerRightArmMove); break;

	case 19: Interactor(neckMove); break;
	case 20: Interactor(headMove); break;

	default: break;
	}
}
// ----- Animation -----
void RobotFULLAnim() {

}
// ----- Customization feature -----
void ChangeShape() {

}
void ChangeTextureColour() {

}

// Grouped Up Customization feature
void RobotFULLCustomization() {
	ChangeShape();
	ChangeTextureColour();
}

// Texture
void Texture() {

}

// Light
void AmbientLight() {

}
void SpotLight() {

}

// Grouped Up Texture & Light
void RobotFULLTextureNLight() {
	Texture();
	AmbientLight(); SpotLight();
}
Move* selected = nullptr;

//--------------------------------------------------------------------
// FINAL Grouped Up
void RobotAssg() {
	RobotInteractiveFeatures();
	DrawFULLRobot();
	Environment3DCam();
	RobotFULLAnim();
	RobotFULLCustomization();
	RobotFULLTextureNLight();
}

bool robotCtrlCheck() {
	int count = sizeof(joints) / sizeof(joints[0]);
	if (robotCTRL < 1 || robotCTRL > count) return false;
	selected = joints[robotCTRL - 1];
	if (selected == nullptr) return false;
	return true;
}

// ----- PRACTICAL 6 -----
GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };   // Ambient light
GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };   // Diffuse light
GLfloat lightPosition[] = { 1.0f, 1.0f, 1.0f, 0.0f };   // position of light

float lightMoveSpeed = 0.5f;

void lightIntilization() {
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

int lightCTRL;

void LightControls() {
	// Adjust light position based on key press
	switch (lightCTRL) {
	case 1: // W - Move light up
		lightPosition[1] += lightMoveSpeed;
		break;
	case 2: // S - Move light down
		lightPosition[1] -= lightMoveSpeed;
		break;
	case 3: // A - Move light left
		lightPosition[0] -= lightMoveSpeed;
		break;
	case 4: // D - Move light right
		lightPosition[0] += lightMoveSpeed;
		break;
	case 5: // E - Move light near
		lightPosition[2] -= lightMoveSpeed;
		break;
	case 6: // Q - Move light far
		lightPosition[2] += lightMoveSpeed;
		break;
	case 7: // Up - Rotate light clockwise
		glPushMatrix();
		glPushMatrix();
		glRotatef(0.5f, 1.0f, 1.0f, 0.0f);
		glPopMatrix();
		break;
	case 8: // Down - Rotate light counterclockwise
		glPushMatrix();
		glPushMatrix();
		glRotatef(-0.5f, 1.0f, 1.0f, 0.0f);
		glPopMatrix();
		break;
	default:
		break;
	}
	lightCTRL = 0;
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition); // Update light position
}
bool lightOn = true;

void toggleLight() {
	if (lightOn) {
		glDisable(GL_LIGHT0);
		lightOn = false;
	}
	else {
		glEnable(GL_LIGHT0);
		lightOn = true;
	}
}

void Prac6LightControls() {
	switch (lightCTRL) {
		//Practical ONLY
	case 9: //ON OFF
		toggleLight();
//		cout << "light disable then enabled";
		break;
	case 10:
		break;
	case 11:
		break;
	case 12:
		break;
	}

}
void Pyramid() {
	glBegin(GL_TRIANGLES);

	// Front face of the pyramid
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	// Right face of the pyramid
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	// Back face of the pyramid
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	// Left face of the pyramid
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	// Bottom face of the pyramid (base)
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glEnd();
}

void DrawLightsPosition(GLfloat* position) {
	glPushMatrix();
	glTranslatef(position[0], position[1], position[2]);  // Move the sphere to the light position
	//gluSphere(quadric, 0.05f, 10, 10);  // Draw the sphere with radius 0.05
	gluCylinder(quad, 1, 1, 1, 30, 30);
	glPopMatrix();
}
void DisplayPrac6() {
	lightIntilization();
	Prac6LightControls();
	LightControls();
	DrawLightsPosition(lightPosition);
}


//--------------------------------------------------------------------
LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// I ChatGPT'ed this, don't ask me
	case WM_CREATE:
		RAWINPUTDEVICE Rid[1];
		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[0].dwFlags = RIDEV_INPUTSINK; // Receive input even if not in foreground
		Rid[0].hwndTarget = hWnd;
		RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		// I ChatGPT'ed this, don't ask me
	case WM_INPUT:
	{
		UINT dwSize = 0;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == NULL) return 0;

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
		{
			// Error handling
		}

		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			// raw->data.mouse.lLastX and raw->data.mouse.lLastY contain the relative movement
			int deltaX = raw->data.mouse.lLastX;
			int deltaY = raw->data.mouse.lLastY;
			// Use deltaX and deltaY for your application logic

			Camera_Look(camera, deltaX * 0.01f, deltaY * 0.01f);
		}
		delete[] lpb;
		break;
	}

	case WM_KEYDOWN:
		switch (wParam) {
		case '1':
			robotCTRL = 1;     // Hip
			break;
		case '2':
			robotCTRL = 2;     // UpperLeftLeg
			lightCTRL = 2;
			break;
		case '3':
			robotCTRL = 3;     // LowerLeftLeg
			lightCTRL = 3;
			break;
		case '4':
			robotCTRL = 4;     // FootLeft
			lightCTRL = 4;
			break;
		case '5':
			robotCTRL = 5;     // ToesLeft
			lightCTRL = 5;
			break;
		case '6':
			robotCTRL = 6;     // UpperRightLeg
			lightCTRL = 6;
			break;
		case '7':
			robotCTRL = 7;     // LowerRightLeg
			lightCTRL = 7;
			break;
		case '8':
			robotCTRL = 8;     // FootRight
			lightCTRL = 8;
			break;
		case '9':
			robotCTRL = 9;     // ToesRight
			lightCTRL = 9;
			break;
		case 'R':
			robotCTRL = 10;    // LowerTorso
			break;
		case 'T':
			robotCTRL = 11;    // UpperTorso
			break;
		case 'Y':
			robotCTRL = 12;    // ShoulderGirdle
			break;
		case 'U':
			robotCTRL = 13;    // ShoulderLeft
			break;
		case 'I':
			robotCTRL = 14;    // UpperLeftArm
			break;
		case 'O':
			robotCTRL = 15;    // LowerLeftArm
			break;
		case 'P':
			robotCTRL = 16;    // ShoulderRight
			break;
		case 'F':
			robotCTRL = 17;    // UpperRightArm
			break;
		case 'G':
			robotCTRL = 18;    // LowerRightArm
			break;
		case 'H':
			robotCTRL = 19;    // Neck
			break;
		case 'J':
			robotCTRL = 20;    // Head
			break;
		case 'K':
			ResetAllJoints();
			break;
		case VK_LEFT: {
			movePosCTRL = 8;
			if (!robotCtrlCheck()) break;
			selected->action = 2;
		}
					break;
		case VK_RIGHT: {
			movePosCTRL = 7;
			if (!robotCtrlCheck()) break;
			selected = joints[robotCTRL - 1];
			selected->action = 1;
		}
					 break;
		case VK_UP: {
			movePosCTRL = 9;
			if (!robotCtrlCheck()) break;
			selected = joints[robotCTRL - 1];
			selected->action = 3;
		}
				  break;
		case VK_DOWN: {
			movePosCTRL = 10;
			if (!robotCtrlCheck()) break;
			selected = joints[robotCTRL - 1];
			selected->action = 4;
		}
					break;
		case VK_NUMPAD1: {
			movePosCTRL = 12;
			if (!robotCtrlCheck()) break;
			selected = joints[robotCTRL - 1];
			selected->action = 5;
		}
					   break;
		case VK_NUMPAD0: {
			movePosCTRL = 11;
			if (!robotCtrlCheck()) break;
			selected = joints[robotCTRL - 1];
			selected->action = 6;
		}
					   break;
		case VK_SPACE: {
			if (!robotCtrlCheck()) break;
			selected = joints[robotCTRL - 1];
			selected->action = 7;
		}
					 break;

		case 'Z':
			shapeChangeCTRL = 1;
			break;
		case 'X':
			shapeChangeCTRL = 2;
			break;
		case 'C':
			shapeChangeCTRL = 3;
			break;
		case 'V':
			shapeChangeCTRL = 4;
			break;
		case 'B':
			shapeChangeCTRL = 5;
			break;
		case 'N':
			shapeChangeCTRL = 6;
			break;
		case 'M':
			shapeChangeCTRL = 7;
			break;
		case ',':
			shapeChangeCTRL = 8;
			break;
			//case '0': {	// Back to OtherCTRL
			//	if (!robotCtrlCheck()) break;
			//	selected = joints[robotCTRL - 1];
			//	selected->action = 8;
			//}
			//	break;


		case VK_NUMPAD8: movePosCTRL = 3; break;
		case VK_NUMPAD5: movePosCTRL = 4; break;
		case VK_NUMPAD4: movePosCTRL = 2; break;
		case VK_NUMPAD6: movePosCTRL = 1; break;
		case VK_NUMPAD7: movePosCTRL = 6; break;
		case VK_NUMPAD9: movePosCTRL = 5; break;
		case VK_ADD: movePosCTRL = 13; break;
		case VK_SUBTRACT: movePosCTRL = 14; break;



		case 'W': Camera_Move(camera, 0, 0, -1, 0.1f);  break;
		case 'A': Camera_Move(camera, -1, 0, 0, 0.1f); break;
		case 'S': Camera_Move(camera, 0, 0, 1, 0.1f);  break;
		case 'D': Camera_Move(camera, 1, 0, 0, 0.1f); break;
		case 'Q': Camera_Move(camera, 0, 1, 0, 0.1f); break;
		case 'E': Camera_Move(camera, 0, -1, 0, 0.1f); break;
		}
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
//--------------------------------------------------------------------
bool initPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.cAlphaBits = 8;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 0;

	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;

	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	// choose pixel format returns the number most similar pixel format available
	int n = ChoosePixelFormat(hdc, &pfd);

	// set pixel format returns whether it sucessfully set the pixel format
	if (SetPixelFormat(hdc, n, &pfd))
	{
		return true;
	}
	else
	{
		return false;
	}
}
//--------------------------------------------------------------------

void display()
{
	//--------------------------------
	//	OpenGL drawing
	//--------------------------------
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Environment3DCam();
	RobotAssg();
	//--------------------------------
//	End of OpenGL drawing
//--------------------------------
}
//--------------------------------------------------------------------

int main(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = WindowProcedure;
	wc.lpszClassName = WINDOW_TITLE;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc)) return false;

	HWND hWnd = CreateWindow(WINDOW_TITLE, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, NULL, wc.hInstance, NULL);

	//--------------------------------
	//	Initialize window for OpenGL
	//--------------------------------

	HDC hdc = GetDC(hWnd);

	//	initialize pixel format for the window
	initPixelFormat(hdc);

	//	get an openGL context
	HGLRC hglrc = wglCreateContext(hdc);

	//	make context current
	if (!wglMakeCurrent(hdc, hglrc)) return false;

	GL3DInitialization();

	//--------------------------------
	//	End initialization
	//--------------------------------

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		display();

		SwapBuffers(hdc);
	}

	UnregisterClass(WINDOW_TITLE, wc.hInstance);

	return true;
}
//--------------------------------------------------------------------

