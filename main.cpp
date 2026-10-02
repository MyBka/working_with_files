#include <iostream>


void counters(FILE* file, int* bytes, int* lines, int* alnum, int* words) {
    const size_t bufferSize = 256;
    char buffer[bufferSize];
    char lastChar = '\0';


    size_t readFile;
    int startWord = 0;
    while ((readFile = fread(buffer, 1, bufferSize, file)) != 0) {

        for (int i = 0; i < readFile; i++) {
            lastChar = buffer[i];

            if (buffer[i] == '\n' && *lines != -1) {
                *lines += 1;
            }
            if (((buffer[i] >= '0' && buffer[i] <= '9') || (std::isalpha(buffer[i]))) && *alnum != -1) {
                *alnum += 1;
                
            }
            if (!(buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t' || buffer[i] == '\r' 
                || buffer[i] == '\v' || buffer[i] == '\f')) {
                    startWord = 1;
            } else {
                if (startWord == 1 && *words != -1) {
                    startWord = 0;
                    *words += 1;
                }
            }
            


            if (*bytes != -1) {
                *bytes += 1;
            }
        }
    }
    if (lastChar != '\0' && lastChar != '\n' && *lines != -1) {
        *lines += 1;
    }
    if (startWord == 1 && *words != -1) {
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
                    j++;
                }
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


    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            FILE* file = fopen(argv[i], "r");
            if (!file) {
                std::cout << "No such file" << std::endl;
                continue;
            }
            
            int simbol;
            if ((simbol = fgetc(file)) == EOF) {
                
                arguments(file, &bytes, &lines, &alnum, &words, argc, argv);
                for (int j = 0; j < 4; j++) {
                    if (*result[j] >= 0) {
                        std::cout << *result[j] << " ";
                    }
                }
                int charcounter = 0;
                while (argv[i][charcounter] != '\0') {
                    std::cout << argv[i][charcounter];
                    charcounter++;
                }
                std::cout << '\n';
                continue;
            }
            rewind(file);

            arguments(file, &bytes, &lines, &alnum, &words, argc, argv);

            counters(file, &bytes, &lines, &alnum, &words);

            for (int j = 0; j < 4; j++) {
                if (*result[j] != -1) {
                    std::cout << *result[j] << " ";
                }
            }
            
            int charcounter = 0;
            while (argv[i][charcounter] != '\0') {
                std::cout << argv[i][charcounter];
                charcounter++;
            }
        
        
            fclose(file);
            std::cout << '\n';

            lines = -1;
            bytes = -1;
            alnum = -1;
            words = -1;
        }
        else {
            continue;
        }
        


        
    }

    return 0;
}