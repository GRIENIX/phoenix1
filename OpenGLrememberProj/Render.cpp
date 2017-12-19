#include "Render.h"




#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h"

GuiTextRectangle rec;

bool textureMode = true;
bool lightMode = true;


//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //массивчик для десяти шейдеров
Shader frac;
Shader cassini;




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
	virtual void SetUpCamera()
	{

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
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//класс недоделан!
class WASDcamera :public CustomCamera
{
public:
		
	float camSpeed;

	WASDcamera()
	{
		camSpeed = 0.4;
		pos.setCoords(5, 5, 5);
		lookPoint.setCoords(0, 0, 0);
		normal.setCoords(0, 0, 1);
	}

	virtual void SetUpCamera()
	{

		if (OpenGL::isKeyPressed('W'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*camSpeed;
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}
		if (OpenGL::isKeyPressed('S'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*(-camSpeed);
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}

		LookAt();
	}

} WASDcam;


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
		/*
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);
		
		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();*/

		//if (OpenGL::isKeyPressed('G'))
		//{
		//	glColor3d(0, 0, 0);
		//	//линия от источника света до окружности
		//		glBegin(GL_LINES);
		//	glVertex3d(pos.X(), pos.Y(), pos.Z());
		//	glVertex3d(pos.X(), pos.Y(), 0);
		//	glEnd();

		//	//рисуем окруность
		//	Circle c;
		//	c.pos.setCoords(pos.X(), pos.Y(), 0);
		//	c.scale = c.scale*1.5;
		//	c.Show();
		//}
		/*
		if (f1)
			glEnable(GL_LIGHTING);
		if (f2)
			glEnable(GL_TEXTURE_2D);
		if (f3)
			glEnable(GL_DEPTH_TEST);
			*/
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




float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//обработчик движения мыши
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


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0*dx/ogl->getWidth()/zoom;
		offsetY += 1.0*dy/ogl->getHeight()/zoom;
	}


	//
	////двигаем свет по плоскости, в точку где мышь
	//if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	//{
	//	LPPOINT POINT = new tagPOINT();
	//	GetCursorPos(POINT);
	//	ScreenToClient(ogl->getHwnd(), POINT);
	//	POINT->y = ogl->getHeight() - POINT->y;

	//	Ray r = camera.getLookRay(POINT->x, POINT->y,60,ogl->aspect);

	//	double z = light.pos.Z();

	//	double k = 0, x = 0, y = 0;
	//	if (r.direction.Z() == 0)
	//		k = 0;
	//	else
	//		k = (z - r.origin.Z()) / r.direction.Z();

	//	x = k*r.direction.X() + r.origin.X();
	//	y = k*r.direction.Y() + r.origin.Y();

	//	light.pos = Vector3(x, y, z);
	//}

	//if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	//{
	//	light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	//}

	
}

//обработчик вращения колеса  мыши
void mouseWheelEvent(OpenGL *ogl, int delta)
{


	float _tmpZ = delta*0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float psi;               
float hx=0, hy=0;
float dx = 0,dy=0;
float fi = 0;			  //угол
int t = 0;				  //взмахи крыльев
int q = 0;				  //поворот
bool w = 1, a=0, d=0;     //движение
float y = 0,x=0;          //положение
bool go = 0;              //начать движение

//обработчик нажатия кнопок клавиатуры
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

	if (key == 'Y')
	{
		frac.LoadShaderFromFile();
		frac.Compile();

		s[0].LoadShaderFromFile();
		s[0].Compile();

		cassini.LoadShaderFromFile();
		cassini.Compile();
	}

	if (key == 'Q')
		Time = 0;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (ogl->isKeyPressed('W')) {
		a = 0;
		d = 0;
		w = 1;
		q = 0;
		go = 1;
		
		if (go) {
			dy += cos((psi*3.14) / 180);
			dx += sin((psi*3.14) / 180);
		}
	}

	if (ogl->isKeyPressed('A')) {
		a = 1;
		go = 0;
		w = 0;
		d = 0;
		q = 0;
		
	}
	

	if (ogl->isKeyPressed('D')) {
		d = 1;
		go = 0;
		w = 0;
		q = 0;
		a = 0;
		
	}
	

}

void keyUpEvent(OpenGL *ogl, int key)
{

}


void DrawQuad()
{
	double A[] = { 0,0 };
	double B[] = { 1,0 };
	double C[] = { 1,1 };
	double D[] = { 0,1 };
	glBegin(GL_QUADS);
	glColor3d(.5, 0, 0);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);
	glEnd();
}


