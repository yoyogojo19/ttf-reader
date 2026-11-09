#pragma once
#include <ios>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#ifndef READER_H
#define READER_H

std::ifstream ttfFile("../JetBrainsMono-Bold.ttf", std::ios::binary);

uint64_t cursor = 0;

void skipBytes(uint64_t bytes) {
    cursor += bytes;
}

void goTo(uint64_t location) {
    cursor = location;
}

uint8_t readByte() {
    ttfFile.seekg(cursor);
    char byte; 
    ttfFile.read(&byte, 1);
    cursor += 1;
    return static_cast<uint8_t>(byte);
}

uint16_t readUInt16() {
    uint8_t a = readByte();
    uint8_t b = readByte();
    return (static_cast<uint16_t>(a) << 8) | static_cast<uint16_t>(b);
}

int16_t readInt16() {
    uint16_t val = readUInt16();
    return static_cast<int16_t>(val);
}

uint32_t readUInt32() {
    uint16_t a = readUInt16();
    uint16_t b = readUInt16();
    return (static_cast<uint32_t>(a) << 16) | static_cast<uint32_t>(b);
}

std::string readTag() {
    std::string tag = "";
    char byte;
    for (int m = 0; m < 4; m++) {
        byte = readByte();
        tag += byte;
    } 
    return tag;
}

bool readFlag(uint8_t Flag, int bitIdx) {
    return ((Flag >> bitIdx) & 1) == 1;
}

struct tableDictonary {
    std::vector<std::string> names;
    std::vector<uint32_t> offsets;

    void add(std::string name, uint32_t offset) {
        names.push_back(name);
        offsets.push_back(offset);
    }

    uint32_t operator[](std::string table) {
        for (int i = 0; i < names.size(); i++) {
            if (names[i] == table) return offsets[i];
        }
        return 0;
    }
};

// code inspired by https://www.youtube.com/watch?v=SO83KQuuZvg&t=485s

std::vector<int> readCoords(uint8_t flags[], int numPoints, bool readingx) {
    // set wich falg to look at
    int sizeBit = readingx ? 1 : 2;
    int skipOrSignBit = readingx ? 4 : 5;
    std::vector<int> coordinates;
    for (int i = 0; i < numPoints; i++){
        coordinates.push_back(0);
    }

    for (int i = 0; i < numPoints; i++) {
        // coordinates are based on privious if is first starts at zero
        coordinates[i] = coordinates[std::max(0, i-1)];
        uint8_t Flag = flags[i];

        // if offset is one byte long
        if (readFlag(Flag, sizeBit)) {
            int offset = readByte();
            int sign = readFlag(Flag, skipOrSignBit) ? 1 : -1;
            coordinates[i] += offset*sign;
        } else if (!readFlag(Flag, skipOrSignBit)) { // if offset is two bytes long
            coordinates[i] += static_cast<int>(readInt16());
        }
    }
    return coordinates;
}

// code inspired by https://www.youtube.com/watch?v=SO83KQuuZvg&t=465s

struct GlyphData {
    std::vector<int> coordsX;
    std::vector<int> coordsY;
    std::vector<int> endEdicies;

    GlyphData(std::vector<int> coordsX, std::vector<int> coordsY, std::vector<int> endEdicies) : coordsX(coordsX), coordsY(coordsY), endEdicies(endEdicies) {}
};

std::vector<GlyphData> allGlyphs;

int readSimpleGlyph() {
    int numContours = readInt16();// get the number of contours
    if (numContours < 0) {
        return 0;
    } 
    std::cout << "simple glyph : " << numContours <<"\n";
    std::vector<int> endEdicies; 
    skipBytes(8); // skip boud size

    // get all of the contours
    for (int i = 0; i < numContours; i++) {
        endEdicies.push_back(readUInt16());
    }
    // get the flags
    int numPoints = endEdicies[numContours-1] + 1;
    uint8_t flags[numPoints];
    skipBytes(readInt16()); // skip instructions
    for (int i = 0; i < numPoints; i++) {
        uint8_t Flag = readByte();
        flags[i] = Flag;
        // if the REAPETE bit is set repete for the uint16 num of times
        if (readFlag(Flag, 3)) {
            int numofrepitiion = readByte();
            for (int j = 0; j < numofrepitiion; j++) {
                flags[++i] = Flag;
            }
        } 
    }
    std::vector<int> coordsX = readCoords(flags, numPoints, true);
    std::vector<int> coordsY = readCoords(flags, numPoints, false);
    allGlyphs.push_back(GlyphData(coordsX, coordsY, endEdicies));
    return allGlyphs.size()-1;
}

#endif