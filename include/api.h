#ifndef API_H
#define API_H
#include <Arduino.h>
#include <vector>

struct Tram {
    String line;
    String dest;
    int mins;
};

std::vector<Tram> fetchTrams();
int getLastHttpCode();
int getLastHtmlSize();
int getLastFoundEntries();

#endif
