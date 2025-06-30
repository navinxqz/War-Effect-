#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <irrKlang.h>
using namespace irrklang;
#include <cmath>
#include <math.h>
#include <Windows.h>
#include <mmsystem.h>

#pragma comment(lib,"Winmm.lib")

GLuint sceneTexture, scene2Texture, scene3Texture, tankTexture, newTankTexture, missileTexture, copperTexture, fireTexture, artilleryTexture, truckTexture;
int tankImgWidth, tankImgHeight;
int newTankImgWidth, newTankImgHeight;
int missileImgWidth, missileImgHeight;
int copperImgWidth, copperImgHeight;
int fireImgWidth, fireImgHeight;
int artilleryImgWidth, artilleryImgHeight;
int truckImgWidth, truckImgHeight;
int scene3ImgWidth, scene3ImgHeight;

int current = 1;
ISoundEngine* engine = nullptr;

// Tank variables
float tankX = 1.1f, tankY = 0.18f;
float newTankX = 1.4f, newTankY = 0.18f;
float tankSpeedX = -0.002f, tankSpeedY = -0.00105f;
bool stop = false;

// Tank fire
float fireOffsetX = -0.1f, fireOffsetY = 0.12f;
bool fireVisible = false;

// Missile variables
bool missileLaunched = false;
float missileStartX = 1.2f, missileStartY = -0.3f;
float missileTargetX = -0.2f, missileTargetY = 0.4f;

float missileX = -1.2f, missileY = 0.3f;
float missileSpeedX = 0.018f, missileSpeedY = 0.013f;
bool missileVisible = true;

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

//truck variables
float truckX = 0.9f, truckY = -0.0f;
float truckSpeedX = 0.004f, truckSpeedY = 0.003f;
//blast
bool explosionActive = false;
float explosionX = 0.0f, explosionY = 0.0f;
int explosionDuration = 5000;

const int NUM_RAINDROPS = 500;
struct Raindrop {
    float x, y;
    float speedX, speedY;
};
Raindrop raindrops[NUM_RAINDROPS];

//thunder
bool thunderActive = false;
int thunderDuration = 150;
int thunderCooldown = 9000;
int nextThunderTime = 0;

// Light cone control
float lightStartX = -0.45f;
float lightStartY = -0.21f;
float lightEndY = -0.8f;

int blinkCount = 0;           
const int maxBlinks = 3;      
int lightBlinkFast = 100;    
int lightBlinkPause = 1500;
bool lightConeVisible = true;

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
void changeFireTexture(int value);
void hideFireTexture(int value);

void initRain() {
    for (int i = 0; i < NUM_RAINDROPS; i++) {
        raindrops[i].x = ((float)rand() / RAND_MAX) * 3.0f - 1.0f;
        raindrops[i].y = ((float)rand() / RAND_MAX) * 2.4f - 1.2f;

        raindrops[i].speedY = 0.01f + ((float)rand() / RAND_MAX) * 0.015f;
        raindrops[i].speedX = -raindrops[i].speedY * 0.6f;
    }
}
void drawRain() {
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.6f, 0.6f, 1.0f, 0.4f);
    glLineWidth(1.2f);
    glBegin(GL_LINES);
    for (int i = 0; i < NUM_RAINDROPS; i++) {
        float x = raindrops[i].x;
        float y = raindrops[i].y;
        float dx = -0.015f;
        float dy = -0.04f;

        glVertex2f(x, y);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
void updateRain() {
    for (int i = 0; i < NUM_RAINDROPS; i++) {
        raindrops[i].x += raindrops[i].speedX;
        raindrops[i].y -= raindrops[i].speedY;

        if (raindrops[i].y < -1.3f || raindrops[i].x < -1.4f) {
            raindrops[i].y = 1.3f;
            raindrops[i].x = ((float)rand() / RAND_MAX) * 3.0f - 1.1f;
            raindrops[i].speedY = 0.01f + ((float)rand() / RAND_MAX) * 0.015f;
            raindrops[i].speedX = -raindrops[i].speedY * 0.5f;
        }
    }
}
void toggleLightCone(int value) {
    if (blinkCount < maxBlinks) {
        lightConeVisible = !lightConeVisible;
        blinkCount++;

        glutTimerFunc(lightBlinkFast, toggleLightCone, 0);
    }
    else {
        // After 3 fast blinks, pause for 3 seconds
        glutTimerFunc(lightBlinkPause, [](int) {
            blinkCount = 0; // Reset blink count
            lightConeVisible = true; // Ensure starts visible
            glutTimerFunc(lightBlinkFast, toggleLightCone, 0); // Restart blinking
            }, 0);
    }
}
void drawLightCone() {
    if (!lightConeVisible) return;
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 1.0f, 0.8f, 0.3f);
    glVertex2f(lightStartX, lightStartY);
    glColor4f(1.0f, 1.0f, 0.8f, 0.0f);
    glVertex2f(-0.65f, lightEndY);
    glVertex2f(-0.2f, lightEndY);
    glEnd();

    glPopAttrib();
}

