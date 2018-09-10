extern int globalData;

void __attribute__((constructor)) writeToData() {
    globalData = 123;
}

void __attribute__((constructor)) incrementData() {
    ++globalData;
}

