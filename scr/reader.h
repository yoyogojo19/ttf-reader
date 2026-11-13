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
    cursor++;
    return static_cast<uint8_t>(byte);
}

uint16_t readUInt16() {
    uint16_t a = readByte();
    uint16_t b = readByte();
    return (a << 8) | b;
}

int16_t readInt16() {
    uint16_t val = readUInt16();
    return static_cast<int16_t>(val);
}

uint32_t readUInt32() {
    uint32_t a = readByte();
    uint32_t b = readByte();
    uint32_t c = readByte();
    uint32_t d = readByte();
    return (a << 24) | (b << 16) | (c << 8) | d;
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

struct GlyphTable {
    std::vector<uint32_t> offsets;
    std::vector<GlyphData> glyphs;

    void addOffset(uint32_t Offset) {
        offsets.push_back(Offset);
    }

    void addGlyph(GlyphData Glyph) {
        glyphs.push_back(Glyph);
    }

    GlyphData operator[](int idx) {
        return glyphs[idx];
    }
};

GlyphTable allGlyphs;

GlyphData readSimpleGlyph() {
    int numContours = readInt16();// get the number of contours
    if (numContours < 0) {
        return allGlyphs[0];
    }
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
    return GlyphData(coordsX, coordsY, endEdicies);
}

#endif