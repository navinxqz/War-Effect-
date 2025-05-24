#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cmath>

GLuint sceneTexture, tankTexture;
int tankImgWidth, tankImgHeight;

float tankX = 1.5f;
float tankY = 0.28f;
float tankSpeedX = -0.002f;
float tankSpeedY = -0.001f;
bool stopAtCenter = false;

float flameTime = 0.0f; // For flickering animation

// Fixed flame positions on buildings
struct FlamePos {
    float x, y;
};
FlamePos flames[] = {
    {-0.6f, -0.15f},
    {-0.2f,  0.05f},
    { 0.3f, -0.05f}
};
int flameCount = sizeof(flames) / sizeof(flames[0]);

// Load texture from file
GLuint loadTexture(const char* filename, int* outWidth = nullptr, int* outHeight = nullptr) {
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (!image) {
        printf("Failed to load image: %s\n", filename);
        return 0;
    }

    if (outWidth) *outWidth = width;
    if (outHeight) *outHeight = height;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return textureID;
}

// Draw a flickering flame at a fixed location
void drawFlame(float baseX, float baseY) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);

    float flameHeight = 0.25f;
    float flameWidth = 0.07f;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 0.5f, 0.0f, 0.8f);
    glVertex2f(baseX, baseY);

    int segments = 20;
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        float x = baseX + sinf(t * 10 + flameTime) * 0.015f * (1.0f - t); // flicker
        float y = baseY + t * flameHeight;
        float alpha = 1.0f - t;
        glColor4f(1.0f, 1.0f - t, 0.0f, alpha);
        glVertex2f(x, y);
    }
    glEnd();

    glColor4f(1, 1, 1, 1);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Enable texturing
    glEnable(GL_TEXTURE_2D);

    // Draw background
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
    glEnd();

    // Draw tank
    glPushMatrix();
    glTranslatef(tankX, tankY, 0.0f);
    glBindTexture(GL_TEXTURE_2D, tankTexture);
    float aspect = (float)tankImgWidth / (float)tankImgHeight;
    float displayWidth = 0.4f;
    float displayHeight = displayWidth / aspect;

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-displayWidth / 2, -displayHeight / 2);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(displayWidth / 2, -displayHeight / 2);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(displayWidth / 2, displayHeight / 2);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-displayWidth / 2, displayHeight / 2);
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    // Draw flames at building positions
    for (int i = 0; i < flameCount; i++) {
        drawFlame(flames[i].x, flames[i].y);
    }

    glutSwapBuffers();
}

void update(int value) {
    flameTime += 0.1f; // Animate flicker

    if (!stopAtCenter) {
        tankX += tankSpeedX;
        tankY += tankSpeedY;
        if (tankX <= 0.0f && tankY <= -0.4f) {
            tankX = 0.0f;
            tankY = -0.4f;
            stopAtCenter = true;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    sceneTexture = loadTexture("scene3.png");
    tankTexture = loadTexture("truck.png", &tankImgWidth, &tankImgHeight);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Tank Animation with Flames on Buildings");

    init();
    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}
