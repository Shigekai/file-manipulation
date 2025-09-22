//
// Created by costa on 21/09/2025.
//

#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h> // For uint32_t
#include "imgdb.h"  // For KeyEntry

int export_image(const KeyEntry *e, const char *out_path, int mode, uint32_t thresholdVal);

#endif //FILTERS_H