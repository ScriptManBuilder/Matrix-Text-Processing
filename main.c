#define _CRT_SECURE_NO_WARNINGS   // <-- Ставим до подключений
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX_FILENAME 256  // Максимальная длина имени файла
// Функция для чтения матрицы из файла (без изменений)
void readMatrixFromFile(const char* filename, float*** matrix, int* rows, int* cols) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        exit(1);
    }

    fscanf(file, "%d %d", rows, cols);

    *matrix = (float**)malloc(*rows * sizeof(float*));
    for (int i = 0; i < *rows; ++i) {
        (*matrix)[i] = (float*)malloc(*cols * sizeof(float));
        for (int j = 0; j < *cols; ++j) {
            fscanf(file, "%f", &(*matrix)[i][j]);
        }
    }
    fclose(file);
}

//-----------------------------------------
// ПОСЛЕДОВАТЕЛЬНЫЕ ФУНКЦИИ
//-----------------------------------------

// Последовательное умножение матрицы на коэффициент и подсчёт суммы
float multiplyMatrixAndSumSequential(float** matrix, int rows, int cols, float coefficient) {
    float sum = 0.0f;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] *= coefficient;
            sum += matrix[i][j];
        }
    }
    return sum;
}

// Последовательный подсчёт количества строк, где есть нулевой элемент
int countRowsWithZeroSequential(float** matrix, int rows, int cols) {
    int count = 0;
    for (int i = 0; i < rows; i++) {
        int rowHasZero = 0;
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] == 0) {
                rowHasZero = 1;
                break;
            }
        }
        count += rowHasZero;
    }
    return count;
}

// Последовательное умножение матрицы на вектор
void multiplyMatrixByVectorSequential(float** matrix, float* vector, int rows, int cols, float* result) {
    for (int i = 0; i < rows; i++) {
        result[i] = 0.0f;
        for (int j = 0; j < cols; j++) {
            result[i] += matrix[i][j] * vector[j];
        }
    }
}

//-----------------------------------------
// ПАРАЛЛЕЛЬНЫЕ ФУНКЦИИ (OpenMP)
//-----------------------------------------

// Параллельное умножение матрицы на коэффициент и подсчёт суммы
float multiplyMatrixAndSumParallel(float** matrix, int rows, int cols, float coefficient) {
    float sum = 0.0f;

    // Используем reduction(+:sum), чтобы корректно суммировать
    // значения из всех потоков в одну переменную sum
#pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] *= coefficient;
            sum += matrix[i][j];
        }
    }
    return sum;
}

// Параллельный подсчёт количества строк, где есть нулевой элемент
int countRowsWithZeroParallel(float** matrix, int rows, int cols) {
    int count = 0;
#pragma omp parallel for reduction(+:count)
    for (int i = 0; i < rows; i++) {
        int rowHasZero = 0;
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] == 0) {
                rowHasZero = 1;
                break;
            }
        }
        count += rowHasZero;
    }
    return count;
}

// Параллельное умножение матрицы на вектор
void multiplyMatrixByVectorParallel(float** matrix, float* vector, int rows, int cols, float* result) {
#pragma omp parallel for
    for (int i = 0; i < rows; i++) {
        float temp = 0.0f;
        for (int j = 0; j < cols; j++) {
            temp += matrix[i][j] * vector[j];
        }
        result[i] = temp;
    }
}

//-----------------------------------------
// Остальные функции (extractNonZeroElements, averageInWindow) 
// можно тоже параллелить аналогичным образом, но 
// в некоторых случаях придётся аккуратно собирать результаты.
//-----------------------------------------

// Функция для выбора всех ненулевых элементов (пока оставим последовательную)
void extractNonZeroElements(float** matrix, int rows, int cols, float** result, int* size) {
    *size = 0;
    // Считаем количество ненулевых элементов
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] != 0) {
                (*size)++;
            }
        }
    }

    *result = (float*)malloc(*size * sizeof(float));
    int idx = 0;
    // Копируем ненулевые элементы
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] != 0) {
                (*result)[idx++] = matrix[i][j];
            }
        }
    }
}

// Функция для усреднения значений каждого окна размером N (тоже пока оставим последовательную)
void averageInWindow(float* vector, int length, int N, float** result, int* resultSize) {
    *resultSize = length / N;
    *result = (float*)malloc(*resultSize * sizeof(float));
    for (int i = 0; i < *resultSize; i++) {
        float sum = 0;
        for (int j = 0; j < N; j++) {
            sum += vector[i * N + j];
        }
        (*result)[i] = sum / N;
    }
}

