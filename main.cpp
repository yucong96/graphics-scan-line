#include "head.h"
#include "io/obj.h"
#include "scan-line/scan-line.h"
#include "opengl/View.h"

using namespace std;
using namespace Eigen;

int width = 600;
int height = 600;

View view;
Object obj;
StaticTable s_table;
ActivateTable4ISL a_table;

void init(const string filepath)
{
	Color background;
	background.r = 0;
	background.g = 255;
	background.b = 0;
	glClearColor(0.0, 0.0, 0.0, 1.0);

	obj.obj2tri(filepath);
	Vector3f bound_box = obj.bound_box();
	double radio = 1.0 * width / height;

	GLfloat left, right, bottom, top;
	double rate = 1.5;
	if (radio < bound_box[0] / bound_box[1]) {
		left = -bound_box[0] * rate, right = bound_box[0] * rate;
		bottom = left * radio, top = right * radio;
	}
	else {
		bottom = -bound_box[1] * rate, top = bound_box[1] * rate;
		left = bottom / radio, right = top / radio;
	}
	GLfloat _near = 0, _far = 25;
	view.set_radius(bound_box.maxCoeff() * 2.0);
	view.set_alpha(50 * PI / 36);
	view.set_phi(3 * PI / 36);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, width-1, 0, height-1);
	
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(view.eye[0], view.eye[1], view.eye[2], view.center[0], view.center[1], view.center[2], view.up[0], view.up[1], view.up[2]);
	MatrixXf model_mat;
	model_mat.resize(4, 4);
	glGetFloatv(GL_MODELVIEW_MATRIX, &model_mat(0, 0));
	glLoadIdentity();

	obj.apply_mat(model_mat, "new");

	s_table = StaticTable(left, right, bottom, top, _near, _far, width, height);
	s_table.add_obj(obj);
	a_table = ActivateTable4ISL(s_table);
	a_table.background_color = background;
	a_table.traverse_display(s_table, view);
	glFlush();
	return;
}

void set_view() {
	cout << view.alpha / PI * 36 << " " << view.phi / PI * 36 << endl;
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(view.eye[0], view.eye[1], view.eye[2], view.center[0], view.center[1], view.center[2], view.up[0], view.up[1], view.up[2]);
	MatrixXf model_mat;
	model_mat.resize(4, 4);
	glGetFloatv(GL_MODELVIEW_MATRIX, &model_mat(0, 0));
	glLoadIdentity();

	obj.apply_mat(model_mat, "new");

	s_table.reset();
	s_table.add_obj(obj);
	a_table.reset();
	a_table.traverse_display(s_table, view);
	glFlush();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	return;
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a': 
		view.set_alpha(view.alpha + PI / 36);
		break;
	case 'd':
		view.set_alpha(view.alpha - PI / 36);
		break;
	case 'w':
		view.set_phi(view.phi + PI / 36);
		break;
	case 's':
		view.set_phi(view.phi - PI / 36);
		break;
	default: 
		break;
	}
	set_view();
	//display();
}

int main(int argc, char *argv[])
{
	string filepath;
	if (argc == 1) {
		filepath = "models/cube.obj";
	}
	if (argc == 2) {
		filepath = argv[1];
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	glutCreateWindow("OpenGL 3D View");
	init(filepath);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}
