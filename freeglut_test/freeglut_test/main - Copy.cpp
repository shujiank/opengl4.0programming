#include "Utils.h"

#include "gumbo.h"

#include "geometry.h"

#define WINDOW_TITLE_PREFIX "tess example"

int
CurrentWidth = 800,
CurrentHeight = 600,
WindowHandle = 0;

unsigned FrameCount = 0;

// for Gumbo
GLsizei VertCount;

// for Icosahedron
static GLsizei IndexCount;


GLuint
	ProjectionMatrixUniformLocation,
	ViewMatrixUniformLocation,
	ModelMatrixUniformLocation,
	PatchMatrix,
	TransposedPatchMatrix,
	LightPosition,
	AmbientMaterial,
	DiffuseMaterial,
	SpecularMaterial,
	Shininess,
	TessLevelInner,
	TessLevelOuter,
	BufferIds[3] = { 0 },
	ShaderIds[6] = { 0 };



Matrix
	ProjectionMatrix,
	ViewMatrix,
	ModelMatrix;

float CubeRotation = 0;
clock_t LastTime = 0;

void Initialize(int, char*[]);
void InitWindow(int, char*[]);
void ResizeFunction(int, int);
void RenderFunction(void);
void TimerFunction(int);
void IdleFunction(void);

void CreateCube(void);
void DestroyCube(void);
void DrawCube(void);
void CreateGumbo(void);
void DrawGumbo(void);
void CreateIcosahedron(void);
void DrawIcosahedron(void);

int main(int argc, char* argv[])
{
	Initialize(argc, argv);

	glutMainLoop();

	exit(EXIT_SUCCESS);
}

void Initialize(int argc, char* argv[])
{
	GLenum GlewInitResult;

	InitWindow(argc, argv);

	GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult) {
		fprintf(
			stderr,
			"ERROR: %s\n",
			glewGetErrorString(GlewInitResult)
		);
		exit(EXIT_FAILURE);
	}

	fprintf(
		stdout,
		"INFO: OpenGL Version: %s\n",
		glGetString(GL_VERSION)
	);

	glGetError();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	ExitOnGLError("ERROR: Could not set OpenGL depth testing options");

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	ExitOnGLError("ERROR: Could not set OpenGL culling options");

	ModelMatrix = IDENTITY_MATRIX;
	ProjectionMatrix = IDENTITY_MATRIX;
	ViewMatrix = IDENTITY_MATRIX;
	TranslateMatrix(&ViewMatrix, 0, 0, -3);
	ProjectionMatrix =
		CreateProjectionMatrix(
			60,
			(float)CurrentWidth / CurrentHeight,
			1.0f,
			100.0f
		);
	CreateCube();
	//CreateGumbo();
	//CreateIcosahedron();
}

void InitWindow(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
	);

	glutInitWindowSize(CurrentWidth, CurrentHeight);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	WindowHandle = glutCreateWindow(WINDOW_TITLE_PREFIX);

	if (WindowHandle < 1) {
		fprintf(
			stderr,
			"ERROR: Could not create a new rendering window.\n"
		);
		exit(EXIT_FAILURE);
	}

	glutReshapeFunc(ResizeFunction);
	glutDisplayFunc(RenderFunction);
	glutIdleFunc(IdleFunction);
	glutTimerFunc(0, TimerFunction, 0);

	//glutCloseFunc(DestroyCube);

}

void ResizeFunction(int Width, int Height)
{
	CurrentWidth = Width;
	CurrentHeight = Height;
	glViewport(0, 0, CurrentWidth, CurrentHeight);

	ProjectionMatrix =
		CreateProjectionMatrix(
			60,
			(float)CurrentWidth / CurrentHeight,
			1.0f,
			100.0f
		);

	glUseProgram(ShaderIds[0]);
	glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, ProjectionMatrix.m);
	glUseProgram(0);
}

void RenderFunction(void)
{
	++FrameCount;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawCube();
	//DrawGumbo();
	//DrawIcosahedron();

	glutSwapBuffers();
	glutPostRedisplay();
}

void IdleFunction(void)
{
	glutPostRedisplay();
}

void TimerFunction(int Value)
{
	if (0 != Value) {
		char* TempString = (char*)
			malloc(512 + strlen(WINDOW_TITLE_PREFIX));

		sprintf(
			TempString,
			"%s: %d Frames Per Second @ %d x %d",
			WINDOW_TITLE_PREFIX,
			FrameCount * 4,
			CurrentWidth,
			CurrentHeight
		);

		glutSetWindowTitle(TempString);
		free(TempString);
	}

	FrameCount = 0;
	glutTimerFunc(250, TimerFunction, 1);
}

