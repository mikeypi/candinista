#ifndef YAMLPRINTER_H
#define YAMLPRINTER_H
#include <stdio.h>
#include "yaml-loader.h"

void configuration_print (FILE* fp, const Configuration* cfg);
void sensor_print_config (FILE *fp, Configuration *cfg);
void group_print (FILE* fp, Configuration* cfg );
#endif
