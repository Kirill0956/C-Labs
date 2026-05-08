#ifndef HUMAN_H
#define HUMAN_H

struct Human {
    char name[50];
    char surname[50];
    int birthYear;
};

void sortByBirthYear(struct Human arr[], int size);
void printHumans(struct Human arr[], int size, const char *title);

#endif