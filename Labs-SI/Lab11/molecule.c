/*
 * molecule.c - загрузка и отрисовка молекулы.
 *
 * Преобразование координат выполняется матрицами 4x4 из libmat:
 *   поворот вокруг X (mat_fill_x), Y (mat_fill_y), Z (mat_fill_z),
 *   центральная проекция (mat_fill_p).
 * Точка (x, y, z, 1) умножается на итоговую матрицу, после чего x и y
 * делятся на однородную координату w.
 *
 * Атомы и связи рисуются в порядке удаления от наблюдателя
 * (алгоритм художника): сначала дальние, потом ближние, поэтому
 * ближние атомы закрывают дальние.
 */
#define _DEFAULT_SOURCE              /* для M_PI */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "matrix.h"
#include "molecule.h"

/* Цвет, радиус и масса атома по его имени */
static void atom_type(ATOM *a, double *mass)
{
    int c = tolower((unsigned char)a->name[0]);
    int c2 = tolower((unsigned char)a->name[1]);

    a->hydrogen = 0;
    if (c == 'c' && c2 == 'l') {            /* хлор */
        *mass = 35.5; a->R = 18;
        a->red = 31 / 255.; a->green = 240 / 255.; a->blue = 31 / 255.;
        return;
    }
    if (c == 'b' && c2 == 'r') {            /* бром */
        *mass = 80; a->R = 22;
        a->red = 166 / 255.; a->green = 41 / 255.; a->blue = 41 / 255.;
        return;
    }
    switch (c) {
    case 'h':
    case '~':                               /* водород */
        *mass = 1; a->R = 5; a->hydrogen = 1;
        a->red = 0.92; a->green = 0.92; a->blue = 0.92;
        break;
    case 'c':                               /* углерод */
        *mass = 12; a->R = 12;
        a->red = 0.45; a->green = 0.45; a->blue = 0.45;
        break;
    case 'o':                               /* кислород */
        *mass = 16; a->R = 13;
        a->red = 0.94; a->green = 0.1; a->blue = 0.1;
        break;
    case 'n':                               /* азот */
        *mass = 14; a->R = 12;
        a->red = 0.2; a->green = 0.35; a->blue = 0.95;
        break;
    case 's':                               /* сера */
        *mass = 32; a->R = 18;
        a->red = 1.0; a->green = 0.78; a->blue = 0.2;
        break;
    case 'p':                               /* фосфор */
        *mass = 31; a->R = 18;
        a->red = 1.0; a->green = 0.5; a->blue = 0.0;
        break;
    case 'f':                               /* фтор, железо */
        *mass = 19; a->R = 11;
        a->red = 0.56; a->green = 0.88; a->blue = 0.31;
        if (c2 == 'e') {
            *mass = 56; a->R = 20;
            a->red = 0.88; a->green = 0.4; a->blue = 0.2;
        }
        break;
    case 'z':                               /* цинк */
        *mass = 65.4; a->R = 20;
        a->red = 0.49; a->green = 0.5; a->blue = 0.69;
        break;
    default:
        *mass = 30; a->R = 14;
        a->red = 0.06; a->green = 0.57; a->blue = 0.26;
    }
}