void CreateCube(void)
{

	const Vertex VERTICES[8] =
	{
		{ { -.5f, -.5f,  .5f, 1 },{ 0, 0, 1, 1 } },
		{ { -.5f,  .5f,  .5f, 1 },{ 1, 0, 0, 1 } },
		{ { .5f,  .5f,  .5f, 1 },{ 0, 1, 0, 1 } },
		{ { .5f, -.5f,  .5f, 1 },{ 1, 1, 0, 1 } },
		{ { -.5f, -.5f, -.5f, 1 },{ 1, 1, 1, 1 } },
		{ { -.5f,  .5f, -.5f, 1 },{ 1, 0, 0, 1 } },
		{ { .5f,  .5f, -.5f, 1 },{ 1, 0, 1, 1 } },
		{ { .5f, -.5f, -.5f, 1 },{ 0, 0, 1, 1 } }
	};

	const GLuint INDICES[36] =
	{
		0,2,1,  0,3,2,
		4,3,0,  4,7,3,
		4,1,5,  4,0,1,
		3,6,2,  3,7,6,
		1,6,5,  1,2,6,
		7,5,6,  7,4,5
	};

	const int Faces[] = {
		2, 1, 0,
		3, 2, 0,
		4, 3, 0,
		5, 4, 0,
		1, 5, 0,

		11, 6,  7,
		11, 7,  8,
		11, 8,  9,
		11, 9,  10,
		11, 10, 6,

		1, 2, 6,
		2, 3, 7,
		3, 4, 8,
		4, 5, 9,
		5, 1, 10,

		2,  7, 6,
		3,  8, 7,
		4,  9, 8,
		5, 10, 9,
		1, 6, 10 };

	const float Verts[] = {
		0.000f,  0.000f,  1.000f,
		0.894f,  0.000f,  0.447f,
		0.276f,  0.851f,  0.447f,
		-0.724f,  0.526f,  0.447f,
		-0.724f, -0.526f,  0.447f,
		0.276f, -0.851f,  0.447f,
		0.724f,  0.526f, -0.447f,
		-0.276f,  0.851f, -0.447f,
		-0.894f,  0.000f, -0.447f,
		-0.276f, -0.851f, -0.447f,
		0.724f, -0.526f, -0.447f,
		0.000f,  0.000f, -1.000f };

	ShaderIds[0] = glCreateProgram();
	ExitOnGLError("ERROR: Could not create the shader program");


	ShaderIds[1] = LoadShader("SimpleShader.fragment.glsl", GL_FRAGMENT_SHADER);
	ShaderIds[2] = LoadShader("SimpleShader.vertex.glsl", GL_VERTEX_SHADER);
	glAttachShader(ShaderIds[0], ShaderIds[1]);
	glAttachShader(ShaderIds[0], ShaderIds[2]);

	glLinkProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not link the shader program");


	ModelMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ModelMatrix");
	ViewMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ViewMatrix");
	ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ProjectionMatrix");
	ExitOnGLError("ERROR: Could not get the shader uniform locations, 10");

	glGenBuffers(2, &BufferIds[1]);
	ExitOnGLError("ERROR: Could not generate the buffer objects");

	glGenVertexArrays(1, &BufferIds[0]);
	ExitOnGLError("ERROR: Could not generate the VAO");
	glBindVertexArray(BufferIds[0]);
	ExitOnGLError("ERROR: Could not bind the VAO");

	glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(1);
	ExitOnGLError("ERROR: Could not enable vertex attributes");

	glBindBuffer(GL_ARRAY_BUFFER, BufferIds[1]);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Verts), Verts, GL_STATIC_DRAW);
	
	ExitOnGLError("ERROR: Could not bind the VBO to the VAO");

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));
	ExitOnGLError("ERROR: Could not set VAO attributes");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Faces), Faces, GL_STATIC_DRAW);
	ExitOnGLError("ERROR: Could not bind the IBO to the VAO");

	glBindVertexArray(0);

}

void DestroyCube(void)
{
	glDetachShader(ShaderIds[0], ShaderIds[1]);
	glDetachShader(ShaderIds[0], ShaderIds[2]);
	glDeleteShader(ShaderIds[1]);
	glDeleteShader(ShaderIds[2]);
	glDeleteProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not destroy the shaders");

	glDeleteBuffers(2, &BufferIds[1]);
	glDeleteVertexArrays(1, &BufferIds[0]);
	ExitOnGLError("ERROR: Could not destroy the buffer objects");
}

