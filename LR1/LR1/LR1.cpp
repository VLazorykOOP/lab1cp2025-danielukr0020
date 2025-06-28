// task1.cpp
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// ===================== Власні винятки =====================
// Помилка відкриття файлу
class FileError : public exception {
    string message;
public:
    FileError(const string& msg) : message("FileError: " + msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

// Помилка ділення на нуль
class DivisionByZeroError : public exception {
    string message;
public:
    DivisionByZeroError(const string& msg) : message("DivisionByZeroError: " + msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

// ===================== Утилітарні функції =====================
// Структура для зберігання одного запису таблиці
struct TableEntry {
    double x, T, U;
};

// Зчитування таблиці з файлу
vector<TableEntry> readTable(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) throw FileError("Cannot open file " + filename);

    vector<TableEntry> data;
    TableEntry e;
    while (file >> e.x >> e.T >> e.U) {
        data.push_back(e);
    }
    return data;
}

// Лінійна інтерполяція по x (T або U)
double interpolate(double x, const vector<TableEntry>& table, bool useT) {
    for (size_t i = 0; i < table.size() - 1; ++i) {
        if (x >= table[i].x && x <= table[i + 1].x) {
            double x0 = table[i].x, x1 = table[i + 1].x;
            double y0 = useT ? table[i].T : table[i].U;
            double y1 = useT ? table[i + 1].T : table[i + 1].U;
            return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
        }
    }
    throw out_of_range("x out of interpolation range");
}

// ===================== Прототипи функцій =====================
double T(double x);
double U(double x);
double Srz(double x, double y, double z);
double Gold(double x, double y);
double Glr(double x, double y);
double Grs(double x, double y, double z);
double fun(double x, double y, double z);
double fun_alg2(double x, double y, double z);
double fun_alg3(double x, double y, double z);

// ===================== Реалізації функцій =====================
// Функція T(x) зчитує з відповідного файлу
// залежно від значення x

// Зчитування значення T(x) з відповідного .dat файлу
// Використовується інтерполяція

double T(double x) {
    string file;
    if (x <= 1) file = "dat_X_1_1.dat";
    else if (x < -1) { x = 1 / x; file = "dat_X00_1.dat"; }
    else { x = 1 / x; file = "dat_X1_00.dat"; }

    auto table = readTable(file);
    return interpolate(x, table, true);
}

// Зчитування значення U(x) з відповідного .dat файлу
// Використовується інтерполяція

double U(double x) {
    string file;
    if (x <= 1) file = "dat_X_1_1.dat";
    else if (x < -1) { x = 1 / x; file = "dat_X00_1.dat"; }
    else { x = 1 / x; file = "dat_X1_00.dat"; }

    auto table = readTable(file);
    return interpolate(x, table, false);
}

// Реалізація функції Srz згідно умови
// У разі помилки переходить до Алгоритму 3

double Srz(double x, double y, double z) {
    try {
        if (z <= x + y)
            return T(z) + U(y) - U(y);
        else
            return T(x) + T(z) - U(y);
    }
    catch (...) {
        return fun_alg3(x, y, z);
    }
}

// Реалізація функції Gold з обробкою ділення на нуль

double Gold(double x, double y) {
    if (x != 0 && x < y) return x / y;
    if (y != 0 && y > x) return y / x;
    throw DivisionByZeroError("Invalid x/y in Gold function");
}

// Реалізація Glr згідно умови

// Використовує Алгоритм 2, якщо сума квадратів менша за 1.04

double Glr(double x, double y) {
    double sum = x * x + y * y;
    if (sum < 1.04) return fun_alg2(x, y, 1.0); // fallback
    if (x >= 1 && y >= 1) return 4.0 / sum;
    if (x >= 1) return y;
    return x;
}

// Основна функція Grs згідно алгоритму

// Обробка винятків ділення на нуль (Gold)

double Grs(double x, double y, double z) {
    try {
        double srz1 = Srz(x, y, z);
        double srz2 = Srz(z, x, y);
        return 1.389 * Glr(x, y) * Gold(x, y) * Srz(x, y, z) +
            0.838 * Gold(y, y / 5) * Glr(x, y) * srz2;
    }
    catch (DivisionByZeroError&) {
        return fun_alg2(x, y, z);
    }
}

// Головна функція fun
// Формула: fun = 0.33 * Grs(x,y,z) + Grs(y,z,x) + Grs(z,x,y)

double fun(double x, double y, double z) {
    return 0.33 * Grs(x, y, z) + Grs(y, z, x) + Grs(z, x, y);
}

// Алгоритм 2 (fallback)
double fun_alg2(double x, double y, double z) {
    return x + y + z; // спрощений варіант
}

// Алгоритм 3 (при помилці з файлами)
double fun_alg3(double x, double y, double z) {
    return 1.3498 * x + 2.2362 * y - 2.348 * x * y * z;
}

// ===================== Основна функція =====================
int main() {
    double x, y, z;
    cout << fixed << setprecision(4);
    cout << "Enter x, y, z: ";
    cin >> x >> y >> z;
    try {
        double result = fun(x, y, z);
        cout << "Result: " << result << endl;
    }
    catch (exception& e) {
        cout << "Error: " << e.what() << endl;
    }
    return 0;
}