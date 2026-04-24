#include "csapp.h"

const char *classify_bmi(double bmi, int age)
{
    if (age < 18) {
        if (bmi < 16.8) return "Underweight";
        else if (bmi < 22.9) return "Healthy weight";
        else if (bmi < 27.8) return "Overweight";
        else return "Obese";
    }
    else if (age <= 64) {
        if (bmi < 18.5) return "Underweight";
        else if (bmi <= 24.9) return "Healthy weight";
        else if (bmi <= 29.9) return "Overweight";
        else return "Obese";
    }
    else {
        if (bmi < 22) return "Underweight";
        else if (bmi < 28) return "Healthy weight";
        else if (bmi < 30) return "Overweight";
        else return "Obese";
    }
}

int main(void)
{
    char *query = getenv("QUERY_STRING");
    double height, weight, bmi;
    int age;

    printf("Content-type: text/html\r\n\r\n");

    if (query == NULL) {
        printf("<html><body><h2>No input</h2></body></html>");
        return 0;
    }

    sscanf(query, "height=%lf&weight=%lf&age=%d", &height, &weight, &age);

    bmi = weight / (height * height);

    printf("<html><body>");
    printf("<h1>BMI Calculator Result</h1>");
    printf("<p>Height: %.2f m</p>", height);
    printf("<p>Weight: %.2f kg</p>", weight);
    printf("<p>Age: %d</p>", age);
    printf("<p><b>BMI = %.2f</b></p>", bmi);
    printf("<p><b>Classification: %s</b></p>", classify_bmi(bmi, age));
    printf("</body></html>");

    return 0;
}
