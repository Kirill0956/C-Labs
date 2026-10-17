/*
 * molecule.h - загрузка молекулы из файла и её отрисовка средствами
 * cairo с использованием библиотеки матриц libmat.
 */
#ifndef MOLECULE_H
#define MOLECULE_H

#include <cairo.h>

typedef struct atom {
    double xyz[3];              /* координаты (центр масс в начале координат) */
    char name[16];              /* имя атома из файла: C2, NPL3, ~ ... */
    double R;                   /* условный радиус */
    double red, green, blue;    /* цвет */
    int hydrogen;               /* 1 - атом водорода */
} ATOM;

typedef struct {
    ATOM *at;
    int nat;                    /* число атомов */
    int (*bond)[2];             /* связи: номера атомов (с нуля) */
    int nbond;                  /* число связей */
} MOLECULE;

/* Параметры отображения */
typedef struct {
    double alpha, beta, gamma;  /* углы поворота вокруг X, Y, Z */
    double scale;               /* масштаб: пикселей на ангстрем */
    double rscale;              /* делитель радиуса атомов (r/R) */
    double dist;                /* расстояние до наблюдателя (d/D) */
    double bondw;               /* толщина связи в ангстремах */
    int labels;                 /* показывать подписи атомов */
    int hydrogens;              /* показывать атомы водорода */
} VIEW;

/* Загрузить молекулу. Формат файла:
     Nat Nl
     номер имя x y z заряд      (Nat строк)
     номер от до тип            (Nl строк)
   Возвращает 0 при успехе. */
int mol_load(const char *fname, MOLECULE *m);
void mol_free(MOLECULE *m);

/* Начальные параметры отображения, подобранные под размер молекулы */
void view_init(VIEW *v, const MOLECULE *m, int width, int height);

/* Нарисовать молекулу в контексте cairo размером width x height */
void mol_draw(cairo_t *cr, const MOLECULE *m, const VIEW *v,
              int width, int height);

#endif
