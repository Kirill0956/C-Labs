/*
 * Лабораторная работа 10. Тессеракт (четырёхмерный куб).
 *
 * Программа использует библиотеку для работы с матрицами libmat
 * (собирается скриптом makelib.sh) и рисует вращающийся тессеракт в окне X11.
 *
 * Как это работает:
 *  1) 16 вершин тессеракта - все точки (±1, ±1, ±1, ±1); 32 ребра
 *     соединяют вершины, отличающиеся ровно одной координатой.
 *  2) Вершина записывается в однородных координатах как строка
 *     (x, y, z, w, 1) и умножается на произведение матриц 5x5:
 *     повороты в плоскостях XY, XZ, XW, YZ, YW, ZW (mat_fill_R).
 *  3) Перспективная проекция 4D -> 3D: матрица 5x5 с элементом
 *     P[3][4] = -1/D4, после умножения координаты делятся на
 *     последнюю компоненту (как при обычной проекции 3D -> 2D).
 *  4) Поворот полученного 3D-объекта для наглядности (mat_fill_x,
 *     mat_fill_y, матрицы 4x4) и проекция 3D -> 2D (mat_fill_p).
 *  5) Рёбра рисуются через Xlib во вспомогательный Pixmap, который
 *     затем копируется в окно (двойная буферизация, без мерцания).
 *
 * Управление:
 *   1..6  - включить/выключить вращение в плоскости
 *           XY, XZ, XW, YZ, YW, ZW
 *   стрелки - повернуть "камеру" (3D-вращение)
 *   + / - - масштаб, пробел - пауза, r - исходное положение, q - выход
 *
 * Сборка и запуск:
 *   ./makelib.sh                      (собирает libmat.so и libmat.a)
 *   gcc -Wall -std=c99 tesseract.c -L. -lmat -lX11 -lm -o tesseract
 *   LD_LIBRARY_PATH=. ./tesseract
 * или просто: make && make run
 *
 * Параметр --shot файл.ppm - нарисовать один кадр, сохранить картинку
 * и выйти (удобно для проверки без интерактивной работы).
 */
#define _DEFAULT_SOURCE
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "matrix.h"

#define DIM     4               /* размерность пространства */
#define NV      16              /* вершин */
#define NE      32              /* рёбер */
#define D4      5.0             /* расстояние до "камеры" в 4D */
#define D3      12.0            /* расстояние до камеры в 3D */

double V[NV][DIM];              /* вершины */
int E[NE][2];                   /* рёбра */
double SX[NV], SY[NV];          /* экранные координаты */
double W4[NV];                  /* исходная координата W (для цвета) */

/* углы поворота в 6 плоскостях и признак вращения */
const int planes[6][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}};
const char *plane_names[6] = {"XY", "XZ", "XW", "YZ", "YW", "ZW"};
double angle[6] = {0, 0, 0, 0, 0, 0};
int spin[6] = {0, 0, 1, 0, 1, 0};           /* по умолчанию XW и YW */
double camx = 0.45, camy = -0.6;            /* поворот камеры */
double scale = 140.0;
int paused = 0;

Display *dis;
int screen;
Window win;
Pixmap buf;
GC gc;
int width = 800, height = 700;
unsigned long col_bg, col_in, col_out, col_link, col_text;

/* Построить вершины и рёбра */
void build_tesseract(void)
{
    for (int i = 0; i < NV; i++)
        for (int d = 0; d < DIM; d++)
            V[i][d] = (i >> d) & 1 ? 1.0 : -1.0;
    int k = 0;
    for (int i = 0; i < NV; i++)
        for (int d = 0; d < DIM; d++) {
            int j = i ^ (1 << d);           /* отличается одной координатой */
            if (i < j) {
                E[k][0] = i;
                E[k][1] = j;
                k++;
            }
        }
}

/* Перемножить A = A * B, старую A освободить */
MATRIX mul_into(MATRIX A, MATRIX B)
{
    MATRIX C = mat_mul(A, B);
    mat_free(A);
    return C;
}

