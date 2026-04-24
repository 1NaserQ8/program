#include "csapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *classify_bmi(double bmi, int age) {
    if (age < 18) {
        if (bmi < 16.8) return "Underweight";
        else if (bmi < 22.9) return "Healthy weight";
        else if (bmi < 27.8) return "Overweight";
        else return "Obese";
    } else if (age <= 64) {
        if (bmi < 18.5) return "Underweight";
        else if (bmi <= 24.9) return "Healthy weight";
        else if (bmi <= 29.9) return "Overweight";
        else return "Obese";
    } else {
        if (bmi < 22) return "Underweight";
        else if (bmi < 28) return "Healthy weight";
        else if (bmi < 30) return "Overweight";
        else return "Obese";
    }
}

int main(void) {
    char *query = getenv("QUERY_STRING");
    double height = 0.0, weight = 0.0, bmi;
    int age = 0;

    printf("Content-type: text/html\r\n\r\n");

    if (query == NULL) {
        printf("<html><body><h2>No input</h2></body></html>");
        return 0;
    }

    sscanf(query, "height=%lf&weight=%lf&age=%d", &height, &weight, &age);

    if (height <= 0 || weight <= 0 || age < 0) {
        printf("<html><body><h2>Invalid input</h2></body></html>");
        return 0;
    }

    bmi = weight / (height * height);

    printf("<html><body>");
    printf("<h1>BMI Calculator Result</h1>");
    printf("<p>Height: %.2f m</p>", height);
    printf("<p>Weight: %.2f kg</p>", weight);
    printf("<p>Age: %d</p>", age);
    printf("<p>BMI: %.2f</p>", bmi);
    printf("<p>Classification: %s</p>", classify_bmi(bmi, age));
    printf("</body></html>");

    return 0;
}