void triggerCameraShake(int numFrames) {
    cameraShakeOffsetX = ((float)rand() / RAND_MAX - 0.5f) * 0.04f;
    cameraShakeOffsetY = ((float)rand() / RAND_MAX - 0.5f) * 0.02f;
    cameraShakeFramesLeft = numFrames;
}
void hideMissile(int value) {
    missileVisible = false;
    engine->play2D("assets/explosion.wav", false);
}
void startExplosion(float x, float y) {
    explosionActive = true;
    explosionX = x;
    explosionY = y;
    engine->play2D("assets/explosion.wav", false);
    triggerCameraShake(maxShakeFrames);

    // Stop explosion after 5 sec
    glutTimerFunc(explosionDuration, [](int) {
        explosionActive = false;
        glutPostRedisplay();
        }, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    if (current == 1) {
        glBindTexture(GL_TEXTURE_2D, sceneTexture);
    }
    else if (current == 2) {
        glBindTexture(GL_TEXTURE_2D, scene2Texture);
    }
    else if (current == 3) {
        glBindTexture(GL_TEXTURE_2D, scene3Texture);
    }
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
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
        drawLightCone();
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
    if (current == 1 || current==3) {
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

        // Draw jeep
        glPushMatrix();
        glTranslatef(newTankX, newTankY, 0.0f);
        glBindTexture(GL_TEXTURE_2D, newTankTexture);
        float newTankAspect = (float)newTankImgWidth / (float)newTankImgHeight;
        float newTankDisplayWidth = 0.22f;
        float newTankDisplayHeight = newTankDisplayWidth / newTankAspect;

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-newTankDisplayWidth / 2, -newTankDisplayHeight / 2);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(newTankDisplayWidth / 2, -newTankDisplayHeight / 2);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(newTankDisplayWidth / 2, newTankDisplayHeight / 2);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-newTankDisplayWidth / 2, newTankDisplayHeight / 2);
        glEnd();
        glPopMatrix();
    }
    if (current == 2) {
        glPushMatrix();
        glTranslatef(truckX, truckY, 0.0f);
        glBindTexture(GL_TEXTURE_2D, truckTexture);
        float truckAspect = (float)truckImgWidth / (float)truckImgHeight;
        float truckDisplayWidth = 0.3f;
        float truckDisplayHeight = truckDisplayWidth / truckAspect;

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-truckDisplayWidth / 2, -truckDisplayHeight / 2);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(truckDisplayWidth / 2, -truckDisplayHeight / 2);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(truckDisplayWidth / 2, truckDisplayHeight / 2);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-truckDisplayWidth / 2, truckDisplayHeight / 2);
        glEnd();
        glPopMatrix();

        if (thunderActive) {
            glDisable(GL_TEXTURE_2D);
            glColor4f(1.0f, 1.0f, 1.0f, 0.5f); // bright white flash
            glBegin(GL_QUADS);
            glVertex2f(-1.0f, -1.0f);
            glVertex2f(1.0f, -1.0f);
            glVertex2f(1.0f, 1.0f);
            glVertex2f(-1.0f, 1.0f);
            glEnd();
            glEnable(GL_TEXTURE_2D);
        }

    }
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
    if (missileVisible) { 
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
    }
    // Copper + rotor
    if (current == 1 || current==3) {
        float copperAspect = (float)copperImgWidth / copperImgHeight;
        float cw = 0.3f, ch = cw / copperAspect;

        glPushMatrix();
        glTranslatef(copperX, copperY + ch / 2 + 0.02f, 0.0f);

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

        glEnable(GL_TEXTURE_2D);
        glPopMatrix(); // End rotor
        glPopMatrix(); // End of copper+rotor
        glDisable(GL_TEXTURE_2D);
    }
    glutSwapBuffers();
    glPopMatrix();
}
void changeFireTexture(int value);
void hideFireTexture(int value);

bool fireSequenceActive = false;
bool copperSoundPlaying = false;