/* Вычислить экранные координаты всех вершин */
void project(void)
{
    /* --- 4D: повороты и перспектива (матрицы 5x5) --- */
    MATRIX M4 = mat_creat(DIM + 1, DIM + 1, UNIT_MATRIX);
    MATRIX R = _mat_creat(DIM + 1, DIM + 1);
    for (int p = 0; p < 6; p++) {
        mat_fill_R(R, planes[p][0], planes[p][1], angle[p]);
        M4 = mul_into(M4, R);
    }
    MATRIX P4 = mat_creat(DIM + 1, DIM + 1, UNIT_MATRIX);
    P4[3][4] = -1.0 / D4;                    /* w' = 1 - w/D4 */
    M4 = mul_into(M4, P4);

    /* --- 3D: поворот камеры и перспектива (матрицы 4x4) --- */
    MATRIX M3 = mat_creat(4, 4, UNIT_MATRIX);
    MATRIX T = _mat_creat(4, 4);
    M3 = mul_into(M3, mat_fill_y(T, camy));
    M3 = mul_into(M3, mat_fill_x(T, camx));
    M3 = mul_into(M3, mat_fill_p(T, 0, 0, D3));

    MATRIX v4 = _mat_creat(1, DIM + 1);
    MATRIX v3 = _mat_creat(1, 4);
    for (int i = 0; i < NV; i++) {
        for (int d = 0; d < DIM; d++)
            v4[0][d] = V[i][d];
        v4[0][DIM] = 1.0;
        MATRIX r4 = mat_mul(v4, M4);
        double h = r4[0][DIM];               /* однородная координата */
        for (int d = 0; d < 3; d++)
            v3[0][d] = r4[0][d] / h;
        v3[0][3] = 1.0;
        W4[i] = V[i][3];

        MATRIX r3 = mat_mul(v3, M3);
        SX[i] = width / 2.0 + scale * r3[0][0] / r3[0][3];
        SY[i] = height / 2.0 - scale * r3[0][1] / r3[0][3];
        mat_free(r4);
        mat_free(r3);
    }

    mat_free(v4);
    mat_free(v3);
    mat_free(T);
    mat_free(M3);
    mat_free(P4);
    mat_free(R);
    mat_free(M4);
}

unsigned long get_color(const char *name)
{
    XColor c;
    Colormap cmap = DefaultColormap(dis, screen);
    if (!XParseColor(dis, cmap, name, &c) || !XAllocColor(dis, cmap, &c))
        return WhitePixel(dis, screen);
    return c.pixel;
}

void draw(void)
{
    project();

    XSetForeground(dis, gc, col_bg);
    XFillRectangle(dis, buf, gc, 0, 0, (unsigned)width, (unsigned)height);

    for (int k = 0; k < NE; k++) {
        int a = E[k][0], b = E[k][1];
        /* "внутренний" куб (W=-1), "внешний" (W=+1) и связи между ними */
        if (W4[a] < 0 && W4[b] < 0)
            XSetForeground(dis, gc, col_in);
        else if (W4[a] > 0 && W4[b] > 0)
            XSetForeground(dis, gc, col_out);
        else
            XSetForeground(dis, gc, col_link);
        XDrawLine(dis, buf, gc, (int)SX[a], (int)SY[a], (int)SX[b], (int)SY[b]);
    }
    XSetForeground(dis, gc, col_text);
    for (int i = 0; i < NV; i++)
        XFillArc(dis, buf, gc, (int)SX[i] - 4, (int)SY[i] - 4, 8, 8, 0, 360 * 64);

    char info[128] = "Rotation:";
    for (int p = 0; p < 6; p++)
        if (spin[p]) {
            strcat(info, " ");
            strcat(info, plane_names[p]);
        }
    strcat(info, paused ? "  [pause]" : "");
    XDrawString(dis, buf, gc, 10, 20, info, (int)strlen(info));
    const char *help = "1-6 planes XY XZ XW YZ YW ZW, arrows camera, +/- zoom, "
                       "space pause, r reset, q quit";
    XDrawString(dis, buf, gc, 10, height - 10, help, (int)strlen(help));

    XCopyArea(dis, buf, win, gc, 0, 0, (unsigned)width, (unsigned)height, 0, 0);
    XFlush(dis);
}

