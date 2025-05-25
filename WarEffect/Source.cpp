#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cmath>

GLuint sceneTexture, tankTexture, missileTexture, copperTexture;
int tankImgWidth, tankImgHeight;
int missileImgWidth, missileImgHeight;
int copperImgWidth, copperImgHeight;

//tank variables
float tankX = 1.2f, tankY = 0.18f;
float tankSpeedX = -0.002f, tankSpeedY = -0.00105f;
bool stop = false;

//missile variables
float missileX = -1.2f, missileY = 0.3f;
float missileSpeedX = 0.009, missileSpeedY = 0.0065f;

//copper variables
float copperX = -0.9f, copperY = -0.1f;
float copperSpeedX = 0.004, copperSpeedY = 0.0035f;
float rotorAngle = 0.0f;

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

    //missile
    glPushMatrix();
    glTranslatef(missileX, missileY, 0);
    glBindTexture(GL_TEXTURE_2D, missileTexture);
    float missileAspect = (float)missileImgWidth / missileImgHeight;
    float pw = 0.2f, ph = pw / missileAspect;
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-pw / 2, -ph / 2);
    glTexCoord2f(1, 0); glVertex2f(pw / 2, -ph / 2);
    glTexCoord2f(1, 1); glVertex2f(pw / 2, ph / 2);
    glTexCoord2f(0, 1); glVertex2f(-pw / 2, ph / 2);
    glEnd();
    glPopMatrix();

    //copper
    /*glPushMatrix();
    glTranslatef(copperX, copperY, 0);
    glBindTexture(GL_TEXTURE_2D, copperTexture);
    float copperAspect = (float)copperImgWidth / copperImgHeight;
    float cw = 0.3f, ch = cw / copperAspect;
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-cw / 2, -ch / 2);
    glTexCoord2f(1, 0); glVertex2f(cw / 2, -ch / 2);
    glTexCoord2f(1, 1); glVertex2f(cw / 2, ch / 2);
    glTexCoord2f(0, 1); glVertex2f(-cw / 2, ch / 2);
    glEnd();
    glPopMatrix();*/

    // copper + rotor
    float copperAspect = (float)copperImgWidth / copperImgHeight;
    float cw = 0.3f, ch = cw / copperAspect;

    glPushMatrix();
    glTranslatef(copperX, copperY + ch / 2 + 0.02f, 0.0f);

    // Draw copper texture
    glBindTexture(GL_TEXTURE_2D, copperTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-cw / 2, -ch / 2);
    glTexCoord2f(1, 0); glVertex2f(cw / 2, -ch / 2);
    glTexCoord2f(1, 1); glVertex2f(cw / 2, ch / 2);
    glTexCoord2f(0, 1); glVertex2f(-cw / 2, ch / 2);
    glEnd();

    glPushMatrix();
    glTranslatef(0.03f, ch / 2 -0.05, 0.0f);
    glRotatef(rotorAngle, 0, 0, 1);
    glDisable(GL_TEXTURE_2D);

    glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
    glBegin(GL_QUADS);
    // Horizontal blade
    glVertex2f(-0.15f, -0.005f);
    glVertex2f(0.15f, -0.005f);
    glVertex2f(0.15f, 0.005f);
    glVertex2f(-0.15f, 0.005f);
    // Vertical blade
    glVertex2f(-0.005f, -0.15f);
    glVertex2f(0.005f, -0.15f);
    glVertex2f(0.005f, 0.15f);
    glVertex2f(-0.005f, 0.15f);
    glEnd();

    glEnable(GL_TEXTURE_2D); // Re-enable for next draw
    glPopMatrix(); // End rotor

    glPopMatrix(); // End of copper+rotor


    glDisable(GL_TEXTURE_2D);

    // Draw flames at building positions
    for (int i = 0; i < flameCount; i++) {
        drawFlame(flames[i].x, flames[i].y);
    }

    glutSwapBuffers();
}

void update(int value) {
    flameTime += 0.1f; // Animate flicker

    if (!stop) {
        tankX += tankSpeedX;
        tankY += tankSpeedY;
        if (tankX <= 0.2f ) {
            stop = true;
        }
    }
    missileX += missileSpeedX;
    missileY += missileSpeedY;

    copperX += copperSpeedX;
    copperY += copperSpeedY;
    rotorAngle += 10.0f;
    if (rotorAngle > 360.0f)rotorAngle -= 360.0f;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    sceneTexture = loadTexture("./assets/scene3.png");
    tankTexture = loadTexture("./assets/tankangle.png", &tankImgWidth, &tankImgHeight);
    missileTexture = loadTexture("./assets/Missile.png", &missileImgWidth, &missileImgHeight);
    copperTexture = loadTexture("./assets/copperback.png", &copperImgWidth, &copperImgHeight);
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