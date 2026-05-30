#include <stdio.h>
#include <stdlib.h>


void vuln() {
    char buf[64];
    // 故意把读取长度写成200，造成溢出
    fgets(buf, 200, stdin); 
}

int main() {
    printf("Input:\n");
    vuln();
    return 0;
}
