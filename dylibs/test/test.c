int other(int x);

extern int globalData;

int testfunction(int x, int y) {
    ++globalData;
    return x + other(y - globalData);
}