void DrawCube(void)
{
	float CubeAngle;
	clock_t Now = clock();
	if (LastTime == 0)
		LastTime = Now;

	CubeRotation += 45.0f * ((float)(Now - LastTime) / CLOCKS_PER_SEC);
	CubeAngle = DegreesToRadians(CubeRotation);
	LastTime = Now;

	ModelMatrix = IDENTITY_MATRIX;
	RotateAboutY(&ModelMatrix, CubeAngle);
	RotateAboutX(&ModelMatrix, CubeAngle);

	glUseProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not use the shader program");

	glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
	glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
	ExitOnGLError("ERROR: Could not set the shader uniforms");

	glBindVertexArray(BufferIds[0]);
	ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");

	glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, (GLvoid*)0);
	ExitOnGLError("ERROR: Could not draw the cube");

	glBindVertexArray(0);
	glUseProgram(0);
}

void CreateGumbo(void) {

	VertCount = sizeof(PatchData) / (sizeof(float) * 3);
	GLsizei stride = 3 * sizeof(float);
	GLsizei totalSize = stride * VertCount;

	glGenBuffers(1, &BufferIds[1]);

	glGenVertexArrays(1, &BufferIds[0]);
	glBindVertexArray(BufferIds[0]);

	glBindBuffer(GL_ARRAY_BUFFER, BufferIds[1]);
	glBufferData(GL_ARRAY_BUFFER, totalSize, PatchData, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	// load shaders
	ShaderIds[0] = glCreateProgram();
	ExitOnGLError("ERROR: Could not create the shader program");


	ShaderIds[1] = LoadShader("vertexShader.glsl", GL_VERTEX_SHADER);
	ShaderIds[2] = LoadShader("tessControlShader.glsl", GL_TESS_CONTROL_SHADER);
	ShaderIds[3] = LoadShader("tessEvalShader.glsl", GL_TESS_EVALUATION_SHADER);
	ShaderIds[4] = LoadShader("geometryShader.glsl", GL_GEOMETRY_SHADER);
	ShaderIds[5] = LoadShader("fragmentShader.glsl", GL_FRAGMENT_SHADER);
	glAttachShader(ShaderIds[0], ShaderIds[1]);
	glAttachShader(ShaderIds[0], ShaderIds[2]);
	glAttachShader(ShaderIds[0], ShaderIds[3]);
	glAttachShader(ShaderIds[0], ShaderIds[4]);
	glAttachShader(ShaderIds[0], ShaderIds[5]);

	glBindAttribLocation(ShaderIds[0], 0, "Position");

	glLinkProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not link the shader program");


	ModelMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "Model");
	ViewMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "View");
	ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "Projection");

	LightPosition = glGetUniformLocation(ShaderIds[0], "LightPosition");
	AmbientMaterial = glGetUniformLocation(ShaderIds[0], "AmbientMaterial");
	DiffuseMaterial = glGetUniformLocation(ShaderIds[0], "DiffuseMaterial");
	SpecularMaterial = glGetUniformLocation(ShaderIds[0], "SpecularMaterial");
	Shininess = glGetUniformLocation(ShaderIds[0], "Shininess");
	TessLevelInner = glGetUniformLocation(ShaderIds[0], "TessLevelInner");
	TessLevelOuter = glGetUniformLocation(ShaderIds[0], "TessLevelOuter");
	PatchMatrix = glGetUniformLocation(ShaderIds[0], "B");
	TransposedPatchMatrix = glGetUniformLocation(ShaderIds[0], "BT");

	ExitOnGLError("ERROR: Could not get the shader uniform locations");

}

void DrawGumbo(void) {

	glUseProgram(ShaderIds[0]);

	glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, ProjectionMatrix.m);
	glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
	glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);

	glUniform3f(DiffuseMaterial, 0, 0.75, 0.75);
	glUniform3f(AmbientMaterial, 0.04f, 0.04f, 0.04f);
	glUniform3f(SpecularMaterial, 0.5f, 0.5f, 0.5f);
	glUniform1f(Shininess, 50);
	glUniform3f(LightPosition, 0.25f, 0.25f, 1);

	glUniform1f(TessLevelInner, 2);
	glUniform1f(TessLevelOuter, 2);

	float bezier[16] = { -1, 3, -3, 1,
						3, -6, 3, 0,
						-3, 3, 0, 0,
						1, 0, 0, 0 };

	glUniformMatrix4fv(PatchMatrix, 1, GL_FALSE, &bezier[0]);
	glUniformMatrix4fv(TransposedPatchMatrix, 1, GL_TRUE, &bezier[0]);

	ExitOnGLError("ERROR: Could not get the shader uniform locations");

	glBindVertexArray(BufferIds[0]);

	glPatchParameteri(GL_PATCH_VERTICES, 16);
	glDrawArrays(GL_PATCHES, 0, VertCount);

	glBindVertexArray(0);
	glUseProgram(0);
}