//sound part
void playTankFireSound() {
    //PlaySound(TEXT("./assets/tankfire7.wav"), NULL, SND_FILENAME | SND_ASYNC);
    engine->play2D("assets/tankfire7.wav", false);
}
void playSiren() {
    engine->play2D("assets/siren.wav", true);
}
void playArtilleryFireSound() {
    engine->play2D("assets/tankfire1.wav", false);
}
void copperSound() {
    engine->play2D("assets/copper.wav", false);
}
void playRainSound() {
    //PlaySound(TEXT("./assets/rain.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    engine->play2D("assets/rain.wav", true);
}
void playTruckSound() {
    engine->play2D("assets/truck.wav", true);
}
void playWarSound() {
    //engine->play2D("assets/Warfare.wav", true);
}
void update(int value) {
    if (current == 2) {
        updateRain();
        truckX += truckSpeedX;
        truckY += truckSpeedY;

        if (truckX > 1.2f) {
            truckX = -1.2f;
            truckY = -1.4f;
        }
        // Thunder logic
        int currentTime = glutGet(GLUT_ELAPSED_TIME);
        if (!thunderActive && currentTime >= nextThunderTime) {
            thunderActive = true;
            triggerCameraShake(maxShakeFrames); // optional shake effect
            engine->play2D("assets/thunder.wav", false); // make sure thunder.wav exists

            // Turn off thunder after some time
            glutTimerFunc(thunderDuration, [](int) {
                thunderActive = false;
                glutPostRedisplay();
                }, 0);

            // Set next thunder after random cooldown
            nextThunderTime = currentTime + thunderCooldown + rand() % 2000; // some randomness
        }

    }
    if (current == 1 || current==3) {
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
        else if (fireSequenceActive) {
            glutTimerFunc(1000, hideFireTexture, 0);
            fireSequenceActive = false;
        }
    }
    if (missileLaunched) {
        float dx = missileTargetX - missileX;
        float dy = missileTargetY - missileY;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist > 0.01f) {
            missileX += (dx / dist) * missileSpeedX;
            missileY += (dy / dist) * missileSpeedY;
        }
        else {
            missileLaunched = false;
            startExplosion(missileTargetX, missileTargetY);
            current = 3;
            glutTimerFunc(700, hideMissile, 0);
        }
    }
    if (current == 1 || current==3) {
        copperX += copperSpeedX;
        copperY += copperSpeedY;
        if (copperX > -1.0f && copperX < 0.45f) {
            if (!copperSoundPlaying) {
                copperSound();
                copperSoundPlaying = true;
            }
        }
        else {
            if (copperSoundPlaying) {
                PlaySound(NULL, NULL, 0);
                copperSoundPlaying = false;
            }
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
        //PlaySound(NULL, NULL, 0);
        missileVisible = true;
        missileX = missileStartX;
        missileY = missileStartY;
        missileLaunched = false;

        engine->stopAllSounds();
        playSiren();
        playWarSound();
    }
    else if (key == '2') {
        current = 2;
        missileVisible = true;
        missileX = missileStartX;
        missileY = missileStartY;
        missileLaunched = false;

        engine->stopAllSounds();
        initRain();
        playRainSound();
        playTruckSound();
    }
    if (current == 1 || current == 3) {
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
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && current == 1) {
        missileLaunched = true;
        missileX = missileStartX;
        missileY = missileStartY;

        float dx = missileTargetX - missileX;
        float dy = missileTargetY - missileY;
        float length = sqrt(dx * dx + dy * dy);

        engine->play2D("assets/tankfire1.wav", false);
        triggerCameraShake(maxShakeFrames);
    }
}

void init() {
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    engine = createIrrKlangDevice();
    if (!engine) {
        printf("Failed to create sound engine.\n");
    }
    glutTimerFunc(lightBlinkFast, toggleLightCone, 0);

    sceneTexture = loadTexture("./assets/sceneup.png");
    scene2Texture = loadTexture("./assets/rain.jpeg");
    scene3Texture = loadTexture("./assets/scenenew.jpeg", &scene3ImgWidth, &scene3ImgHeight);
    tankTexture = loadTexture("./assets/tankangle.png", &tankImgWidth, &tankImgHeight);
    missileTexture = loadTexture("./assets/Missile.png", &missileImgWidth, &missileImgHeight);
    copperTexture = loadTexture("./assets/copperback.png", &copperImgWidth, &copperImgHeight);
    fireTexture = loadTexture("./assets/TankFire1.png", &fireImgWidth, &fireImgHeight);
    artilleryTexture = loadTexture("./assets/artillery-gun.png", &artilleryImgWidth, &artilleryImgHeight);
    newTankTexture = loadTexture("./assets/jeep.png", &newTankImgWidth, &newTankImgHeight);
    truckTexture = loadTexture("./assets/truck.png", &truckImgWidth, &truckImgHeight);

    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("WarZone");

    init();
    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}