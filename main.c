#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define _CRT_SECURE_NO_WARNINGS

// Функція для читання матриці з файлу
void readMatrixFromFile(const char* filename, float*** matrix, int* rows, int* cols) {
    FILE* file = fopen(filename, "r");  // Відкриваємо файл для читання
    if (file == NULL) {
        printf("Error opening file\n");  // Якщо файл не відкрився, виводимо помилку
        exit(1);
    }

    fscanf_s(file, "%d %d", rows, cols);  // Читаємо розміри матриці

    *matrix = (float**)malloc(*rows * sizeof(float*));  // Виділяємо пам'ять для матриці
    for (int i = 0; i < *rows; ++i) {
        (*matrix)[i] = (float*)malloc(*cols * sizeof(float));  // Виділяємо пам'ять для кожного рядка
        for (int j = 0; j < *cols; ++j) {
            fscanf_s(file, "%f", &(*matrix)[i][j]);  // Читаємо значення елементів матриці
        }
    }
    fclose(file);  // Закриваємо файл після читання
}

// Функція для запису результату в файл
void writeToFile(const char* filename, float sum) {
    FILE* file = fopen(filename, "w");  // Відкриваємо файл для запису
    if (file == NULL) {
        printf("Error opening file\n");  // Якщо файл не відкрився, виводимо помилку
        exit(1);
    }

    fprintf(file, "%.2f\n", sum);  // Записуємо результат в файл
    fclose(file);  // Закриваємо файл
}

// Функція для множення матриці на коефіцієнт та підрахунку суми елементів
float multiplyMatrixAndSum(float** matrix, int rows, int cols, float coefficient) {
    float sum = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] *= coefficient;  // Множимо елемент матриці на коефіцієнт
            sum += matrix[i][j];  // Додаємо елемент до загальної суми
        }
    }
    return sum;  // Повертаємо загальну суму
}

// Функція для вибору всіх ненульових елементів матриці в новий масив
void extractNonZeroElements(float** matrix, int rows, int cols, float** result, int* size) {
    *size = 0;

    // Спочатку підраховуємо кількість ненульових елементів
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] != 0) {  // Якщо елемент ненульовий
                (*size)++;
            }
        }
    }

    // Виділяємо пам'ять для масиву результатів
    *result = (float*)malloc(*size * sizeof(float));
    int idx = 0;

    // Тепер копіюємо ненульові елементи в новий масив
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] != 0) {  // Якщо елемент ненульовий
                (*result)[idx++] = matrix[i][j];  // Додаємо елемент у результат
            }
        }
    }
}

// Функція для підрахунку кількості рядків, де є нульовий елемент
int countRowsWithZero(float** matrix, int rows, int cols) {
    int count = 0;
    for (int i = 0; i < rows; i++) {
        int rowHasZero = 0;
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] == 0) {  // Якщо знайдено нульовий елемент
                rowHasZero = 1;
                break;
            }
        }
        if (rowHasZero) {
            count++;  // Підраховуємо рядки з нулями
        }
    }
    return count;  // Повертаємо кількість рядків з нулями
}

// Функція для усереднення значень кожного вікна з N елементів
void averageInWindow(float* vector, int length, int N, float** result, int* resultSize) {
    *resultSize = length / N;
    *result = (float*)malloc(*resultSize * sizeof(float));
    for (int i = 0; i < *resultSize; i++) {
        float sum = 0;
        for (int j = 0; j < N; j++) {
            sum += vector[i * N + j];  // Підсумовуємо елементи вікна
        }
        (*result)[i] = sum / N;  // Усереднюємо елементи
    }
}

// Функція для множення матриці на вектор
void multiplyMatrixByVector(float** matrix, float* vector, int rows, int cols, float* result) {
    for (int i = 0; i < rows; i++) {
        result[i] = 0;
        for (int j = 0; j < cols; j++) {
            result[i] += matrix[i][j] * vector[j];  // Множимо і додаємо до результату
        }
    }
}int main() {
    const char* matrixFile = "input.txt";  // Шлях до файлу матриці
    const char* outputFile = "output.txt"; // Шлях до файлу виводу
    float coefficient = 2.5;  // Коефіцієнт для множення

    float** matrix;
    int rows, cols;
    readMatrixFromFile(matrixFile, &matrix, &rows, &cols);  // Читання матриці з файлу

    // Відкриття файлу для запису
    FILE* output = fopen(outputFile, "w");
    if (!output) {
        printf("Error opening output file!\n");
        return 1;
    }

    /* Виведення початкової матриці ТІЛЬКИ У ФАЙЛ
    fprintf(output, "Initial Matrix:\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(output, "%.2f ", matrix[i][j]);
        }
        fprintf(output, "\n");
    }*/

    // Час виконання
    clock_t start, end;
    double cpu_time_used;

    start = clock();

    // Множення матриці на коефіцієнт та підрахунок суми елементів
    float sum = multiplyMatrixAndSum(matrix, rows, cols, coefficient);

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Запис результату у файл
    fprintf(output, "Sum of matrix after multiplication: %.2f\n", sum);
   

    printf("Sum of matrix after multiplication: %.2f\n", sum);
    

    // Вибір всіх ненульових елементів у новий масив
    float* nonZeroElements;
    int nonZeroSize;
    extractNonZeroElements(matrix, rows, cols, &nonZeroElements, &nonZeroSize);
    fprintf(output, "Non-zero elements count: %d\n", nonZeroSize);
    printf("Non-zero elements count: %d\n", nonZeroSize);

    // Підрахунок кількості рядків з нулями
    int zeroRowsCount = countRowsWithZero(matrix, rows, cols);
    fprintf(output, "Rows with zero: %d\n", zeroRowsCount);
    printf("Rows with zero: %d\n", zeroRowsCount);

    fprintf(output, "Execution Time: %.6f seconds\n", cpu_time_used);

    printf("Execution Time: %.6f seconds\n", cpu_time_used);
    // Усереднення елементів вектора по вікнах
    int N = 10;  // Розмір вікна
    float* avgResult;
    int avgSize;
    averageInWindow(nonZeroElements, nonZeroSize, N, &avgResult, &avgSize);
    fprintf(output, "Average result:\n");
    for (int i = 0; i < avgSize; i++) {
        fprintf(output, "%.2f ", avgResult[i]);
    }
    fprintf(output, "\n");

    // Множення матриці на вектор (ТІЛЬКИ У ФАЙЛ)
    float* vector = (float*)malloc(cols * sizeof(float));
    for (int i = 0; i < cols; i++) {
        vector[i] = i + 1.0f;
    }
    float* vectorResult = (float*)malloc(rows * sizeof(float));
    multiplyMatrixByVector(matrix, vector, rows, cols, vectorResult);

    fprintf(output, "Matrix-vector multiplication result:\n");
    for (int i = 0; i < rows; i++) {
        fprintf(output, "%.2f ", vectorResult[i]);
    }
    fprintf(output, "\n");

   
   
    // Закриття файлу
    fclose(output);

    // Вивільнення пам'яті
    free(nonZeroElements);
    free(avgResult);
    free(vector);
    free(vectorResult);
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return 0;
}