ObjFile objModel,monkey;
ObjFile phoenix_mask[21],	phoenix_wings[21],	phoenix_body[21],  phoenix_move_left_mask[3], phoenix_move_left_body[3], phoenix_move_left_wings[3], phoenix_move_right_mask[3], phoenix_move_right_body[3], phoenix_move_right_wings[3];
Texture monkeyTex,     phoenixTex_body,      phoenixTex_mask,        phoenixTex_wings;

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
	
	


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
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

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   загрузка текстуры из файла
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	frac.FshaderFileName = "shaders\\frac.frag"; //имя файла фрагментного шейдера
	frac.LoadShaderFromFile(); //загружаем шейдеры из файла
	frac.Compile(); //компилируем

	cassini.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	cassini.FshaderFileName = "shaders\\cassini.frag"; //имя файла фрагментного шейдера
	cassini.LoadShaderFromFile(); //загружаем шейдеры из файла
	cassini.Compile(); //компилируем
	

	s[0].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\light.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	s[1].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //имя файла фрагментного шейдера
	s[1].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[1].Compile(); //компилируем

	


	/*loadModel("obj\\lpgun6.obj", &objModel);*/
	
	loadModel("poses//mask0.obj", &phoenix_mask[0]);  //модели маски
	loadModel("poses//mask5.obj", &phoenix_mask[1]);
	loadModel("poses//mask10.obj", &phoenix_mask[2]);
	loadModel("poses//mask14.obj", &phoenix_mask[3]);
	loadModel("poses//mask18.obj", &phoenix_mask[4]);
	loadModel("poses//mask24.obj", &phoenix_mask[5]);
	loadModel("poses//mask28.obj", &phoenix_mask[6]);
	loadModel("poses//mask31.obj", &phoenix_mask[7]);
	loadModel("poses//mask34.obj", &phoenix_mask[8]);
	loadModel("poses//mask38.obj", &phoenix_mask[9]);
	loadModel("poses//mask42.obj", &phoenix_mask[10]);
	loadModel("poses//mask47.obj", &phoenix_mask[11]);
	loadModel("poses//mask52.obj", &phoenix_mask[12]);
	loadModel("poses//mask57.obj", &phoenix_mask[13]);
	loadModel("poses//mask63.obj", &phoenix_mask[14]);
	loadModel("poses//mask70.obj", &phoenix_mask[15]);
	loadModel("poses//mask78.obj", &phoenix_mask[16]);
	loadModel("poses//mask88.obj", &phoenix_mask[17]);
	loadModel("poses//mask98.obj", &phoenix_mask[18]);
	loadModel("poses//mask108.obj", &phoenix_mask[19]);
	loadModel("poses//mask120.obj", &phoenix_mask[20]);

	loadModel("poses//body0.obj", &phoenix_body[0]);  //модели тела
	loadModel("poses//body5.obj", &phoenix_body[1]);
	loadModel("poses//body10.obj", &phoenix_body[2]);
	loadModel("poses//body14.obj", &phoenix_body[3]);
	loadModel("poses//body18.obj", &phoenix_body[4]);
	loadModel("poses//body24.obj", &phoenix_body[5]);
	loadModel("poses//body28.obj", &phoenix_body[6]);
	loadModel("poses//body31.obj", &phoenix_body[7]);
	loadModel("poses//body34.obj", &phoenix_body[8]);
	loadModel("poses//body38.obj", &phoenix_body[9]);
	loadModel("poses//body42.obj", &phoenix_body[10]);
	loadModel("poses//body47.obj", &phoenix_body[11]);
	loadModel("poses//body52.obj", &phoenix_body[12]);
	loadModel("poses//body57.obj", &phoenix_body[13]);
	loadModel("poses//body63.obj", &phoenix_body[14]);
	loadModel("poses//body70.obj", &phoenix_body[15]);
	loadModel("poses//body78.obj", &phoenix_body[16]);
	loadModel("poses//body88.obj", &phoenix_body[17]);
	loadModel("poses//body98.obj", &phoenix_body[18]);
	loadModel("poses//body108.obj", &phoenix_body[19]);
	loadModel("poses//body120.obj", &phoenix_body[20]);


	loadModel("poses//wings0.obj", &phoenix_wings[0]);   //модели крыльев и хвоста
	loadModel("poses//wings5.obj", &phoenix_wings[1]);
	loadModel("poses//wings10.obj", &phoenix_wings[2]);
	loadModel("poses//wings14.obj", &phoenix_wings[3]);
	loadModel("poses//wings18.obj", &phoenix_wings[4]);
	loadModel("poses//wings24.obj", &phoenix_wings[5]);
	loadModel("poses//wings28.obj", &phoenix_wings[6]);
	loadModel("poses//wings31.obj", &phoenix_wings[7]);
	loadModel("poses//wings34.obj", &phoenix_wings[8]);
	loadModel("poses//wings38.obj", &phoenix_wings[9]);
	loadModel("poses//wings42.obj", &phoenix_wings[10]);
	loadModel("poses//wings47.obj", &phoenix_wings[11]);
	loadModel("poses//wings52.obj", &phoenix_wings[12]);
	loadModel("poses//wings57.obj", &phoenix_wings[13]);
	loadModel("poses//wings63.obj", &phoenix_wings[14]);
	loadModel("poses//wings70.obj", &phoenix_wings[15]);
	loadModel("poses//wings78.obj", &phoenix_wings[16]);
	loadModel("poses//wings88.obj", &phoenix_wings[17]);
	loadModel("poses//wings98.obj", &phoenix_wings[18]);
	loadModel("poses//wings108.obj", &phoenix_wings[19]);
	loadModel("poses//wings120.obj", &phoenix_wings[20]);


	                                                                    //поворот маски
	loadModel("move_left//mask0.obj", &phoenix_move_left_mask[0]);
	loadModel("move_left//mask1.obj", &phoenix_move_left_mask[1]);
	loadModel("move_left//mask2.obj", &phoenix_move_left_mask[2]);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	loadModel("move_right//mask0.obj", &phoenix_move_right_mask[0]);
	loadModel("move_right//mask1.obj", &phoenix_move_right_mask[1]);
	loadModel("move_right//mask2.obj", &phoenix_move_right_mask[2]);

																		//поворот тела
	loadModel("move_left//body0.obj", &phoenix_move_left_body[0]);
	loadModel("move_left//body1.obj", &phoenix_move_left_body[1]);
	loadModel("move_left//body2.obj", &phoenix_move_left_body[2]);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	loadModel("move_right//body0.obj", &phoenix_move_right_body[0]);
	loadModel("move_right//body1.obj", &phoenix_move_right_body[1]);
	loadModel("move_right//body2.obj", &phoenix_move_right_body[2]);

																	   //поворот крыльев
	loadModel("move_left//wings0.obj", &phoenix_move_left_wings[0]);
	loadModel("move_left//wings1.obj", &phoenix_move_left_wings[1]);
	loadModel("move_left//wings2.obj", &phoenix_move_left_wings[2]);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	loadModel("move_right//wings0.obj", &phoenix_move_right_wings[0]);
	loadModel("move_right//wings1.obj", &phoenix_move_right_wings[1]);
	loadModel("move_right//wings2.obj", &phoenix_move_right_wings[2]);



	glActiveTexture(GL_TEXTURE0);

	//loadModel("obj\\monkey.obj", &monkey);
	/*monkeyTex.loadTextureFromFile("textures//tex.bmp");*/
	phoenixTex_mask.loadTextureFromFile("textures//mask.bmp");         //текстура маски
	phoenixTex_body.loadTextureFromFile("textures//body.bmp");         //текстура тела
	phoenixTex_wings.loadTextureFromFile("textures//wings.bmp");       //текстура крыльев и хвоста
	/*monkeyTex.bindTexture();*/


	tick_n = GetTickCount();
	tick_o = tick_n;

	rec.setSize(300, 100);
	rec.setPosition(10, ogl->getHeight() - 100-10);
	rec.setText("W - движение вперед(зажать)\nA - поворот налево\nD - поворот направо",0,0,0);

	
}