/* Сохранить содержимое буфера в файл PPM */
void save_ppm(const char *fname)
{
    XImage *img = XGetImage(dis, buf, 0, 0, (unsigned)width, (unsigned)height,
                            AllPlanes, ZPixmap);
    FILE *f = fopen(fname, "wb");
    if (img == NULL || f == NULL) {
        fprintf(stderr, "Не удалось сохранить %s\n", fname);
        return;
    }
    /* у Pixmap нет своего Visual - маски цветов берём у экрана */
    Visual *vis = DefaultVisual(dis, screen);
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            unsigned long p = XGetPixel(img, x, y);
            unsigned char rgb[3] = {
                (unsigned char)((p & vis->red_mask) >> 16),
                (unsigned char)((p & vis->green_mask) >> 8),
                (unsigned char)(p & vis->blue_mask)};
            fwrite(rgb, 1, 3, f);
        }
    fclose(f);
    XDestroyImage(img);
}

void make_buffer(void)
{
    buf = XCreatePixmap(dis, win, (unsigned)width, (unsigned)height,
                        (unsigned)DefaultDepth(dis, screen));
}

int main(int argc, char *argv[])
{
    const char *shot = NULL;
    if (argc > 2 && strcmp(argv[1], "--shot") == 0) {
        shot = argv[2];
        for (int p = 0; p < 6; p++)
            angle[p] = 0.35 * (p + 1);
    }

    build_tesseract();

    dis = XOpenDisplay(NULL);
    if (dis == NULL) {
        fprintf(stderr, "Не удалось подключиться к X-серверу\n");
        return 1;
    }
    screen = DefaultScreen(dis);
    win = XCreateSimpleWindow(dis, RootWindow(dis, screen), 0, 0,
                              (unsigned)width, (unsigned)height, 1,
                              BlackPixel(dis, screen), BlackPixel(dis, screen));
    XStoreName(dis, win, "Lab 10: Tesseract");
    XSelectInput(dis, win, ExposureMask | KeyPressMask | StructureNotifyMask);
    Atom wm_delete = XInternAtom(dis, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dis, win, &wm_delete, 1);
    gc = XCreateGC(dis, win, 0, NULL);
    XSetLineAttributes(dis, gc, 2, LineSolid, CapRound, JoinRound);

    col_bg = get_color("#101820");
    col_in = get_color("#4FC3F7");
    col_out = get_color("#FFB74D");
    col_link = get_color("#9575CD");
    col_text = get_color("#ECEFF1");

    make_buffer();

    if (shot) {
        draw();
        save_ppm(shot);
        printf("Кадр сохранён в %s\n", shot);
        XFreePixmap(dis, buf);
        XFreeGC(dis, gc);
        XCloseDisplay(dis);
        return 0;
    }

    XMapWindow(dis, win);

    for (;;) {
        while (XPending(dis)) {
            XEvent ev;
            XNextEvent(dis, &ev);
            if (ev.type == ConfigureNotify &&
                (ev.xconfigure.width != width || ev.xconfigure.height != height)) {
                width = ev.xconfigure.width;
                height = ev.xconfigure.height;
                XFreePixmap(dis, buf);
                make_buffer();
            } else if (ev.type == ClientMessage &&
                       (Atom)ev.xclient.data.l[0] == wm_delete) {
                goto quit;
            } else if (ev.type == KeyPress) {
                KeySym key = XLookupKeysym(&ev.xkey, 0);
                if (key >= XK_1 && key <= XK_6)
                    spin[key - XK_1] = !spin[key - XK_1];
                else if (key == XK_Left)  camy -= 0.1;
                else if (key == XK_Right) camy += 0.1;
                else if (key == XK_Up)    camx -= 0.1;
                else if (key == XK_Down)  camx += 0.1;
                else if (key == XK_plus || key == XK_equal || key == XK_KP_Add)
                    scale *= 1.1;
                else if (key == XK_minus || key == XK_KP_Subtract)
                    scale /= 1.1;
                else if (key == XK_space)
                    paused = !paused;
                else if (key == XK_r)
                    memset(angle, 0, sizeof(angle));
                else if (key == XK_q || key == XK_Escape)
                    goto quit;
            }
        }
        if (!paused)
            for (int p = 0; p < 6; p++)
                if (spin[p])
                    angle[p] += 0.01;
        draw();
        usleep(16000);                       /* ~60 кадров в секунду */
    }

quit:
    XFreePixmap(dis, buf);
    XFreeGC(dis, gc);
    XDestroyWindow(dis, win);
    XCloseDisplay(dis);
    return 0;
}
