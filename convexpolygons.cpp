#include <iostream>
#include <vector>
#include <algorithm>
#include <glut.h> //change to <GL/glut.h> if not building right
#include <cmath>

struct Vertex { //2d space with x and y
    float x;
    float y;
};

struct Line { // line ax+by+c=0
    float a;
    float b;
    float c;
};

struct Colour {
    float r;
    float g;
    float b;
};

std::vector<Vertex> vertices; //global vectors 
std::vector<Vertex> convexVertices; //store points

void setPixel(float x, float y, const Colour& colour) {
    glColor3f(colour.r, colour.g, colour.b);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd(); //set the color of a pixel at x,y position
} 
void drawLine(float x1, float y1, float x2, float y2, const Colour& colour) {
    glColor3f(colour.r, colour.g, colour.b);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd(); //draws a line between 2 points in x,y position
}

void drawPolygon(const std::vector<Vertex>& polygon, const Colour& colour) {
    int numVertices = polygon.size();
    for (int i = 0; i < numVertices; ++i) {
        int j = (i + 1) % numVertices;
        drawLine(polygon[i].x, polygon[i].y, polygon[j].x, polygon[j].y, colour); //
    }
}

bool isConvex(const std::vector<Vertex>& polygon) {
    int numVertices = polygon.size();
    if (numVertices < 3) {
        return false;  //fills - checks intersections 
    }

    bool isClockwise = false;
    bool isCounterClockwise = false;

    for (int i = 0; i < numVertices; ++i) {
        int j = (i + 1) % numVertices;
        int k = (i + 2) % numVertices;

        float crossProduct = (polygon[j].x - polygon[i].x) * (polygon[k].y - polygon[j].y) -
                             (polygon[j].y - polygon[i].y) * (polygon[k].x - polygon[j].x);

        if (crossProduct > 0) {
            isClockwise = true;
        } else if (crossProduct < 0) {
            isCounterClockwise = true;
        }

        // If both clockwise and counterclockwise orientations are detected,
        // the polygon is concave.
        if (isClockwise && isCounterClockwise) {
            return false;
        }
    }

    // If neither clockwise nor counterclockwise orientations are detected,
    // the polygon is degenerate.
    if (!isClockwise && !isCounterClockwise) {
        return false;
    }

    return true;
}

void convex1(const std::vector<Vertex>& polygon, const Colour& c) {
    int numVertices = polygon.size();
    if (numVertices < 3 || !isConvex(polygon)) {
        return;
    }

    std::vector<Line> lines;
    std::vector<float> es;
    lines.reserve(numVertices);
    es.reserve(numVertices);

    // Calculate the lines and initial estimates for each vertex
    for (int i = 0; i < numVertices; ++i) {
        int j = (i + 1) % numVertices;
        Line line;
        line.a = polygon[j].y - polygon[i].y;
        line.b = polygon[i].x - polygon[j].x;
        line.c = -line.a * polygon[i].x - line.b * polygon[i].y;
        lines.push_back(line);
        float e = line.a * polygon[i].x + line.b * polygon[i].y + line.c;
        es.push_back(e);
    }

    // Calculate the bounding box
    float bb_xmin = polygon[0].x;
    float bb_xmax = polygon[0].x;
    float bb_ymin = polygon[0].y;
    float bb_ymax = polygon[0].y;
    for (int i = 1; i < numVertices; ++i) {
        bb_xmin = std::min(bb_xmin, polygon[i].x);
        bb_xmax = std::max(bb_xmax, polygon[i].x);
        bb_ymin = std::min(bb_ymin, polygon[i].y);
        bb_ymax = std::max(bb_ymax, polygon[i].y);
    }

    // Fill the polygon using scanline algorithm
    for (int y = std::floor(bb_ymin); y <= std::ceil(bb_ymax); ++y) {
        bool insidePolygon = false;
        float et = 0;

        for (int x = std::floor(bb_xmin); x <= std::ceil(bb_xmax); ++x) {
            int windingNumber = 0;

            for (int i = 0; i < numVertices; ++i) {
                int j = (i + 1) % numVertices;
                if ((polygon[i].y <= y && polygon[j].y > y) || (polygon[j].y <= y && polygon[i].y > y)) {
                    float xi = (polygon[j].x - polygon[i].x) * (y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x;
                    if (x <= xi) {
                        ++windingNumber;
                    }
                }
            }

            if (windingNumber % 2 == 1) {
                setPixel(x, y, c);
            }
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the polygon if it is convex
    if (vertices.size() >= 3) {
        if (isConvex(vertices)) {
            Colour polygonColour{ 0.0f, 0.0f, 1.0f };
            drawPolygon(vertices, polygonColour);

            // Fill the convex polygon
            Colour fillColour{ 0.0f, 0.0f, 1.0f };
            convex1(vertices, fillColour);
        }
        else {
            Colour polygonColour{ 0.0f, 0.0f, 1.0f };
            drawPolygon(vertices, polygonColour);
        }
    }

    glFlush();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // ESC key
        exit(0);
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        Vertex vertex;
        vertex.x = x;
        vertex.y = glutGet(GLUT_WINDOW_HEIGHT) - y; // Flip y for OpenGL window coordinates

        if (vertices.size() < 3) {
            vertices.push_back(vertex);
        }
        else {
            std::vector<Vertex> tempVertices = vertices;
            tempVertices.push_back(vertex);
            if (isConvex(tempVertices)) {
                vertices.push_back(vertex);
            }
        }

        glutPostRedisplay();
    }
}

int main(int argc, char** argv) { //GLUT library
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Convex Polygon");
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMainLoop();

    return 0;
}
