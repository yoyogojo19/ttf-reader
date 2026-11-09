#include <cstddef>
#include <cstdint>
#include <iostream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "reader.h"

int main() {
    // find the number of tables
    skipBytes(4); // skip scalar type
    int tableNum = readUInt16();
    std::cout << "Number of tables : " << tableNum << "\n";

    // get all the tables in a custom Dictionnary
    tableDictonary allTables;
    skipBytes(6); // skip Search range, Entry selector and Range shift
    for (int i = 0; i < tableNum; i++) {
        std::string tag = readTag();
        skipBytes(4); // skip Check sum
        uint32_t Offset = readUInt32();
        skipBytes(4); // skip Length
        allTables.add(tag, Offset);
    }

    // go to the Glyph table
    goTo(allTables["glyf"]);
    for (int i = 0; i < 2; i ++) {
        readSimpleGlyph();
    } 
    GlyphData glyf0 = readSimpleGlyph();
    std::cout << "Glyph0 :\n";
    for (auto i : glyf0.endEdicies) {
        std::cout << "End indicies : " << i << "\n";
    } for (int i = 0; i < glyf0.coordsX.size(); i++) {
        std::cout << "Point" << i << " : " << glyf0.coordsX[i] << "," << glyf0.coordsY[i] << "\n";
    }

    glfwInit();
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "TTF test", NULL, NULL);
    glewInit();
    glfwMakeContextCurrent(window);

    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glPointSize(20.0f);
        glColor3f(1.0f,1.0f,1.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < glyf0.coordsX.size(); i++) {
            glVertex2f(static_cast<float>(glyf0.coordsX[i]) / 1920.0, static_cast<float>(glyf0.coordsY[i]) / 1080.0);
        }
        glEnd();
        glBegin(GL_LINES);
        for (int i = 0; i < glyf0.endEdicies.size(); i++) {
            switch (i) {
                case 0:
                    glColor3f(1.0f, 0.0f, 0.0f);
                    break;
                case 1:
                    glColor3f(0.0f, 1.0f, 0.0f);
                    break;
                case 2:
                    glColor3f(0.0f, 0.0f, 1.0f);
                    break;
            }
            for (int j = (i-1 < 0 ? 0 : glyf0.endEdicies[i-1]+1); j <= glyf0.endEdicies[i]; j++) {
                glVertex2f(static_cast<float>(glyf0.coordsX[j]) / 1920.0, static_cast<float>(glyf0.coordsY[j]) / 1080.0);
                glVertex2f(
                    static_cast<float>((j+1 <= glyf0.endEdicies[i]) ? glyf0.coordsX[j+1] : glyf0.coordsX[(i-1 < 0 ? 0 : glyf0.endEdicies[i-1]+1)]) / 1920.0,
                    static_cast<float>((j+1 <= glyf0.endEdicies[i]) ? glyf0.coordsY[j+1] : glyf0.coordsY[(i-1 < 0 ? 0 : glyf0.endEdicies[i-1]+1)]) / 1080.0
                );
            }
        }
        glEnd();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    return 0;
}