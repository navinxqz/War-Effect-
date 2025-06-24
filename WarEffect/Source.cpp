#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cmath>
#include <math.h>
#include <Windows.h>
#include <mmsystem.h>

#pragma comment(lib,"Winmm.lib")

GLuint sceneTexture, scene2Texture, tankTexture, newTankTexture, missileTexture, copperTexture, fireTexture, artilleryTexture;
int tankImgWidth, tankImgHeight;
int newTankImgWidth, newTankImgHeight;
int missileImgWidth, missileImgHeight;
int copperImgWidth, copperImgHeight;
int fireImgWidth, fireImgHeight;
int artilleryImgWidth, artilleryImgHeight;

int current = 1;

// Tank variables
float tankX = 1.2f, tankY = 0.18f;
float newTankX = 1.5f, newTankY = 0.18f;
float tankSpeedX = -0.002f, tankSpeedY = -0.00105f;
bool stop = false;

// Tank fire
float fireOffsetX = -0.1f, fireOffsetY = 0.12f;
bool fireVisible = false;

// Missile variables
float missileX = -1.2f, missileY = 0.3f;
float missileSpeedX = 0.009, missileSpeedY = 0.0065f;

// Copper variables
float copperX = -0.9f, copperY = -0.1f;
float copperSpeedX = 0.004, copperSpeedY = 0.0035f;
float rotorAngle = 0.0f;

// Artillery gun variables
float artilleryX = -0.0f, artilleryY = -0.8f;

// Artillery fire
bool artilleryFireVisible = false;
float artilleryFireOffsetX = -0.15f, artilleryFireOffsetY = 0.15f;

// Camera shake variables
float cameraShakeOffsetX = 0.0f;
float cameraShakeOffsetY = 0.0f;
int cameraShakeFramesLeft = 0;
const int maxShakeFrames = 6;


// Flame variables
float flameTime = 0.0f;

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

const int NUM_RAINDROPS = 200;
struct Raindrop {
    float x, y;
    float speedY;
};
Raindrop raindrops[NUM_RAINDROPS];

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
    //glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    GLint internalFormat = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return textureID;
}

// Forward declaration of the function to change fire texture
void changeFireTexture(int value);
void hideFireTexture(int value);

void initRain() {
    for (int i = 0; i < NUM_RAINDROPS; i++) {
        raindrops[i].x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;    // X in [-1, 1]
        raindrops[i].y = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;    // Y in [-1, 1]
        raindrops[i].speedY = 0.01f + ((float)rand() / RAND_MAX) * 0.02f;
    }
}
void drawRain() {
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.6f, 0.6f, 1.0f, 0.5f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    for (int i = 0; i < NUM_RAINDROPS; i++) {
        float x = raindrops[i].x;
        float y = raindrops[i].y;
        glVertex2f(x, y);
        glVertex2f(x, y - 0.05f);    // Raindrop length
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
void updateRain() {
    for (int i = 0; i < NUM_RAINDROPS; i++) {
        raindrops[i].y -= raindrops[i].speedY;
        if (raindrops[i].y < -1.0f) {
            raindrops[i].y = 1.0f;   // reset to top
            raindrops[i].x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;  // random x again
        }
    }
}
void triggerCameraShake(int numFrames) {
    cameraShakeOffsetX = ((float)rand() / RAND_MAX - 0.5f) * 0.04f; // Random between -0.04 and 0.04
    cameraShakeOffsetY = ((float)rand() / RAND_MAX - 0.5f) * 0.02f;
    cameraShakeFramesLeft = numFrames;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);

    if (current == 1) {
        glBindTexture(GL_TEXTURE_2D, sceneTexture);
    }
    else if (current == 2) {
        glBindTexture(GL_TEXTURE_2D, scene2Texture);
    }glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glPushMatrix();
    glTranslatef(cameraShakeOffsetX, cameraShakeOffsetY, 0.0f);

    // Draw background
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
    glEnd();

    if (current == 2) {
        drawRain();
    }

    // Check fire visible
    if (fireVisible) {
        glPushMatrix();
        glTranslatef(tankX + fireOffsetX, tankY + fireOffsetY, 0.0f);
        glBindTexture(GL_TEXTURE_2D, fireTexture);

        float fireAspect = (float)fireImgWidth / (float)fireImgHeight;
        float fireWidth = 0.4f;
        float fireHeight = fireWidth / fireAspect;

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

    // Draw new tank
    glPushMatrix();
    glTranslatef(newTankX, newTankY, 0.0f);
    glBindTexture(GL_TEXTURE_2D, newTankTexture);
    float newTankAspect = (float)newTankImgWidth / (float)newTankImgHeight;
    float newTankDisplayWidth = 0.2f;
    float newTankDisplayHeight = newTankDisplayWidth / newTankAspect;

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-newTankDisplayWidth / 2, -newTankDisplayHeight / 2);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(newTankDisplayWidth / 2, -newTankDisplayHeight / 2);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(newTankDisplayWidth / 2, newTankDisplayHeight / 2);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-newTankDisplayWidth / 2, newTankDisplayHeight / 2);
    glEnd();
    glPopMatrix();

    // Draw artillery gun
    glPushMatrix();
    glTranslatef(artilleryX, artilleryY, 0.0f);
    glBindTexture(GL_TEXTURE_2D, artilleryTexture);
    float artAspect = (float)artilleryImgWidth / artilleryImgHeight;
    float artWidth = 0.3f;
    float artHeight = artWidth / artAspect;
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-artWidth / 2, -artHeight / 2);
    glTexCoord2f(1, 0); glVertex2f(artWidth / 2, -artHeight / 2);
    glTexCoord2f(1, 1); glVertex2f(artWidth / 2, artHeight / 2);
    glTexCoord2f(0, 1); glVertex2f(-artWidth / 2, artHeight / 2);
    glEnd();
    glPopMatrix();

    if (artilleryFireVisible) {
        glPushMatrix();
        glTranslatef(artilleryX + artilleryFireOffsetX, artilleryY + artilleryFireOffsetY, 0.0f);
        glBindTexture(GL_TEXTURE_2D, fireTexture); // Reuse the same fireTexture
        float fireAspect = (float)fireImgWidth / (float)fireImgHeight;
        float fireWidth = 0.4f;
        float fireHeight = fireWidth / fireAspect;
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-fireWidth / 2, -fireHeight / 2);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(fireWidth / 2, -fireHeight / 2);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(fireWidth / 2, fireHeight / 2);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-fireWidth / 2, fireHeight / 2);
        glEnd();
        glPopMatrix();
    }

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
    glPopMatrix();
}
void changeFireTexture(int value);
void hideFireTexture(int value);