int main() {
    char matrixFile[MAX_FILENAME];
    char outputFile[MAX_FILENAME];

    // Запрос имен файлов с клавиатуры
    printf("Enter the name of the input file with the matrix ( input.txt ): ");
    scanf("%255s", matrixFile);  // ограничиваем количество символов
    printf("Enter the output file name for the results ( output.txt ): ");
    scanf("%255s", outputFile);
    float coefficient = 2.5f;

    float** matrix;
    int rows, cols;
    readMatrixFromFile(matrixFile, &matrix, &rows, &cols);

    // Создадим вектор для умножения "матрица-вектор"
    float* vector = (float*)malloc(cols * sizeof(float));
    for (int i = 0; i < cols; i++) {
        vector[i] = (float)(i + 1);
    }
    float* vectorResult = (float*)malloc(rows * sizeof(float));

    // Открываем файл для вывода
    FILE* output = fopen(outputFile, "w");
    if (!output) {
        printf("Error opening output file!\n");
        return 1;
    }

    //----------------------------------------------------------------------
    // 1) ПОСЛЕДОВАТЕЛЬНЫЙ РЕЖИМ
    //----------------------------------------------------------------------
    clock_t startSeq = clock();

    // Умножение матрицы на коэффициент и суммирование
    float sumSeq = multiplyMatrixAndSumSequential(matrix, rows, cols, coefficient);

    // Подсчёт строк с нулевым элементом
    int zeroRowsCountSeq = countRowsWithZeroSequential(matrix, rows, cols);

    // Умножение матрицы на вектор
    multiplyMatrixByVectorSequential(matrix, vector, rows, cols, vectorResult);

    clock_t endSeq = clock();
    double timeSequential = (double)(endSeq - startSeq) / CLOCKS_PER_SEC;
    // --- Вывод в консоль ---
    printf("=== SEQUENTIAL RESULTS ===\n");
    printf("Sum after multiply: %.2f\n", sumSeq);
    printf("Rows with zero: %d\n", zeroRowsCountSeq);
    printf("Matrix-Vector result (first 5 elements): ");
    for (int i = 0; i < (rows < 5 ? rows : 5); i++) {
        printf("%.2f ", vectorResult[i]);
    }
    printf("\nSequential Time: %.6f seconds\n\n", timeSequential);

    // --- Вывод в файл ---
    fprintf(output, "=== SEQUENTIAL RESULTS ===\n");
    fprintf(output, "Sum after multiply: %.2f\n", sumSeq);
    fprintf(output, "Rows with zero: %d\n", zeroRowsCountSeq);
    fprintf(output, "Matrix-Vector result (first 5 elements): ");
    for (int i = 0; i < (rows < 5 ? rows : 5); i++) {
        fprintf(output, "%.2f ", vectorResult[i]);
    }
    fprintf(output, "\nSequential Time: %.6f seconds\n\n", timeSequential);

 

    //----------------------------------------------------------------------
    // 2) ПАРАЛЛЕЛЬНЫЙ РЕЖИМ (OpenMP)
    //----------------------------------------------------------------------
    // Заново читаем матрицу, чтобы вернуть её в исходное состояние
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    readMatrixFromFile(matrixFile, &matrix, &rows, &cols);

    clock_t startPar = clock();

    float sumPar = multiplyMatrixAndSumParallel(matrix, rows, cols, coefficient);
    int zeroRowsCountPar = countRowsWithZeroParallel(matrix, rows, cols);
    multiplyMatrixByVectorParallel(matrix, vector, rows, cols, vectorResult);

    clock_t endPar = clock();
    double timeParallel = (double)(endPar - startPar) / CLOCKS_PER_SEC;


    // --- Вывод в консоль ---
    printf("=== PARALLEL RESULTS (OpenMP) ===\n");
    printf("Sum after multiply: %.2f\n", sumPar);
    printf("Rows with zero: %d\n", zeroRowsCountPar);
    printf("Matrix-Vector result (first 5 elements): ");
    for (int i = 0; i < (rows < 5 ? rows : 5); i++) {
        printf("%.2f ", vectorResult[i]);
    }
    printf("\nParallel Time: %.6f seconds\n\n", timeParallel);

    // --- Вывод в файл ---
    fprintf(output, "=== PARALLEL RESULTS (OpenMP) ===\n");
    fprintf(output, "Sum after multiply: %.2f\n", sumPar);
    fprintf(output, "Rows with zero: %d\n", zeroRowsCountPar);
    fprintf(output, "Matrix-Vector result (first 5 elements): ");
    for (int i = 0; i < (rows < 5 ? rows : 5); i++) {
        fprintf(output, "%.2f ", vectorResult[i]);
    }
    fprintf(output, "\nParallel Time: %.6f seconds\n\n", timeParallel);


    printf("\nThe program has worked successfully and completed its execution. ");


    // Закрываем файл
    fclose(output);

    // Освобождаем память
    free(vector);
    free(vectorResult);
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return 0;
}