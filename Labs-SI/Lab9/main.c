/*
 * Лабораторная работа 9. Графика X11 (Xlib).
 *
 * Программа-"рисовалка" на основе примера X.c:
 *   левая кнопка мыши  - добавить вершину ломаной (первая вершина
 *                        начинает новую фигуру, цвет берётся из палитры);
 *   правая кнопка мыши - замкнуть фигуру (соединить последнюю вершину
 *                        с первой);
 *   f - включить/выключить заливку замкнутых фигур;
 *   u - отменить последнюю фигуру;
 *   c - очистить окно;
 *   q - выход (также можно закрыть окно крестиком).
 *
 * В отличие от исходного примера, все фигуры хранятся в памяти, поэтому
 * при перекрытии окна, сворачивании и изменении размера (событие Expose)
 * рисунок полностью восстанавливается.
 *
 * Сборка:  gcc -Wall -Wextra -std=c99 main.c -o draw -lX11
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

#define NCOLORS 7

typedef struct {
    XPoint *pts;                /* вершины */
    int n, cap;
    int closed;                 /* фигура замкнута */
    unsigned long color;
} FIGURE;

/* Переменные X11 */
Display *dis;
int screen;
Window win;
GC gc;
Atom wm_delete;

unsigned long palette[NCOLORS];
int color_index = 0;

FIGURE *figs = NULL;            /* все фигуры */
int nfigs = 0, figs_cap = 0;
int drawing = 0;                /* текущая фигура ещё не замкнута */
int fill = 0;                   /* режим заливки */

void init_x(void);
void close_x(void);
void redraw(void);
void draw_figure(const FIGURE *f);

void *xrealloc(void *p, size_t size)
{
    p = realloc(p, size);
    if (p == NULL) {
        fprintf(stderr, "Не хватает памяти\n");
        exit(1);
    }
    return p;
}

/* Начать новую фигуру */
FIGURE *new_figure(void)
{
    if (nfigs == figs_cap) {
        figs_cap = figs_cap ? figs_cap * 2 : 16;
        figs = xrealloc(figs, sizeof(FIGURE) * (size_t)figs_cap);
    }
    FIGURE *f = &figs[nfigs++];
    f->pts = NULL;
    f->n = f->cap = 0;
    f->closed = 0;
    f->color = palette[color_index];
    color_index = (color_index + 1) % NCOLORS;
    return f;
}

void add_point(FIGURE *f, int x, int y)
{
    if (f->n == f->cap) {
        f->cap = f->cap ? f->cap * 2 : 16;
        f->pts = xrealloc(f->pts, sizeof(XPoint) * (size_t)f->cap);
    }
    f->pts[f->n].x = (short)x;
    f->pts[f->n].y = (short)y;
    f->n++;
}

void free_figures(void)
{
    for (int i = 0; i < nfigs; i++)
        free(figs[i].pts);
    nfigs = 0;
    drawing = 0;
}

int main(void)
{
    XEvent event;
    KeySym key;
    char text[16];

    init_x();

    for (;;) {
        XNextEvent(dis, &event);

        switch (event.type) {
        case Expose:
            if (event.xexpose.count == 0)
                redraw();
            break;

        case ClientMessage:                     /* закрытие окна */
            if ((Atom)event.xclient.data.l[0] == wm_delete)
                close_x();
            break;

        case KeyPress:
            if (XLookupString(&event.xkey, text, sizeof(text), &key, 0) != 1)
                break;
            switch (text[0]) {
            case 'q': case 'Q':
                close_x();
                break;
            case 'c': case 'C':
                free_figures();
                redraw();
                break;
            case 'u': case 'U':
                if (nfigs > 0) {
                    free(figs[--nfigs].pts);
                    drawing = 0;
                    redraw();
                }
                break;
            case 'f': case 'F':
                fill = !fill;
                redraw();
                break;
            }
            printf("Нажата клавиша %c\n", text[0]);
            break;

        case ButtonPress: {
            int x = event.xbutton.x, y = event.xbutton.y;

            if (event.xbutton.button == Button1) {
                if (!drawing) {                 /* новая фигура */
                    FIGURE *f = new_figure();
                    add_point(f, x, y);
                    drawing = 1;
                    XSetForeground(dis, gc, f->color);
                    XDrawPoint(dis, win, gc, x, y);
                } else {                        /* следующая вершина */
                    FIGURE *f = &figs[nfigs - 1];
                    XPoint last = f->pts[f->n - 1];
                    add_point(f, x, y);
                    XSetForeground(dis, gc, f->color);
                    XDrawLine(dis, win, gc, last.x, last.y, x, y);
                }
            } else if (event.xbutton.button == Button3 && drawing) {
                FIGURE *f = &figs[nfigs - 1];   /* замкнуть фигуру */
                f->closed = 1;
                drawing = 0;
                draw_figure(f);
            }
            break;
        }
        }
    }
}