void Render(OpenGL *ogl)
{   
	
	tick_o = tick_n;
	tick_n = GetTickCount();
	Time += (tick_n - tick_o) / 1000.0;

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

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
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//Прогать тут  











	//





	s[0].UseShader();

	//передача параметров в шейдер.  Шаг один - ищем адрес uniform переменной по ее имени. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//Шаг 2 - передаем ей значение
	glUniform3fARB(location, light.pos.X(), light.pos.Y(),light.pos.Z());

	location = glGetUniformLocationARB(s[0].program, "Ia");
	glUniform3fARB(location, 0.2, 0.2, 0.2);

	location = glGetUniformLocationARB(s[0].program, "Id");
	glUniform3fARB(location, 1.0, 1.0, 1.0);

	location = glGetUniformLocationARB(s[0].program, "Is");
	glUniform3fARB(location, .7, .7, .7);


	location = glGetUniformLocationARB(s[0].program, "ma");
	glUniform3fARB(location, 0.2, 0.2, 0.1);

	location = glGetUniformLocationARB(s[0].program, "md");
	glUniform3fARB(location, 0.4, 0.65, 0.5);

	location = glGetUniformLocationARB(s[0].program, "ms");
	glUniform4fARB(location, 0.9, 0.8, 0.3, 25.6);

	location = glGetUniformLocationARB(s[0].program, "camera");
	glUniform3fARB(location, camera.pos.X(), camera.pos.Y(), camera.pos.Z());

	


	Shader::DontUseShaders();

	//phoenix

	s[1].UseShader();
	int l = glGetUniformLocationARB(s[1].program,"tex"); 
	glUniform1iARB(l, 0);    //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	

	static bool f1 = 0;                       //флаги на взмахи крыльев
	static bool f2 = 0;



	glRotated(90, 1, 0, 0);
	

	if (w) {                                                //вперед



	    if (t == 20  || f1) {									 //крылья вверх
	  	    f2 = 0;
		    f1 = 1;
		    t--;
		    if (t == 0) {
			  f2 = 1;
			  f1 = 0;
		    }
	    }
	    else if (t == 0 || f2) {								 //крылья вниз
		    f2 = 1;
		    f1 = 0;
		    t++;
	    }

		  glPushMatrix();								

				


					glTranslatef(dx+hx, 0, dy+hy);                        //двигаемся вперед
					glRotatef(psi, 0, 1, 0);
					/*fi = 0;*/

				phoenixTex_mask.bindTexture();                            ///////////////////////////////рисуем феникса
				phoenix_mask[t].DrawObj();
				phoenixTex_body.bindTexture();
				phoenix_body[t].DrawObj();
				phoenixTex_wings.bindTexture();
				phoenix_wings[t].DrawObj();


		  glPopMatrix();								///////////////////////////////////////////////////////////
	  }
	else if (a) {                                       //влево                   


		x = 5 * cos((-fi*3.14) / 180);
		y = 5 * sin((-fi*3.14) / 180);
		glTranslatef(-x+5+dx+hx, 0, -y+dy+hy);

		hx = -x + 5;
		hy = -y;

		glPushMatrix();

		psi = fi;
		glRotatef(fi, 0, 1, 0);
		fi += 3;

		phoenixTex_mask.bindTexture();							///////////////////////////////рисуем феникса
		phoenix_move_left_mask[q].DrawObj();

		phoenixTex_body.bindTexture();
		phoenix_move_left_body[q].DrawObj();

		phoenixTex_wings.bindTexture();
		phoenix_move_left_wings[q].DrawObj();

		glPopMatrix();

		if (q < 2)
			q++;
	}
	else if (d) {                                       //вправо      


		x = 5 * cos((fi*3.14) / 180);
		y = 5 * sin((fi*3.14) / 180);
		glTranslatef(x-5 + dx, 0, y + dy);
		
		hx = x - 5;
		hy = y;

		  glPushMatrix();

		  psi = -fi;
		  glRotatef(-fi, 0, 1, 0);
		  fi += 3;
		
		phoenixTex_mask.bindTexture();							///////////////////////////////рисуем феникса
		phoenix_move_right_mask[q].DrawObj();

		phoenixTex_body.bindTexture();
		phoenix_move_right_body[q].DrawObj();

		phoenixTex_wings.bindTexture();
		phoenix_move_right_wings[q].DrawObj();

		glPopMatrix();

		if (q < 2)
			q++;


	}

	



	
	Shader::DontUseShaders();

	
	
}   //конец тела функции


bool gui_init = false;

//рисует интерфейс, вызывется после обычного рендера
void RenderGUI(OpenGL *ogl)
{
	
	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	

	glActiveTexture(GL_TEXTURE0);
	rec.Draw();


		
	Shader::DontUseShaders(); 



	
}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 100 - 10);
}

