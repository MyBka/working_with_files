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

void arguments(FILE* file, int* bytes, int* lines, int* alnum, int* words, int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') {
                if (argv[i][2] == 'l') {
                    *lines = 0;
                }
                if (argv[i][2] == 'w') {
                    *words = 0;
                }
                if (argv[i][2] == 'b') {
                    *bytes = 0;
                }
                if (argv[i][2] == 'a') {
                    *alnum = 0;
                }
            }
            else {
                int j = 1;
                while (argv[i][j] != '\0') {
                    if (argv[i][j] == 'l') {
                        *lines = 0;
                    }
                    if (argv[i][j] == 'w') {
                        *words = 0;
                    }
                    if (argv[i][j] == 'c') {
                        *bytes = 0;
                    }
                    if (argv[i][j] == 'a') {
                        *alnum = 0;
                    }
                }
                j++;
            }
        }
    }
    if (*bytes == -1 && *lines == -1 && *alnum == -1 && *words == -1) {
        *lines = 0;
        *words = 0;
        *bytes = 0;
    }
} 

int main(int argc, char** argv) {
    int lines = -1, bytes = -1, words = -1, alnum = -1;
    int* result[4] = {&lines, &words, &alnum, &bytes};
     
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