#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    int opt;
    bool fullOutput = false;

    while ((opt = getopt(argc, argv, "f")) != -1) {
        switch (opt) {
            case 'f':
                fullOutput = true;
                break;
            case '?':
            default:
                printf("Error: unknown key. Use -f for the full mathematical expression.\n");
                return 1;
        }
    }

    if (argc - optind != 3) {
        printf("Error: Invalid input.\n", argv[0]);
        return 1;
    }

    char *endptr;

    double a = strtod(argv[optind], &endptr);
    if (*endptr != '\0') {
        printf("Error: First operand '%s' is not a number.\n", argv[optind]);
        return 1;
    }

    char oper = argv[optind + 1][0];
    if (argv[optind + 1][1] != '\0' || (oper != '+' && oper != '-' && oper != '*' && oper != '/')) {
        printf("Error: Unknown operator '%s'. Exist: +, -, *, /\n", argv[optind + 1]);
        return 1;
    }

    double b = strtod(argv[optind + 2], &endptr);
    if (*endptr != '\0') {
        printf("Error: Second operand '%s' is not a number.\n", argv[optind + 2]);
        return 1;
    }

    if (oper == '/' && b == 0) {
        printf("Error: Division by zero.\n");
        return 1;
    }

    assert(oper == '+' || oper == '-' || oper == '*' || oper == '/');
    if (oper == '/') {
        assert(b != 0);
    }

    double result = 0.0;
    switch (oper) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/': result = a / b; break;
    }

    if (fullOutput) {
        printf("%.2f %c %.2f = %.2f\n", a, oper, b, result);
    } else {
        printf("%.2f\n", result);
    }

    return 0;
}