//bool fireVisible = false;
//float flameTime = 0.0f;
bool fireSequenceActive = false;
bool copperSoundPlaying = false;

//sound part
void playTankFireSound() {
    PlaySound(TEXT("./assets/tankfire7.wav"), NULL, SND_FILENAME | SND_ASYNC);
}
void playArtilleryFireSound() {
    PlaySound(TEXT("./assets/tankfire1.wav"), NULL, SND_FILENAME | SND_ASYNC);
}
void copperSound() {
    PlaySound(TEXT("./assets/copper.wav"), NULL, SND_FILENAME | SND_ASYNC);
}
void playRainSound() {
    PlaySound(TEXT("./assets/rain.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

void update(int value) {
    if (current == 2) {
        updateRain();
    }
    flameTime += 0.1f;
    newTankX += tankSpeedX;
    newTankY += tankSpeedY;

    if (!stop) {
        tankX += tankSpeedX;
        tankY += tankSpeedY;

        if (tankX <= 0.2f) {
            stop = true;

            fireVisible = true;
            if (fireTexture != 0) {
                glDeleteTextures(1, &fireTexture);
            }
            fireTexture = loadTexture("./assets/TankFire1.png", &fireImgWidth, &fireImgHeight);
            glutTimerFunc(500, changeFireTexture, 0);
            fireSequenceActive = true;
        }
    }
    else if (fireSequenceActive){
        glutTimerFunc(1000, hideFireTexture, 0); 
        fireSequenceActive = false;
    }
    missileX += missileSpeedX;
    missileY += missileSpeedY;

    copperX += copperSpeedX;
    copperY += copperSpeedY;

    if (copperX > -1.0f && copperX < 0.45f){
        if (!copperSoundPlaying) {
            copperSound();
            copperSoundPlaying = true;
        }
    }else {
        if (copperSoundPlaying) {
            PlaySound(NULL, NULL, 0);
            copperSoundPlaying = false;
        }
    }

    if (copperX > 1.0f) {
        copperX = -1.0f;
        copperY = -0.4f;
    }
    rotorAngle += 10.0f;
    if (rotorAngle > 360.0f) rotorAngle -= 360.0f;

    if (cameraShakeFramesLeft > 0) {
        cameraShakeFramesLeft--;
        if (cameraShakeFramesLeft == 0) {
            cameraShakeOffsetX = 0.0f;
            cameraShakeOffsetY = 0.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void changeFireTexture(int value) {
    if (fireTexture != 0) {
        glDeleteTextures(1, &fireTexture);
    }
    fireTexture = loadTexture("./assets/TankFire2.png", &fireImgWidth, &fireImgHeight);
    glutPostRedisplay();
}
void hideFireTexture(int value) {
    fireVisible = false;
    glutPostRedisplay();
}
//keyboard part
void keyboard(unsigned char key, int x, int y) {
    if (key == '1') {
        current = 1;
        PlaySound(NULL, NULL, 0);
    }
    else if (key == '2') {
        current = 2;
        initRain();
        playRainSound();
    }
    if (current == 1) {
        if (key == 'F' || key == 'f') {
            fireVisible = true;
            if (fireTexture != 0) { glDeleteTextures(1, &fireTexture); }
            fireTexture = loadTexture("./assets/TankFire1.png", &fireImgWidth, &fireImgHeight);
            playTankFireSound();
            triggerCameraShake(maxShakeFrames); 

            glutTimerFunc(1000, changeFireTexture, 0);
            glutTimerFunc(2000, hideFireTexture, 0);
            glutPostRedisplay();
        }
        else if (key == 'G' || key == 'g') {
            artilleryFireVisible = true;
            fireTexture = loadTexture("./assets/TankFire1.png", &fireImgWidth, &fireImgHeight);
            playArtilleryFireSound();
            triggerCameraShake(maxShakeFrames);

            glutTimerFunc(1000, [](int) { artilleryFireVisible = false; glutPostRedisplay(); }, 0);
            glutPostRedisplay();
        }
    }
}
void init() {
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    sceneTexture = loadTexture("./assets/scene3.png");
    scene2Texture = loadTexture("./assets/rainy.png");
    tankTexture = loadTexture("./assets/tankangle.png", &tankImgWidth, &tankImgHeight);
    missileTexture = loadTexture("./assets/Missile.png", &missileImgWidth, &missileImgHeight);
    copperTexture = loadTexture("./assets/copperback.png", &copperImgWidth, &copperImgHeight);
    fireTexture = loadTexture("./assets/TankFire1.png", &fireImgWidth, &fireImgHeight);
    artilleryTexture = loadTexture("./assets/artillery-gun.png", &artilleryImgWidth, &artilleryImgHeight);
    newTankTexture = loadTexture("./assets/tank.png", &newTankImgWidth, &newTankImgHeight); // Load new tank texture

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