void CreateIcosahedron(void) {

	const int Faces[] = {
		2, 1, 0,
		3, 2, 0,
		4, 3, 0,
		5, 4, 0,
		1, 5, 0,

		11, 6,  7,
		11, 7,  8,
		11, 8,  9,
		11, 9,  10,
		11, 10, 6,

		1, 2, 6,
		2, 3, 7,
		3, 4, 8,
		4, 5, 9,
		5, 1, 10,

		2,  7, 6,
		3,  8, 7,
		4,  9, 8,
		5, 10, 9,
		1, 6, 10 };

	const float Verts[] = {
		0.000f,  0.000f,  1.000f,
		0.894f,  0.000f,  0.447f,
		0.276f,  0.851f,  0.447f,
		-0.724f,  0.526f,  0.447f,
		-0.724f, -0.526f,  0.447f,
		0.276f, -0.851f,  0.447f,
		0.724f,  0.526f, -0.447f,
		-0.276f,  0.851f, -0.447f,
		-0.894f,  0.000f, -0.447f,
		-0.276f, -0.851f, -0.447f,
		0.724f, -0.526f, -0.447f,
		0.000f,  0.000f, -1.000f };

	IndexCount = sizeof(Faces) / sizeof(Faces[0]);

	// Create the VAO:
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create the VBO for positions:
	GLuint positions;
	GLsizei stride = 3 * sizeof(float);
	glGenBuffers(1, &positions);
	glBindBuffer(GL_ARRAY_BUFFER, positions);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Verts), Verts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

	// Create the VBO for indices:
	GLuint indices;
	glGenBuffers(1, &indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Faces), Faces, GL_STATIC_DRAW);

	// load shaders

	ShaderIds[0] = glCreateProgram();
	ExitOnGLError("ERROR: Could not create the shader program");

	/*
	ShaderIds[1] = LoadShader("vertexShader.glsl", GL_VERTEX_SHADER);
	ShaderIds[2] = LoadShader("tessControlShader.glsl", GL_TESS_CONTROL_SHADER);
	ShaderIds[3] = LoadShader("tessEvalShader.glsl", GL_TESS_EVALUATION_SHADER);
	ShaderIds[4] = LoadShader("geometryShader.glsl", GL_GEOMETRY_SHADER);
	ShaderIds[5] = LoadShader("fragmentShader.glsl", GL_FRAGMENT_SHADER);

	glAttachShader(ShaderIds[0], ShaderIds[1]);
	glAttachShader(ShaderIds[0], ShaderIds[2]);
	glAttachShader(ShaderIds[0], ShaderIds[3]);
	glAttachShader(ShaderIds[0], ShaderIds[4]);
	glAttachShader(ShaderIds[0], ShaderIds[5]);
	*/

	ShaderIds[1] = LoadShader("SimpleShader.fragment.glsl", GL_FRAGMENT_SHADER);
	ShaderIds[2] = LoadShader("SimpleShader.vertex.glsl", GL_VERTEX_SHADER);

	glAttachShader(ShaderIds[0], ShaderIds[1]);
	glAttachShader(ShaderIds[0], ShaderIds[2]);

	glLinkProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not link the shader program");

	glUseProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not use the shader program");

	ModelMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ModelMatrix");
	ViewMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ViewMatrix");
	ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ProjectionMatrix");

	ExitOnGLError("ERROR: Could not get the shader uniform locations, 2");

	LightPosition = glGetUniformLocation(ShaderIds[0], "LightPosition");
	AmbientMaterial = glGetUniformLocation(ShaderIds[0], "AmbientMaterial");
	DiffuseMaterial = glGetUniformLocation(ShaderIds[0], "DiffuseMaterial");
	TessLevelInner = glGetUniformLocation(ShaderIds[0], "TessLevelInner");
	TessLevelOuter = glGetUniformLocation(ShaderIds[0], "TessLevelOuter");

	ExitOnGLError("ERROR: Could not get the shader uniform locations, 1");
}

void DrawIcosahedron(void) {

	glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, ProjectionMatrix.m);
	glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
	glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);

	glUniform3f(DiffuseMaterial, 0, 0.75, 0.75);
	glUniform3f(AmbientMaterial, 0.04f, 0.04f, 0.04f);
	glUniform3f(LightPosition, 0.25f, 0.25f, 1);

	glUniform1f(TessLevelInner, 2);
	glUniform1f(TessLevelOuter, 2);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glPatchParameteri(GL_PATCH_VERTICES, 3);
	//glDrawElements(GL_PATCHES, IndexCount, GL_UNSIGNED_INT, 0);
	glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, (GLvoid*)0);

}