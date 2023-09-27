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

GLuint texId[10];
const int numoftext = 3;
int thistext = 0;

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
	
	if (key == 'Q')
	{
		
		if (thistext < numoftext-1)
		{
			thistext++;
		}
		else 
		{
			thistext = 0;
		}		
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
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



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
	

	for (int i = 0; i < numoftext; ++i)
	{
		//массив трехбайтных элементов  (R G B)
		RGBTRIPLE* texarray;

		//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
		char* texCharArray;
		int texW, texH;
		std::string s = "texture" + std::to_string(i) + ".bmp";
		OpenGL::LoadBMP(s.data(), &texW, &texH, &texarray);
		OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



		//генерируем ИД для текстуры

		glGenTextures(1, &texId[i]);
		//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
		glBindTexture(GL_TEXTURE_2D, texId[i]);

		//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

		//отчистка памяти
		free(texCharArray);
		free(texarray);

		//наводим шмон
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}


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

void normal(double A[], double B[], double C[], double* tmp)
{
	double a[] = { 0,0,0 };
	double b[] = { 0,0,0 };
	double len;
	for (int i = 0; i < 3; i++)
	{
		a[i] = A[i] - B[i];
		b[i] = C[i] - B[i];
	}
	tmp[0] = a[1] * b[2] - b[1] * a[2];
	tmp[1] = -a[0] * b[2] + b[0] * a[2];
	tmp[2] = a[0] * b[1] - b[0] * a[1];
	len = sqrt(pow(tmp[0], 2) + pow(tmp[1], 2) + pow(tmp[2], 2));
	tmp[0] /= len;
	tmp[1] /= len;
	tmp[2] /= len;
}

#define PI 3.1415927
void figure(int num, double h) {
	double t0, t1, t2, t3,temp;	
	double M[] = { -1.5, 2,0 };
	double A[] = { 2.5, 0, 0 };
	double B[] = { 0.5, 0.5, 0 };
	double C[] = { 0, 4.5, 0 };
	double D[] = { -3, 1.5, 0 };
	double E[] = { -0.5, 0, 0 };
	double F[] = { -2, -3, 0 };
	double G[] = { 0,-3, 0 };
	double H[] = { 0.5,-0.5, 0 };
	double O[] = { 0,0};
	double A1[] = { A[0], A[1], h };
	double B1[] = { B[0], B[1], h };
	double C1[] = { C[0], C[1], h };
	double D1[] = { D[0], D[1], h };
	double E1[] = { E[0], E[1], h };
	double F1[] = { F[0], F[1], h };
	double G1[] = { G[0], G[1], h };
	double H1[] = { H[0], H[1], h };

	double Otex[] = { 3.0 / 51.0,1.0 };
	double rtex = 12.0 / 51.0;
	double angleCtex = acos((15.0 / 51.0 - Otex[0]) / rtex);
	double angleDtex = acos((3.0 / 51.0 - Otex[0]) / rtex);

	double Otex1[] = { 11.0 / 51.0, 21.0 / 51.0 };
	double rtex1 = 4.0 / 51.0;
	double angleFtex = acos((7.0 / 51.0 - Otex1[0]) / rtex1);
	double angleGtex = acos((15.0 / 51.0 - Otex1[0]) / rtex1);

	double Otex2[] = { 48.0 / 51.0,1.0 };
	double rtex2 = 12.0 / 51.0;
	double angleC1tex = acos((36.0 / 51.0 - Otex2[0]) / rtex2);
	double angleD1tex = acos((48.0 / 51.0 - Otex2[0]) / rtex2);

	double Otex3[] = { 40.0 / 51.0, 21.0 / 51.0 };
	double rtex3 = 4.0 / 51.0;
	double angleF1tex = acos((44.0 / 51.0 - Otex3[0]) / rtex3);
	double angleG1tex = acos((36.0 / 51.0 - Otex3[0]) / rtex3);

	double z1 = pow(C[0], 2) + pow(C[1], 2);
	double z2 = pow(D[0], 2) + pow(D[1], 2);
	double z3 = pow(M[0], 2) + pow(M[1], 2);
	double x12 = C[0] - D[0];
	double x23 = D[0] - M[0];
	double x31 = M[0] - C[0];
	double y12 = C[1] - D[1];
	double y23 = D[1] - M[1];
	double y31 = M[1] - C[1];
	double zx = y12 * z3 + y23 * z1 + y31 * z2;
	double zy = x12 * z3 + x23 * z1 + x31 * z2;
	double z = x12 * y31 - y12 * x31;
	O[0] = -1 * zx / (2 * z);
	O[1] = zy / (2 * z);
	double tmp[] = { 0,0,0 };

	double r = sqrt(pow(G[0] - F[0], 2) + pow(G[1] - F[1], 2) + pow(G[2] - F[2], 2)) / 2;
	double r1 = sqrt(pow(C[0], 2) - 2 * C[0] * O[0] + pow(O[0], 2) + pow(C[1], 2) - 2 * C[1] * O[1] + pow(O[1], 2));
	int cnt = 50;
	double angleA = acos((F[0] - (F[0] + G[0]) / 2) / r);
	double angleB = acos((G[0] - (F[0] + G[0]) / 2) / r);
	double angleA1 = acos((C[0] - O[0]) / r1);
	double angleB1 = acos((D[0] - O[0]) / r1);
	switch (num)
	{
	case 1:

		t0 = 16.9 / 51.0;
		t1 = 31.0 / 51.0;	
		
		glBegin(GL_TRIANGLES);
		glTexCoord2d(t0, t1);
		glVertex3dv(H);

		t0 = 24.9 / 51.0;
		t1 = 33.0 / 51.0;
		t2 = 17.0 / 51.0;
		t3 = 35.0 / 51.0;

		glTexCoord2d(t0,t1);
		glVertex3dv(A);	
		glTexCoord2d(t2, t3);
		glVertex3dv(B);	
		

		t0 = 14.895 / 51.0;
		t1 = 1.0;		
		t2 = 16.9 / 51.0;
		t3 = 35.0 / 51.0;

		glTexCoord2d(t2, t3);
		glVertex3dv(B);
		glTexCoord2d(t0, t1);
		glVertex3dv(C);	
		t0 = 12.8 / 51.0;
		t1 = 33.0 / 51.0;
		glTexCoord2d(t0, t1);
		glVertex3dv(E);
		
		glEnd();


		glBegin(GL_TRIANGLE_FAN);	
		t0 = 13.0 / 51.0;
		t1 = 33.0 / 51.0;
		glTexCoord2d(t0, t1);
		glVertex3dv(E);
		for (double i = -angleA1; i >= -angleB1 - 0.01; i -= 0.01)
		{
			glTexCoord2d(Otex[0] + rtex * cos(angleCtex), Otex[1] + rtex * sin(angleCtex));
			glVertex3d(O[0] + r1 * cos(i), O[1] + r1 * sin(i), 0);
			angleCtex -= 0.01;
		}
		glEnd();

		t0 = 13.0 / 51.0;
		t1 = 33.0 / 51.0;

		glBegin(GL_TRIANGLE_FAN);	
		glTexCoord2d(t0, t1);
		glVertex3dv(E);		
		for (double i = -angleA; i <= angleB+0.01; i += 0.01)
		{
			glTexCoord2d(Otex1[0] + rtex1 * cos(angleFtex), Otex1[1] + rtex1 * sin(angleFtex));
			double O1[] = { (F[0] + G[0]) / 2 + r * cos(i), (F[1] + G[1]) / 2 + r * sin(i), 0 };
			glVertex3dv(O1);
			angleFtex += 0.01;
		}		
		glEnd();

		glBegin(GL_TRIANGLES);
		t0 = 14.9 / 51.0;
		t1 = 21.0 / 51.0;
		t2 = 16.8 / 51.0;
		t3 = 31.0 / 51.0;
		glTexCoord2d(t0, t1);
		glVertex3dv(G);
		glTexCoord2d(t2, t3);
		glVertex3dv(H);

		t0 = 13.0 / 51.0;
		t1 = 33.0 / 51.0;
		t2 = 17.0 / 51.0;
		t3 = 31.0 / 51.0;
		glTexCoord2d(t0, t1);
		glVertex3dv(E);

		glTexCoord2d(t2, t3);
		glVertex3dv(H);
			
		t0 = 17.0 / 51.0;
		t1 = 35.0 / 51.0;
		t2 = 13.0 / 51.0;
		t3 = 33.0 / 51.0;
		glTexCoord2d(t0, t1);
		glVertex3dv(B);
		glTexCoord2d(t2, t3);
		glVertex3dv(E);
		glEnd();
		break;
	case 2:		
		glPushMatrix();

		t0 = 4.1 / 51.0;
		t1 = 8.0 / 51.0;
		t2 = 40.0 / 51.0;
		t3 = 47.9 / 51.0;
		
		glBegin(GL_QUADS);		
		glColor3d(1, 0, 1);		
		normal(B, A, A1, tmp);
		glNormal3dv(tmp);
		glTexCoord2d(t2, t0);
		glVertex3dv(A);
		glTexCoord2d(t3, t0);
		glVertex3dv(B);
		glTexCoord2d(t3, t1);
		glVertex3dv(B1);
		glTexCoord2d(t2, t1);
		glVertex3dv(A1);		

		t0 = 10.1 / 51.0;
		t1 = 14.0 / 51.0;
		t2 = 2.0 / 51.0;
		t3 = 18.0 / 51.0;
		
		normal(C, B, B1, tmp);
		glNormal3dv(tmp);
		glTexCoord2d(t2, t0);
		glVertex3dv(B);
		glTexCoord2d(t3, t0);
		glVertex3dv(C);
		glTexCoord2d(t3, t1);
		glVertex3dv(C1);
		glTexCoord2d(t2, t1);
		glVertex3dv(B1);

		t0 = 10.1 / 51.0;
		t1 = 14.0 / 51.0;
		t2 = 20.0 / 51.0;
		t3 = 31.9 / 51.0;


		normal(E, D, D1, tmp);
		glNormal3dv(tmp);
		glTexCoord2d(t2, t0);
		glVertex3dv(D);
		glTexCoord2d(t3, t0);
		glVertex3dv(E);
		glTexCoord2d(t3, t1);
		glVertex3dv(E1);
		glTexCoord2d(t2, t1);
		glVertex3dv(D1);

		t0 = 4.1 / 51.0;
		t1 = 8.1 / 51.0;
		t2 = 2.0 / 51.0;
		t3 = 15.0 / 51.0;

		normal(F, E, E1, tmp);
		glNormal3dv(tmp);
		glTexCoord2d(t2, t0);
		glVertex3dv(E);
		glTexCoord2d(t3, t0);
		glVertex3dv(F);
		glTexCoord2d(t3, t1);
		glVertex3dv(F1);
		glTexCoord2d(t2, t1);
		glVertex3dv(E1);

		t0 = 4.1 / 51.0;
		t1 = 8.0 / 51.0;
		t2 = 17.0 / 51.0;
		t3 = 26.9 / 51.0;

		normal(H, G, G1, tmp);
		glNormal3dv(tmp);
		glTexCoord2d(t2, t0);
		glVertex3dv(G);
		glTexCoord2d(t3, t0);
		glVertex3dv(H);
		glTexCoord2d(t3, t1);
		glVertex3dv(H1);
		glTexCoord2d(t2, t1);
		glVertex3dv(G1);

		t0 = 4.1 / 51.0;
		t1 = 8.0 / 51.0;
		t2 = 30.0 / 51.0;
		t3 = 37.9 / 51.0;

		normal(A, H, H1, tmp);
		glNormal3dv(tmp);
		glTexCoord2d(t2, t0);
		glVertex3dv(H);
		glTexCoord2d(t3, t0);
		glVertex3dv(A);
		glTexCoord2d(t3, t1);
		glVertex3dv(A1);
		glTexCoord2d(t2, t1);
		glVertex3dv(H1);
		glEnd();			
		
		t0 = 10.3 / 51.0;
		t1 = 14.0 / 51.0;
		t2 = 34.0 / 51.0;
		t3 = 46.9 / 51.0;

		
		temp = sqrt(pow(t3-t2,2)+pow(t0-t0,2))/((angleA - angleB) / 0.01);
		glBegin(GL_QUADS);
		for (double i = -angleA; i <= angleB; i+= 0.01)
		{
			double O1[] = { (F[0] + G[0]) / 2 + r * cos(i), (F[1] + G[1]) / 2 + r * sin(i), 0 };
			double O2[] = { (F[0] + G[0]) / 2 + r * cos(i), (F[1] + G[1]) / 2 + r * sin(i), h };
			double O3[] = { (F[0] + G[0]) / 2 + r * cos(i + 0.01), (F[1] + G[1]) / 2 + r * sin(i + 0.01), h };
			double O4[] = { (F[0] + G[0]) / 2 + r * cos(i + 0.01), (F[1] + G[1]) / 2 + r * sin(i + 0.01), 0};
			t3 = t2 + temp;
			normal(O3, O1, O2, tmp);
			glNormal3dv(tmp);
			glTexCoord2d(t2, t0);			
			glVertex3dv(O1);
			glTexCoord2d(t2, t1);
			glVertex3dv(O2);
			glTexCoord2d(t3, t1);
			glVertex3dv(O3);
			glTexCoord2d(t3, t0);
			glVertex3dv(O4);
			t2 = t3;
		}
		glEnd();

		t0 = 15.1 / 51.0;
		t1 = 19.0 / 51.0;
		t2 = 15.0 / 51.0;
		t3 = 34.9 / 51.0;

		temp = sqrt(pow(t3 - t2, 2) + pow(t0 - t0, 2)) / ((angleB1-angleA1)/ 0.01);
		glBegin(GL_QUADS);
		for (double i = -angleB1; i <= -angleA1; i += 0.01)
		{
			double O11[] = { O[0] + r1 * cos(i), O[1] + r1 * sin(i), 0 };
			double O22[] = { O[0] + r1 * cos(i), O[1] + r1 * sin(i), h };
			double O33[] = { O[0] + r1 * cos(i + 0.01), O[1] + r1 * sin(i + 0.01), h };
			double O44[] = { O[0] + r1 * cos(i + 0.01), O[1] + r1 * sin(i + 0.01), 0 };
			t2 = t3 - temp;
			normal(O22, O11, O33, tmp);
			glNormal3dv(tmp);
			glTexCoord2d(t3, t0);
			glVertex3dv(O11);
			glTexCoord2d(t3, t1);
			glVertex3dv(O22);
			glTexCoord2d(t2, t1);
			glVertex3dv(O33);
			glTexCoord2d(t2, t0);
			glVertex3dv(O44);
			t3 = t2;
		}
		glEnd();
		glPopMatrix();
		break;
     case 3:			
			glBegin(GL_TRIANGLES);
			t0 = 26.0 / 51.0;
			t1 = 33.0 / 51.0;
			t2 = 34.0 / 51.0;
			t3 = 35.0 / 51.0;
			glTexCoord2d(t0, t1);
			glVertex3dv(A);
			glTexCoord2d(t2, t3);
			glVertex3dv(B);
			t0 = 34.0 / 51.0;
			t1 = 31.0 / 51.0;
			t2 = 34.0 / 51.0;
			t3 = 35.0 / 51.0;
			glTexCoord2d(t0, t1);
			glVertex3dv(H);

			glTexCoord2d(t2, t3);
			glVertex3dv(B);
			t0 = 36.0 / 51.0;
			t1 = 1.0;
			t2 = 38.0 / 51.0;
			t3 = 33.0 / 51.0;
			glTexCoord2d(t0, t1);
			glVertex3dv(C);
			glTexCoord2d(t2, t3);
			glVertex3dv(E);

			t0 = 36.0 / 51.0;
			t1 = 21.0 / 51.0;
			t2 = 34.0 / 51.0;
			t3 = 31.0 / 51.0;
			glTexCoord2d(t0, t1);
			glVertex3dv(G);
			glTexCoord2d(t2, t3);
			glVertex3dv(H);
			t0 = 38.0 / 51.0;
			t1 = 33.0 / 51.0;
			t2 = 34.0 / 51.0;
			t3 = 31.0 / 51.0;
			glTexCoord2d(t0, t1);
			glVertex3dv(E);

			glTexCoord2d(t2, t3);
			glVertex3dv(H);
			t0 = 34.0 / 51.0;
			t1 = 35.0 / 51.0;
			t2 = 38.0 / 51.0;
			t3 = 33.0 / 51.0;
			glTexCoord2d(t0, t1);
			glVertex3dv(B);
			glTexCoord2d(t2, t3);
			glVertex3dv(E);
			glEnd();

						
			t0 = 37.8 / 51.0;
			t1= 33.0 / 51.0;
			glBegin(GL_TRIANGLE_FAN);
			glTexCoord2d(t0, t1);
			glVertex3dv(E);
			for (double i = -angleA1; i >= -angleB1-0.01; i -= 0.01)
			{
				glTexCoord2d(Otex2[0] + rtex2 * cos(angleC1tex), Otex2[1] + rtex2 * sin(angleC1tex));
				glVertex3d(O[0] + r1 * cos(i), O[1] + r1 * sin(i), 0);
				angleC1tex += 0.01;
			}
			glEnd();

			t0 = 37.8 / 51.0;
			t1 = 33.0 / 51.0;
			glBegin(GL_TRIANGLE_FAN);
			glTexCoord2d(t0, t1);
			glVertex3dv(E);
			for (double i = -angleB; i >= -angleA - 0.01; i -= 0.01)
			{
				glTexCoord2d(Otex3[0] + rtex3 * cos(angleG1tex), Otex3[1] + rtex3 * sin(angleG1tex));
				double O1[] = { (F[0] + G[0]) / 2 + r * cos(i), (F[1] + G[1]) / 2 + r * sin(i), 0 };
				glVertex3dv(O1);
				angleG1tex += 0.01;
			}
			glEnd();
			break;
	}
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


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


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
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина
	double h = 1.0;		
	glNormal3d(0, 0, -1);
	glBindTexture(GL_TEXTURE_2D, texId[thistext]);
	glColor3d(1, 0.627, 0.478);
	figure(3, h);
	figure(2, h);
	glTranslated(0, 0, h);
	glNormal3d(0, 0, 1);
	glColor4d(1, 0.627, 0.478, 0.6);
	figure(1, h);

	
   //Сообщение вверху экрана

	
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
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "Q - смена текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}