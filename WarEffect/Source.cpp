#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cmath>
#include <math.h>
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib,"Winmm.lib")

GLuint sceneTexture, tankTexture, missileTexture, copperTexture, fireTexture; // Added fireTexture
int tankImgWidth, tankImgHeight;
int missileImgWidth, missileImgHeight;
int copperImgWidth, copperImgHeight;
int fireImgWidth, fireImgHeight; // Added fire image dimensions

// Tank variables
float tankX = 1.2f, tankY = 0.18f;
float tankSpeedX = -0.002f, tankSpeedY = -0.00105f;
bool stop = false;

// Tank fire
float fireOffsetX = -0.1f;
float fireOffsetY = 0.12f;
bool fireVisible = false; // Track fire visibility

// Missile variables
float missileX = -1.2f, missileY = 0.3f;
float missileSpeedX = 0.009, missileSpeedY = 0.0065f;

// Copper variables
float copperX = -0.9f, copperY = -0.1f;
float copperSpeedX = 0.004, copperSpeedY = 0.0035f;
float rotorAngle = 0.0f;

// Flame variables
float flameTime = 0.0f; // Declare flameTime

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

// Forward declaration of the function to change fire texture
void changeFireTexture(int value);

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

    // Check if fire should be visible
    if (fireVisible) {
        glPushMatrix();
        // Translate fire texture position with offsets applied
        glTranslatef(tankX + fireOffsetX, tankY + fireOffsetY, 0.0f);
        // Bind the fire texture
        glBindTexture(GL_TEXTURE_2D, fireTexture);

        // Calculate fire texture aspect ratio and size (adjust fireWidth as needed)
        float fireAspect = (float)fireImgWidth / (float)fireImgHeight;
        float fireWidth = 0.4f;  // Example width
        float fireHeight = fireWidth / fireAspect;

        // Draw a textured quad for the fire texture
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-fireWidth / 2, -fireHeight / 2);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(fireWidth / 2, -fireHeight / 2);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(fireWidth / 2, fireHeight / 2);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-fireWidth / 2, fireHeight / 2);
        glEnd();
        glPopMatrix();
    }

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

    // Missile
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

    // Copper + rotor
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
    glTranslatef(0.03f, ch / 2 - 0.05, 0.0f);
    glRotatef(rotorAngle, 0.0f, 0.0f, 1.0f);
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
        //drawFlame(flames[i].x, flames[i].y);
    }
    glutSwapBuffers();
}
void hideFireTexture(int value);
// Add a flag to track fire sequence state
bool fireSequenceActive = false;

void update(int value) {
    flameTime += 0.1f; // Animate flicker

    if (!stop) {
        tankX += tankSpeedX;
        tankY += tankSpeedY;
        if (tankX <= 0.2f) {
            stop = true;
            // Show initial fire immediately
            fireVisible = true;
            if (fireTexture != 0) {
                glDeleteTextures(1, &fireTexture);
            }
            fireTexture = loadTexture("./assets/TankFire1.png", &fireImgWidth, &fireImgHeight);
            // Schedule texture change after 3 seconds to TankFire2.png
            glutTimerFunc(1000, changeFireTexture, 0);
            fireSequenceActive = true; // Mark fire sequence as active
        }
    }
    else if (fireSequenceActive) {
        // After 5 seconds, hide the fire texture
        glutTimerFunc(2500, hideFireTexture, 0); // Hide fire texture after 2500ms
        fireSequenceActive = false; // Reset the fire sequence state
    }

    missileX += missileSpeedX;
    missileY += missileSpeedY;

    copperX += copperSpeedX;
    copperY += copperSpeedY;
    rotorAngle += 10.0f;
    if (rotorAngle > 360.0f) rotorAngle -= 360.0f;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// Function to change the fire texture after delay
void changeFireTexture(int value) {
    // Delete old texture first (optional cleanup)
    if (fireTexture != 0) {
        glDeleteTextures(1, &fireTexture);
    }
    // Load new fire texture image (adjust path accordingly)
    fireTexture = loadTexture("./assets/TankFire2.png", &fireImgWidth, &fireImgHeight); // Load new fire texture
    glutPostRedisplay();
}

// Function to hide the fire texture after 5 seconds
void hideFireTexture(int value) {
    fireVisible = false; // Hide fire texture
    glutPostRedisplay();
}
void playTankFireSound() {
    PlaySound(TEXT("./assets/tankfire7.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

// Keyboard callback to listen for 'F' key press
void keyboard(unsigned char key, int x, int y) {
    if (key == 'F' || key == 'f') {
        // Show TankFire1.png immediately
        fireVisible = true;
        if (fireTexture != 0) {
            glDeleteTextures(1, &fireTexture);
        }
        fireTexture = loadTexture("./assets/TankFire1.png", &fireImgWidth, &fireImgHeight);
        playTankFireSound();
        // Schedule TankFire2.png after 1000ms
        glutTimerFunc(1000, changeFireTexture, 0);

        // Schedule hiding fire after 2500ms (1000 + 2500)
        glutTimerFunc(2000, hideFireTexture, 0);

        glutPostRedisplay();
    }
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
    fireTexture = loadTexture("./assets/TankFire1.png", &fireImgWidth, &fireImgHeight); // Load initial fire texture

    glutKeyboardFunc(keyboard);

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