int mol_load(const char *fname, MOLECULE *m)
{
    memset(m, 0, sizeof(*m));
    FILE *f = fopen(fname, "r");
    if (f == NULL) {
        perror(fname);
        return -1;
    }
    if (fscanf(f, "%d%d", &m->nat, &m->nbond) != 2 || m->nat <= 0 ||
        m->nbond < 0) {
        fprintf(stderr, "%s: неверный заголовок файла\n", fname);
        fclose(f);
        return -1;
    }
    m->at = calloc((size_t)m->nat, sizeof(ATOM));
    m->bond = calloc((size_t)(m->nbond ? m->nbond : 1), sizeof(m->bond[0]));
    if (m->at == NULL || m->bond == NULL) {
        fprintf(stderr, "Не хватает памяти\n");
        fclose(f);
        return -1;
    }

    double xc = 0, yc = 0, zc = 0, M = 0, mass, q;
    int num;
    char line[256];
    if (fgets(line, sizeof(line), f) == NULL)        /* остаток строки */
        line[0] = '\0';
    for (int i = 0; i < m->nat; i++) {
        ATOM *a = &m->at[i];
        if (fgets(line, sizeof(line), f) == NULL ||
            sscanf(line, "%d%15s%lf%lf%lf%lf", &num, a->name,
                   &a->xyz[0], &a->xyz[1], &a->xyz[2], &q) < 5) {
            fprintf(stderr, "%s: ошибка в описании атома %d\n", fname, i + 1);
            fclose(f);
            return -1;
        }
        atom_type(a, &mass);
        xc += a->xyz[0] * mass;
        yc += a->xyz[1] * mass;
        zc += a->xyz[2] * mass;
        M += mass;
    }
    /* перенос начала координат в центр масс - вращение вокруг него */
    xc /= M;
    yc /= M;
    zc /= M;
    for (int i = 0; i < m->nat; i++) {
        m->at[i].xyz[0] -= xc;
        m->at[i].xyz[1] -= yc;
        m->at[i].xyz[2] -= zc;
    }

    int nb = 0, from, to;
    for (int i = 0; i < m->nbond; i++) {
        if (fgets(line, sizeof(line), f) == NULL)
            break;
        if (sscanf(line, "%d%d%d", &num, &from, &to) != 3)
            continue;
        if (from < 1 || from > m->nat || to < 1 || to > m->nat)
            continue;                               /* неверная связь */
        m->bond[nb][0] = from - 1;
        m->bond[nb][1] = to - 1;
        nb++;
    }
    m->nbond = nb;
    fclose(f);
    return 0;
}

void mol_free(MOLECULE *m)
{
    free(m->at);
    free(m->bond);
    m->at = NULL;
    m->bond = NULL;
    m->nat = m->nbond = 0;
}

void view_init(VIEW *v, const MOLECULE *m, int width, int height)
{
    double rmax = 0.5;
    for (int i = 0; i < m->nat; i++) {
        double r = sqrt(m->at[i].xyz[0] * m->at[i].xyz[0] +
                        m->at[i].xyz[1] * m->at[i].xyz[1] +
                        m->at[i].xyz[2] * m->at[i].xyz[2]);
        if (r > rmax)
            rmax = r;
    }
    /* средняя длина связи: радиусы атомов подбираются под неё,
       чтобы и молекулы (связь ~1.4 А), и "кубик" (ребро 1) выглядели
       одинаково */
    double L = 0;
    for (int i = 0; i < m->nbond; i++) {
        const double *a = m->at[m->bond[i][0]].xyz, *b = m->at[m->bond[i][1]].xyz;
        L += sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) +
                  (a[2] - b[2]) * (a[2] - b[2]));
    }
    L = m->nbond > 0 && L > 0 ? L / m->nbond : 1.45;

    int side = width < height ? width : height;
    v->alpha = v->beta = v->gamma = 0;
    v->scale = 0.35 * side / rmax;
    v->rscale = 40.0 * 1.45 / L;
    v->bondw = 0.08 * L;
    v->dist = 6.0 * rmax;
    v->labels = 0;
    v->hydrogens = 1;
}

/* ------------------ отрисовка ------------------ */

typedef struct {
    double depth;               /* чем больше, тем ближе к наблюдателю */
    int kind;                   /* 0 - связь, 1 - атом */
    int index;
} ITEM;

static int cmp_depth(const void *a, const void *b)
{
    const ITEM *p = a, *q = b;
    if (p->depth < q->depth) return -1;
    if (p->depth > q->depth) return 1;
    return p->kind - q->kind;   /* при равной глубине связь раньше атома */
}

