#include "headers.h"

//Compara duas imagens para ordenação na B-Tree
//Nome: prioridade máxima
//FilterMode: prioridade secundária
//ThresholdValue: prioridade terciária (apenas se FilterMode for FILTER_THRESHOLD)
//Duas imagens negativas ou sem filtro de mesmo serão negadas
int compareImageKeys(const IImage *a, const IImage *b) {
    int nameCompare = strcmp(a->name, b->name);
    if (nameCompare != 0) {
        return nameCompare;
    }
    
    if (a->filterMode != b->filterMode) {
        return a->filterMode - b->filterMode;
    }
    
    if (a->filterMode == FILTER_THRESHOLD) {
        return (int)a->thresholdValue - (int)b->thresholdValue;
    }
    
    if (a->filterMode == FILTER_NEGATIVE) {
        return 0; 
    }

    return 0;
}