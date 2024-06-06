#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;
bool textureReplace = true;		// (Н) Эта переменная отвечает за смену текстур

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	if (key == 'E')											// (Н) Смена текстуры происходит по этой кнопке
	{
		textureReplace = !textureReplace;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;
GLuint texId2;									// (Н) Я хз как по другому делать, поэтому я просто скопировал часть кода, ко всем переменным в конце добавил "2", и всё работает

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)																						(Н)	Код брался и копировался отсюда
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);					// (Н) Чтоб не мучаться с картинками, я коприовал старую и рисовал на ней
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);


	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);																// (Н) И досюда


	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE* texarray2;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray2;
	int texW2, texH2;
	OpenGL::LoadBMP("texture1.bmp", &texW2, &texH2, &texarray2);
	OpenGL::RGBtoChar(texarray2, texW2, texH2, &texCharArray2);


	//генерируем ИД для текстуры
	glGenTextures(1, &texId2);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId2);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW2, texH2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);


	//отчистка памяти
	free(texCharArray2);
	free(texarray2);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}


double N_VectorX(double A[], double B[], double C[]) {
	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[0] = N_X / Abs_Vector;
	return N_Vector[0];
}
double N_VectorY(double A[], double B[], double C[]) {

	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[1] = N_Y / Abs_Vector;
	return N_Vector[1];
}
double N_VectorZ(double A[], double B[], double C[]) {
	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[2] = N_Z / Abs_Vector;
	return N_Vector[2];
}


void Render(OpenGL *ogl)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	if (textureReplace)														// (Н) Здесь переключаются текстуры: textureReplace - создаётся на 20 строчке
		glBindTexture(GL_TEXTURE_2D, texId);								// (Н) Назначение кнопки на 225
	else 
		glBindTexture(GL_TEXTURE_2D, texId2);

	
	//альфаналожение
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	glDisable(GL_BLEND);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут						(Н) это обман, пришлось прогать не только тут


	//Начало рисования квадратика станкина
	double A[] = { 2, 0, 0 };
	double B[] = { 0, -6,0 };
	double C[] = { 7, -10,0 };
	double D[] = { 2,-15,0 };
	double E[] = { -2,-8,0 };
	double F[] = { -9,-8,0 };
	double G[] = { -8,-4,0 };
	double H[] = { -2,-5,0 };

	double A1[] = { 2, 0, 10 };
	double B1[] = { 0, -6,10 };
	double C1[] = { 7,-10,10 };
	double D1[] = { 2,-15,10 };
	double E1[] = { -2,-8,10 };
	double F1[] = { -9,-8,10 };
	double G1[] = { -8,-4,10 };
	double H1[] = { -2,-5,10 };


	

	glColor3d(0.7, 0.7, 0.7);

	double xx, yy, zz;

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);					// (Н) Это нормаль
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(H1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(B1);
	glVertex3dv(D1);
	glVertex3dv(E1);
	glEnd();


	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(B1);
	glVertex3dv(E1);
	glVertex3dv(H1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(F1);
	glVertex3dv(H1);
	glVertex3dv(E1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(F1);
	glVertex3dv(G1);
	glVertex3dv(H1);
	glEnd();


	//////////////////////////////////////////


	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);					// (Н) Это нормаль
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(H);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(B);
	glVertex3dv(D);
	glVertex3dv(E);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(B);
	glVertex3dv(E);
	glVertex3dv(H);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(F);
	glVertex3dv(H);
	glVertex3dv(E);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(F);
	glVertex3dv(G);
	glVertex3dv(H);
	glEnd();

	
	//////////////////////////////////////////////////

	xx = N_VectorX(A1, A, H);
	yy = N_VectorY(A1, A, H);
	
	glBegin(GL_QUADS);
		glNormal3d(xx, yy, 0);
		glColor3f(0.5f, 0.5f, 0.5f);
		glVertex3dv(A1);
		glVertex3dv(H1);
		glVertex3dv(H);
		glVertex3dv(A);
	glEnd();

	xx = N_VectorX(B1, B, A);
	yy = N_VectorY(B1, B, A);

	glBegin(GL_QUADS);
	glNormal3d(xx, yy, 0);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(B1);
	glVertex3dv(A1);
	glVertex3dv(A);
	glVertex3dv(B);
	glEnd();

	xx = N_VectorX(C1, C, B);
	yy = N_VectorY(C1, C, B);

	glBegin(GL_QUADS);
	glNormal3d(xx, yy, 0);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(C);
	glVertex3dv(B);
	glEnd();

	xx = N_VectorX(D1, D, C);
	yy = N_VectorY(D1, D, C);

	glBegin(GL_QUADS);
	glNormal3d(xx, yy, 0);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(D1);
	glVertex3dv(C1);
	glVertex3dv(C);
	glVertex3dv(D);
	glEnd();

	xx = N_VectorX(E1, E, D);
	yy = N_VectorY(E1, E, D);

	glBegin(GL_QUADS);
	glNormal3d(xx, yy, 0);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(D1);
	glVertex3dv(E1);
	glVertex3dv(E);
	glVertex3dv(D);
	glEnd();

	xx = N_VectorX(F1, F, E);
	yy = N_VectorY(F1, F, E);

	glBegin(GL_QUADS);
	glNormal3d(xx, yy, 0);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(F1);
	glVertex3dv(E1);
	glVertex3dv(E);
	glVertex3dv(F);
	glEnd();

	xx = N_VectorX(G1, G, F);
	yy = N_VectorY(G1, G, F);

	glBegin(GL_QUADS);
	glNormal3d(xx, yy, 0);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(F1);
	glVertex3dv(G1);
	glVertex3dv(G);
	glVertex3dv(F);
	glEnd();

	xx = N_VectorX(H1, H, G);
	yy = N_VectorY(H1, H, G);

	glBegin(GL_QUADS);
	glNormal3d(xx, yy, 0);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex3dv(H1);
	glVertex3dv(G1);
	glVertex3dv(G);
	glVertex3dv(H);
	glEnd();



	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 200 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "E - Переключение текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	ss << "UV-развёртка" << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}