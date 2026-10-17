/*
 * Лабораторная работа 11. Просмотр молекул (GTK 3 + cairo + libmat).
 *
 * Программа загружает описание молекулы (каталог molecules) и рисует
 * её в окне: атомы - объёмные шары своего цвета, связи - линии.
 * Молекулу можно вращать, масштабировать, менять размер атомов
 * и перспективу. Расчёт координат - матрицами из библиотеки libmat,
 * рисование - в molecule.c.
 *
 * Управление:
 *   стрелки, a/A, s/S - вращение вокруг осей X и Y (каждое нажатие
 *                       добавляет скорость)
 *   z / Z             - вращение вокруг оси Z
 *   пробел            - остановить вращение
 *   0                 - вернуть исходное положение
 *   + / -             - масштаб
 *   r / R             - уменьшить / увеличить радиус атомов
 *   d / D             - приблизить / отдалить наблюдателя (перспектива)
 *   l                 - подписи атомов вкл/выкл
 *   h                 - атомы водорода вкл/выкл
 *   q, x, Esc         - выход
 *
 * Сборка (нужен пакет libgtk-3-dev):
 *   ./makelib.sh
 *   gcc -Wall -std=c99 C.c molecule.c $(pkg-config --cflags --libs gtk+-3.0) \
 *       -L. -lmat -lm -o C
 * или ./C.sh (собирает библиотеку и программу), или make.
 * Запуск:
 *   LD_LIBRARY_PATH=. ./C molecules/benzimid
 */
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"
#include "molecule.h"

static MOLECULE mol;
static VIEW view;
static double dAlpha = 0, dBeta = 0, dGamma = 0;   /* скорости вращения */
static int view_ready = 0;

/* Перерисовка окна */
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    (void)data;
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    if (!view_ready) {               /* масштаб по размеру окна */
        view_init(&view, &mol, w, h);
        view_ready = 1;
    }
    mol_draw(cr, &mol, &view, w, h);

    /* строка состояния */
    char info[160];
    snprintf(info, sizeof(info),
             "atoms: %d  bonds: %d  scale: %.0f  r: %.0f  d: %.1f%s%s",
             mol.nat, mol.nbond, view.scale, view.rscale, view.dist,
             view.labels ? "  [labels]" : "",
             view.hydrogens ? "" : "  [no H]");
    cairo_set_source_rgb(cr, 0.8, 0.85, 0.9);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, 10, h - 10);
    cairo_show_text(cr, info);
    return FALSE;
}

/* Таймер: поворот на очередной шаг и запрос перерисовки (~33 кадра/с) */
static gboolean on_timer(gpointer data)
{
    GtkWidget *darea = data;
    view.alpha += dAlpha;
    view.beta += dBeta;
    view.gamma += dGamma;
    gtk_widget_queue_draw(darea);
    return TRUE;                     /* таймер продолжает работать */
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event,
                             gpointer data)
{
    (void)widget;
    (void)data;
    switch (event->keyval) {
    case GDK_KEY_plus:
    case GDK_KEY_equal:
    case GDK_KEY_KP_Add:
        view.scale *= 1.1;
        break;
    case GDK_KEY_minus:
    case GDK_KEY_KP_Subtract:
        view.scale *= 0.9;
        break;

    case GDK_KEY_r:                  /* атомы меньше */
        view.rscale *= 1.1;
        break;
    case GDK_KEY_R:                  /* атомы больше */
        view.rscale /= 1.1;
        break;
    case GDK_KEY_d:                  /* наблюдатель ближе - сильнее перспектива */
        view.dist *= 0.9;
        break;
    case GDK_KEY_D:
        view.dist *= 1.1;
        break;

    case GDK_KEY_S:
    case GDK_KEY_Right:
        dBeta -= 0.01;
        break;
    case GDK_KEY_s:
    case GDK_KEY_Left:
        dBeta += 0.01;
        break;
    case GDK_KEY_a:
    case GDK_KEY_Down:
        dAlpha -= 0.01;
        break;
    case GDK_KEY_A:
    case GDK_KEY_Up:
        dAlpha += 0.01;
        break;
    case GDK_KEY_z:
        dGamma += 0.01;
        break;
    case GDK_KEY_Z:
        dGamma -= 0.01;
        break;
    case GDK_KEY_space:              /* остановить вращение */
        dAlpha = dBeta = dGamma = 0;
        break;
    case GDK_KEY_0:                  /* исходное положение */
        view.alpha = view.beta = view.gamma = 0;
        dAlpha = dBeta = dGamma = 0;
        break;

    case GDK_KEY_l:
    case GDK_KEY_L:
        view.labels = !view.labels;
        break;
    case GDK_KEY_h:
    case GDK_KEY_H:
        view.hydrogens = !view.hydrogens;
        break;

    case GDK_KEY_x:
    case GDK_KEY_X:
    case GDK_KEY_q:
    case GDK_KEY_Q:
    case GDK_KEY_Escape:
        gtk_main_quit();
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    if (argc < 2) {
        printf("Использование: %s файл_молекулы\n", argv[0]);
        printf("Например:      %s molecules/benzimid\n", argv[0]);
        return 1;
    }
    if (mol_load(argv[1], &mol) != 0)
        return 1;
    printf("%s: атомов %d, связей %d\n", argv[1], mol.nat, mol.nbond);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), darea);

    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw), NULL);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(window), "key_press_event",
                     G_CALLBACK(on_key_press), NULL);
    g_timeout_add(30, on_timer, darea);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);
    gtk_window_set_title(GTK_WINDOW(window), "MOLECULE");
    gtk_widget_show_all(window);

    gtk_main();

    mol_free(&mol);
    return 0;
}
