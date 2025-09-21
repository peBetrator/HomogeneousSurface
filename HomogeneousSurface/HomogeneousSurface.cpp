#include <windows.h> 
#include <GL/gl.h> 
#pragma comment(lib,"OpenGL32.lib") 
#include <GL/glu.h> 
#pragma comment(lib,"Glu32.lib") 
#pragma comment (lib, "legacy_stdio_definitions.lib") 
#include "GL/glaux.h" 
#pragma comment(lib,"Glaux.lib") 
void CALLBACK resize(int width, int height)
{
    // Здесь указывается часть окна в пределах которой 
    // будут рисовать функции OpenGL. 
    GLuint wp = width < height ? width - 20 : height - 20;
    glViewport(10, 10, wp, wp);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Устанавливаем тип проекции 
    // glOrtho - параллельная 
    // glFrustum - перспектива 
    // Параметры о обеих функции идентичны: 
    // они задают объем видимости 
    glOrtho(-6.2, 6.2, -6.2, 6.2, 2.0, 12.0);
    //   glFrustum(-5.0, 5.0, -5.0, 5.0, 2.0, 12.0);  

    glMatrixMode(GL_MODELVIEW);
}

void CALLBACK display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //   glClear(GL_COLOR_BUFFER_BIT); 
    //   glClear(GL_DEPTH_BUFFER_BIT); 

    glPushMatrix();
    glTranslated(0.0, 0.0, -6.0);
    glRotated(35.0, 1.0, 0.0, 0.0);
    glRotated(-35.0, 0.0, 1.0, 0.0);

    glPointSize(10.0f);
    glEnable(GL_POINT_SMOOTH);

    glBegin(GL_POINTS);
    glColor3d(0.0, 0.0, 0.0);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glEnd();

    glDisable(GL_POINT_SMOOTH);

    glLineWidth(1.5f);
    glEnable(GL_LINE_SMOOTH);

    glBegin(GL_LINES);
    // Axa X 
    glColor3d(0.0, 0.0, 0.0);
    glVertex3d(-5.5, 0.0, 0.0);
    glColor3d(1.0, 0.0, 0.0);
    glVertex3d(5.5, 0.0, 0.0);
    // Axa Y 
    glColor3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, -5.5, 0.0);
    glColor3d(0.0, 1.0, 0.0);
    glVertex3d(0.0, 5.5, 0.0);
    // Axa Z 
    glColor3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, -5.5);
    glColor3d(0.0, 0.0, 1.0);
    glVertex3d(0.0, 0.0, 5.5);
    glEnd();

    // Con X 
    glColor3d(1.0, 0.0, 0.0);
    glPushMatrix();
    glTranslated(5.3f, 0.0f, 0.0f);
    glRotated(90.0, 0.0f, 1.0f, 0.0f);
    auxSolidCone(0.1f, 0.2f);
    glPopMatrix();

    // Con Y 
    glColor3d(0.0, 1.0, 0.0);
    glPushMatrix();
    glTranslated(0.0f, 5.3f, 0.0f);
    glRotated(-90.0, 1.0f, 0.0f, 0.0f);
    auxSolidCone(0.1f, 0.2f);
    glPopMatrix();

    // Con Z 
    glColor3d(0.0, 0.0, 1.0);
    glPushMatrix();
    glTranslated(0.0f, 0.0f, 5.3f);
    auxSolidCone(0.1f, 0.2f);
    glPopMatrix();

    //VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV 
    // TODO: add draw code for native data here  
    // to learn OpenGL functions 



      //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 

    glPopMatrix();

    auxSwapBuffers();
}

int main()
{
    // Устанавливаем координаты окна на экране 
    // левый верхний угол (0, 0) 
    // ширина и высота 500  
    auxInitPosition(0, 0, 500, 500);

    // Устанавливаем параметры контекста OpenGL 
    auxInitDisplayMode(AUX_RGB | AUX_DEPTH | AUX_DOUBLE);

    // Создаем окно на экране 
    auxInitWindow(L"OpenGL");

    // Это окно будет получать сообщения о событиях 
    // от клавиатуры, мыши, таймера и любые другие сообщения. 
    // Пока не поступают никакие сообщения циклически будет 
    // вызываться функция display(). 
    // Так можно создавать анимации. 
    // Если нужно статическое изображение 
    // следующая строка может быть закомментирована 
    auxIdleFunc(display);

    // В случае изменения размеров окна – поступает  
    // соответствующее сообщение 
    // В Windows - это WM_SIZE. 
    // Указываем, что функция resize() должна быть вызвана  
    // каждый раз когда изменяются размеры окна 
    auxReshapeFunc(resize);

    // Далее задаем ряд тестов и параметров 
    // Включается тест прозрачности, т.е. будет приниматься 
    // во внимание 4-й параметр в glColor() 
    glEnable(GL_ALPHA_TEST);

    // Тест глубины 
    glEnable(GL_DEPTH_TEST);

    // Функция glColor() будет задавать  
    // свойства материалов. 
    // Следовательно, отсутствует необходимость 
    // в дополнительном вызове функции glMaterialfv() 
    glEnable(GL_COLOR_MATERIAL);

    // Разрешаем освешение 
    glEnable(GL_LIGHTING);

    // Активируем источник освещения с номером 0 
    glEnable(GL_LIGHT0);
    // Задаем позицию источника освещения 
    float pos[4] = { 3.0f, 3.0f, 3.0f, 1.0f };
    float dir[3] = { -1.0f, -1.0f, -1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);

    // Разрешаем смешивание цветов (для прозрачных поверхностей) 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Устанавливаем цвет начальной закраски окна 
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Указываем, что функция display() должна использоваться для 
    // перерисовки окна. 
    // Эта функция будет вызвана каждый раз когда возникает 
    // необходимость в перерисовки окна,   
    // т.е. при поступлении сообщения WM_PAINT от Windows 
    // Например, когда окно развертывается на весь экран. 
    auxMainLoop(display);

    return 0;
}