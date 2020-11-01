// 编 译 选 项：g++ main.cpp -o main -lglut -lGL
#include <GL/glut.h>

void render() {
// 设 置 背 景 色： 矢 车 菊 蓝
glClearColor(0.392, 0.584, 0.930, 1.0);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// 立 即 模 式 中 的 绘 制 以glBegin和glEnd包 裹
// GL_POINTS: 绘 制 点
 // GL_LINES: 绘 制 线 段
// GL_TRIANGLES: 绘 制 三 角 形
glBegin(GL_TRIANGLES);
 glColor3f(1.0, 0.0, 0.0); glVertex2f(0.5, -0.5); // 红
 glColor3f(0.0, 1.0, 0.0); glVertex2f(-0.5, -0.5); // 绿
 glColor3f(0.0, 0.0, 1.0); glVertex2f(0.0, 0.5); // 蓝
 glEnd();
 // 渲 染 图 片
 glFlush();
 }
int main(int argc, char** argv) {
 // 初 始 化GLUT， 它 负 责 创 建OpenGL环 境 以 及 一 个GUI窗 口
 glutInit(&argc, argv);
 glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
 glutInitWindowPosition(60, 60);
 glutInitWindowSize(640, 480);
 glutCreateWindow("PA2 Immediate Mode");
 // 设 置 绘 制 函 数 为render()
 glutDisplayFunc(render);
 // 开 始UI主 循 环
 glutMainLoop();
 return 0;
 }
