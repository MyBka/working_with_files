#include <iostream>


void counters(FILE* file, int* bytes, int* lines, int* alnum, int* words) {
    const size_t bufferSize = 256;
    char buffer[bufferSize];


    size_t readFile;
    int startWord = 0;
    while ((readFile = fread(buffer, 1, bufferSize, file)) != 0) {

        for (int i = 0; i < readFile; i++) {
            if (buffer[i] == '\n') {
                *lines += 1;
            }
            if ((buffer[i] >= '0' && buffer[i] <= '9') || (std::isalpha(buffer[i]))) {
                *alnum += 1;
                
            }
            if (!(buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t' || buffer[i] == '\r' 
                || buffer[i] == '\v' || buffer[i] == '\f')) {
                    startWord = 1;
            } else {
                if (startWord == 1) {
                    startWord = 0;
                    *words += 1;
                }
            }
            



            *bytes += 1;
        }
    }
    if (*bytes > 0) {
        *lines += 1;
    }
    if (startWord == 1) {
        *words += 1;
    }
    
}

int main(int argc, char** argv) {
    int lines = -1, bytes = -1, words = -1, alnum = -1;
     
    FILE* file = fopen(argv[argc-1], "r");

    if (!file) {
        std::cout << "No such file" << std::endl;
        return 1;
    }
    
    int simbol;
    if ((simbol = fgetc(file)) == EOF) {
        // Сразу передаем функцию, где будут нулевые переменные (которые надо выводить, остальные скрыты)
        return 2;
    }
    rewind(file);



    fclose(file);
    std::cout << "Размер файла в байтах: " << bytes << std::endl;



    return 0;
}