void draw_figure(const FIGURE *f)
{
    XSetForeground(dis, gc, f->color);
    if (f->n == 1) {
        XDrawPoint(dis, win, gc, f->pts[0].x, f->pts[0].y);
        return;
    }
    if (f->closed && fill && f->n >= 3)
        XFillPolygon(dis, win, gc, f->pts, f->n, Complex, CoordModeOrigin);
    XDrawLines(dis, win, gc, f->pts, f->n, CoordModeOrigin);
    if (f->closed)
        XDrawLine(dis, win, gc, f->pts[f->n - 1].x, f->pts[f->n - 1].y,
                  f->pts[0].x, f->pts[0].y);
}

/* Перерисовка всего окна из сохранённых фигур */
void redraw(void)
{
    XClearWindow(dis, win);
    for (int i = 0; i < nfigs; i++)
        draw_figure(&figs[i]);
    XFlush(dis);
}

unsigned long get_color(const char *name)
{
    XColor c;
    Colormap cmap = DefaultColormap(dis, screen);
    if (!XParseColor(dis, cmap, name, &c) || !XAllocColor(dis, cmap, &c))
        return BlackPixel(dis, screen);
    return c.pixel;
}

void init_x(void)
{
    dis = XOpenDisplay(NULL);
    if (dis == NULL) {
        fprintf(stderr, "Не удалось подключиться к X-серверу\n");
        exit(1);
    }
    screen = DefaultScreen(dis);
    unsigned long black = BlackPixel(dis, screen);
    unsigned long white = WhitePixel(dis, screen);

    win = XCreateSimpleWindow(dis, DefaultRootWindow(dis), 0, 0,
                              800, 500, 5, black, white);
    XSetStandardProperties(dis, win, "Lab 9: X11", "Lab9", None, NULL, 0, NULL);
    XSelectInput(dis, win, ExposureMask | ButtonPressMask | KeyPressMask);

    /* чтобы получать событие при закрытии окна крестиком */
    wm_delete = XInternAtom(dis, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dis, win, &wm_delete, 1);

    gc = XCreateGC(dis, win, 0, NULL);
    XSetBackground(dis, gc, white);
    XSetForeground(dis, gc, black);
    XSetLineAttributes(dis, gc, 2, LineSolid, CapRound, JoinRound);

    const char *names[NCOLORS] = {"indian red", "orange", "dark green",
                                  "gold", "blue", "dark blue", "violet"};
    for (int i = 0; i < NCOLORS; i++)
        palette[i] = get_color(names[i]);

    XClearWindow(dis, win);
    XMapRaised(dis, win);

    printf("ЛКМ - вершина, ПКМ - замкнуть, f - заливка, u - отмена, "
           "c - очистить, q - выход\n");
}

void close_x(void)
{
    free_figures();
    free(figs);
    XFreeGC(dis, gc);
    XDestroyWindow(dis, win);
    XCloseDisplay(dis);
    exit(0);
}
