#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(){
    double f = 39.375;
    int m = static_cast<int>(f);
    int t = 256;
    int a = m>>3;
    int b = m>>16;
    printf("%d %d %d", t, a, b);
}