void mol_draw(cairo_t *cr, const MOLECULE *m, const VIEW *v,
              int width, int height)
{
    /* фон */
    cairo_set_source_rgb(cr, 0.07, 0.09, 0.12);
    cairo_paint(cr);
    if (m->nat == 0)
        return;

    /* итоговая матрица преобразования: Rx * Ry * Rz * P */
    MATRIX R = _mat_creat(4, 4);
    MATRIX A = mat_creat(4, 4, UNIT_MATRIX), B;
    B = mat_mul(A, mat_fill_x(R, v->alpha)); mat_free(A);
    A = mat_mul(B, mat_fill_y(R, v->beta));  mat_free(B);
    B = mat_mul(A, mat_fill_z(R, v->gamma)); mat_free(A);
    A = mat_mul(B, mat_fill_p(R, 0, 0, v->dist)); mat_free(B);
    MATRIX P = A;

    double *X = malloc(sizeof(double) * (size_t)m->nat);
    double *Y = malloc(sizeof(double) * (size_t)m->nat);
    double *Z = malloc(sizeof(double) * (size_t)m->nat);
    double *W = malloc(sizeof(double) * (size_t)m->nat);
    ITEM *items = malloc(sizeof(ITEM) * (size_t)(m->nat + m->nbond));
    MATRIX vec = _mat_creat(1, 4);
    if (!X || !Y || !Z || !W || !items || !vec) {
        fprintf(stderr, "Не хватает памяти\n");
        exit(1);
    }

    double xc = width / 2.0, yc = height / 2.0;
    for (int i = 0; i < m->nat; i++) {
        vec[0][0] = m->at[i].xyz[0];
        vec[0][1] = m->at[i].xyz[1];
        vec[0][2] = m->at[i].xyz[2];
        vec[0][3] = 1.0;
        MATRIX r = mat_mul(vec, P);
        W[i] = r[0][3] > 0.05 ? r[0][3] : 0.05;   /* за спиной наблюдателя */
        X[i] = xc + v->scale * r[0][0] / W[i];
        Y[i] = yc - v->scale * r[0][1] / W[i];
        Z[i] = r[0][2];
        mat_free(r);
    }

    /* список объектов для рисования, отсортированный по глубине */
    int n = 0;
    for (int i = 0; i < m->nbond; i++) {
        int a = m->bond[i][0], b = m->bond[i][1];
        if (!v->hydrogens && (m->at[a].hydrogen || m->at[b].hydrogen))
            continue;
        items[n].depth = (Z[a] + Z[b]) / 2;
        items[n].kind = 0;
        items[n].index = i;
        n++;
    }
    for (int i = 0; i < m->nat; i++) {
        if (!v->hydrogens && m->at[i].hydrogen)
            continue;
        items[n].depth = Z[i];
        items[n].kind = 1;
        items[n].index = i;
        n++;
    }
    qsort(items, (size_t)n, sizeof(ITEM), cmp_depth);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);

    for (int k = 0; k < n; k++) {
        if (items[k].kind == 0) {                    /* связь */
            int a = m->bond[items[k].index][0], b = m->bond[items[k].index][1];
            double xm = (X[a] + X[b]) / 2, ym = (Y[a] + Y[b]) / 2;
            double lw = v->bondw * v->scale / ((W[a] + W[b]) / 2);
            if (lw < 1.5) lw = 1.5;
            cairo_set_line_width(cr, lw);
            /* каждая половина связи - цветом своего атома */
            const ATOM *pa = &m->at[a], *pb = &m->at[b];
            cairo_set_source_rgb(cr, pa->red * 0.8, pa->green * 0.8, pa->blue * 0.8);
            cairo_move_to(cr, X[a], Y[a]);
            cairo_line_to(cr, xm, ym);
            cairo_stroke(cr);
            cairo_set_source_rgb(cr, pb->red * 0.8, pb->green * 0.8, pb->blue * 0.8);
            cairo_move_to(cr, xm, ym);
            cairo_line_to(cr, X[b], Y[b]);
            cairo_stroke(cr);
        } else {                                      /* атом - шар */
            int i = items[k].index;
            const ATOM *a = &m->at[i];
            double r = a->R / v->rscale * v->scale / W[i];
            if (r < 1.5) r = 1.5;
            /* радиальный градиент - эффект объёма */
            cairo_pattern_t *g = cairo_pattern_create_radial(
                X[i] - r / 3, Y[i] - r / 3, r / 8, X[i], Y[i], r);
            cairo_pattern_add_color_stop_rgb(g, 0,
                0.5 + a->red / 2, 0.5 + a->green / 2, 0.5 + a->blue / 2);
            cairo_pattern_add_color_stop_rgb(g, 1,
                a->red * 0.45, a->green * 0.45, a->blue * 0.45);
            cairo_arc(cr, X[i], Y[i], r, 0, 2 * M_PI);
            cairo_set_source(cr, g);
            cairo_fill(cr);
            cairo_pattern_destroy(g);

            if (v->labels) {
                cairo_set_source_rgb(cr, 1, 1, 1);
                cairo_move_to(cr, X[i] + r * 0.7, Y[i] - r * 0.7);
                cairo_show_text(cr, a->name);
            }
        }
    }

    free(X); free(Y); free(Z); free(W); free(items);
    mat_free(vec);
    mat_free(R);
    mat_free(P);
}
