/**
 * 3D Maze Game for PSP - OpenGL ES Version
 *
 * A first-person maze exploration game using hardware-accelerated OpenGL.
 * Features procedurally generated mazes, textured walls, and 3 levels.
 *
 * Created by Claude Code (Anthropic)
 */

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

/* Module info provided by SDL2 */

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define TEX_SIZE 64
#define WALL_HEIGHT 1.0f
#define PLAYER_HEIGHT 0.5f
#define PLAYER_RADIUS 0.25f
#define MOVE_SPEED 0.08f
#define ROT_SPEED 0.04f

/* Game States */
typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_PAUSE,
    STATE_LEVEL_COMPLETE,
    STATE_WIN,
    STATE_QUIT
} GameState;

/* Player structure */
typedef struct {
    float x, y;
    float angle;
} Player;

/* Level configuration */
typedef struct {
    int mazeWidth;
    int mazeHeight;
} LevelConfig;

/* Wall vertex for rendering */
typedef struct {
    float x1, z1, x2, z2;
    int isExit;
} Wall;

/* Global state */
static GameState gState = STATE_MENU;
static Player gPlayer;
static int *gWallGrid = NULL;
static int gGridWidth = 0;
static int gGridHeight = 0;
static int gCurrentLevel = 0;
static int gMenuSelection = 0;
static int gPauseSelection = 0;

static Wall *gWalls = NULL;
static int gWallCount = 0;

static GLuint gBrickTexture = 0;
static GLuint gExitTexture = 0;
static GLuint gFloorTexture = 0;
static GLuint gCeilingTexture = 0;

static Mix_Music *gMusic = NULL;
static Mix_Chunk *gWinSound = NULL;
static Mix_Chunk *gSelectSound = NULL;

static SceCtrlData gPad;
static int gButtonPressed = 0;

/* FPS counter */
static Uint32 gLastTime = 0;
static int gFrameCount = 0;
static int gFPS = 0;

static LevelConfig gLevels[3] = {
    {5, 5},
    {8, 8},
    {12, 10}
};

/* ============== Texture Generation ============== */

static unsigned int *generateBrickTextureData(void)
{
    unsigned int *data = malloc(TEX_SIZE * TEX_SIZE * sizeof(unsigned int));
    if (!data) return NULL;

    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int brickH = 16;
            int brickW = 32;
            int row = y / brickH;
            int offset = (row % 2) * (brickW / 2);

            int isMortarH = (y % brickH) < 2;
            int isMortarV = ((x + offset) % brickW) < 2 && !isMortarH;

            if (isMortarH || isMortarV) {
                data[y * TEX_SIZE + x] = 0xFF505050;
            } else {
                int variation = (rand() % 40) - 20;
                unsigned char r = (unsigned char)(140 + variation);
                unsigned char g = (unsigned char)(70 + variation / 2);
                unsigned char b = (unsigned char)(40 + variation / 3);
                data[y * TEX_SIZE + x] = 0xFF000000 | (b << 16) | (g << 8) | r;
            }
        }
    }
    return data;
}

static unsigned int *generateExitTextureData(void)
{
    unsigned int *data = malloc(TEX_SIZE * TEX_SIZE * sizeof(unsigned int));
    if (!data) return NULL;

    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int checker = ((x / 8) + (y / 8)) % 2;
            if (checker) {
                data[y * TEX_SIZE + x] = 0xFF00FF00;
            } else {
                data[y * TEX_SIZE + x] = 0xFF008800;
            }
        }
    }
    return data;
}

static unsigned int *generateFloorTextureData(void)
{
    unsigned int *data = malloc(TEX_SIZE * TEX_SIZE * sizeof(unsigned int));
    if (!data) return NULL;

    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int checker = ((x / 16) + (y / 16)) % 2;
            int variation = (rand() % 20) - 10;
            unsigned char base = checker ? 60 : 50;
            unsigned char c = (unsigned char)(base + variation);
            data[y * TEX_SIZE + x] = 0xFF000000 | (c << 16) | (c << 8) | c;
        }
    }
    return data;
}

static unsigned int *generateCeilingTextureData(void)
{
    unsigned int *data = malloc(TEX_SIZE * TEX_SIZE * sizeof(unsigned int));
    if (!data) return NULL;

    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int variation = (rand() % 15) - 7;
            unsigned char c = (unsigned char)(40 + variation);
            data[y * TEX_SIZE + x] = 0xFF000000 | ((c+20) << 16) | (c << 8) | c;
        }
    }
    return data;
}

static GLuint createTexture(unsigned int *data)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return tex;
}

static void initTextures(void)
{
    unsigned int *data;

    data = generateBrickTextureData();
    gBrickTexture = createTexture(data);
    free(data);

    data = generateExitTextureData();
    gExitTexture = createTexture(data);
    free(data);

    data = generateFloorTextureData();
    gFloorTexture = createTexture(data);
    free(data);

    data = generateCeilingTextureData();
    gCeilingTexture = createTexture(data);
    free(data);
}

/* ============== Audio ============== */

static void saveWavFile(const char *filename, short *buffer, int samples, int sampleRate)
{
    SDL_RWops *file = SDL_RWFromFile(filename, "wb");
    if (!file) return;

    unsigned int dataSize = samples * 2;
    unsigned int chunkSize = 36 + dataSize;

    SDL_RWwrite(file, "RIFF", 4, 1);
    SDL_RWwrite(file, &chunkSize, 4, 1);
    SDL_RWwrite(file, "WAVE", 4, 1);
    SDL_RWwrite(file, "fmt ", 4, 1);

    unsigned int fmtSize = 16;
    unsigned short audioFormat = 1;
    unsigned short numChannels = 1;
    unsigned int byteRate = sampleRate * 2;
    unsigned short blockAlign = 2;
    unsigned short bitsPerSample = 16;

    SDL_RWwrite(file, &fmtSize, 4, 1);
    SDL_RWwrite(file, &audioFormat, 2, 1);
    SDL_RWwrite(file, &numChannels, 2, 1);
    SDL_RWwrite(file, &sampleRate, 4, 1);
    SDL_RWwrite(file, &byteRate, 4, 1);
    SDL_RWwrite(file, &blockAlign, 2, 1);
    SDL_RWwrite(file, &bitsPerSample, 2, 1);

    SDL_RWwrite(file, "data", 4, 1);
    SDL_RWwrite(file, &dataSize, 4, 1);
    SDL_RWwrite(file, buffer, dataSize, 1);

    SDL_RWclose(file);
}

static void generateAudioFiles(void)
{
    int sampleRate = 22050;

    /* Background music - simple melody */
    int musicSamples = sampleRate * 4;
    short *musicBuffer = malloc(musicSamples * sizeof(short));
    int notes[] = {262, 294, 330, 349, 392, 349, 330, 294};
    int samplesPerNote = musicSamples / 8;
    for (int n = 0; n < 8; n++) {
        for (int i = 0; i < samplesPerNote; i++) {
            int idx = n * samplesPerNote + i;
            double t = (double)i / sampleRate;
            double envelope = 1.0;
            int fadeLen = sampleRate / 20;
            if (i < fadeLen) envelope = (double)i / fadeLen;
            else if (i > samplesPerNote - fadeLen) envelope = (double)(samplesPerNote - i) / fadeLen;
            musicBuffer[idx] = (short)(15000 * envelope * sin(2.0 * M_PI * notes[n] * t));
        }
    }
    saveWavFile("bgmusic.wav", musicBuffer, musicSamples, sampleRate);
    free(musicBuffer);

    /* Select sound */
    int selectSamples = sampleRate / 10;
    short *selectBuffer = malloc(selectSamples * sizeof(short));
    for (int i = 0; i < selectSamples; i++) {
        double t = (double)i / sampleRate;
        double envelope = 1.0 - (double)i / selectSamples;
        selectBuffer[i] = (short)(20000 * envelope * sin(2.0 * M_PI * 440 * t));
    }
    saveWavFile("select.wav", selectBuffer, selectSamples, sampleRate);
    free(selectBuffer);

    /* Win sound */
    int winSamples = sampleRate;
    short *winBuffer = malloc(winSamples * sizeof(short));
    int winNotes[] = {523, 659, 784, 1047};
    int winSamplesPerNote = winSamples / 4;
    for (int n = 0; n < 4; n++) {
        for (int i = 0; i < winSamplesPerNote; i++) {
            int idx = n * winSamplesPerNote + i;
            double t = (double)i / sampleRate;
            double envelope = 1.0;
            if (i > winSamplesPerNote - sampleRate/30)
                envelope = (double)(winSamplesPerNote - i) / (sampleRate/30);
            winBuffer[idx] = (short)(20000 * envelope * sin(2.0 * M_PI * winNotes[n] * t));
        }
    }
    saveWavFile("win.wav", winBuffer, winSamples, sampleRate);
    free(winBuffer);
}

/* ============== Maze Generation ============== */

#define WALL_N 1
#define WALL_E 2
#define WALL_S 4
#define WALL_W 8

typedef struct {
    int walls;
    int visited;
} MazeCell;

static void generateMaze(int width, int height)
{
    MazeCell *cells = malloc(width * height * sizeof(MazeCell));
    if (!cells) return;

    for (int i = 0; i < width * height; i++) {
        cells[i].walls = WALL_N | WALL_E | WALL_S | WALL_W;
        cells[i].visited = 0;
    }

    typedef struct { int x, y; } Pos;
    Pos *stack = malloc(width * height * sizeof(Pos));
    int stackTop = 0;

    int cx = 0, cy = 0;
    cells[0].visited = 1;

    int visited = 1;
    int total = width * height;

    while (visited < total) {
        Pos neighbors[4];
        int dirs[4];
        int count = 0;

        if (cy > 0 && !cells[(cy - 1) * width + cx].visited) {
            neighbors[count] = (Pos){cx, cy - 1};
            dirs[count++] = 0;
        }
        if (cx < width - 1 && !cells[cy * width + cx + 1].visited) {
            neighbors[count] = (Pos){cx + 1, cy};
            dirs[count++] = 1;
        }
        if (cy < height - 1 && !cells[(cy + 1) * width + cx].visited) {
            neighbors[count] = (Pos){cx, cy + 1};
            dirs[count++] = 2;
        }
        if (cx > 0 && !cells[cy * width + cx - 1].visited) {
            neighbors[count] = (Pos){cx - 1, cy};
            dirs[count++] = 3;
        }

        if (count > 0) {
            int choice = rand() % count;
            Pos next = neighbors[choice];
            int dir = dirs[choice];

            switch (dir) {
                case 0:
                    cells[cy * width + cx].walls &= ~WALL_N;
                    cells[next.y * width + next.x].walls &= ~WALL_S;
                    break;
                case 1:
                    cells[cy * width + cx].walls &= ~WALL_E;
                    cells[next.y * width + next.x].walls &= ~WALL_W;
                    break;
                case 2:
                    cells[cy * width + cx].walls &= ~WALL_S;
                    cells[next.y * width + next.x].walls &= ~WALL_N;
                    break;
                case 3:
                    cells[cy * width + cx].walls &= ~WALL_W;
                    cells[next.y * width + next.x].walls &= ~WALL_E;
                    break;
            }

            stack[stackTop++] = (Pos){cx, cy};
            cx = next.x;
            cy = next.y;
            cells[cy * width + cx].visited = 1;
            visited++;
        } else {
            stackTop--;
            cx = stack[stackTop].x;
            cy = stack[stackTop].y;
        }
    }

    /* Convert to wall grid */
    gGridWidth = width * 2 + 1;
    gGridHeight = height * 2 + 1;

    if (gWallGrid) free(gWallGrid);
    gWallGrid = malloc(gGridWidth * gGridHeight * sizeof(int));

    for (int i = 0; i < gGridWidth * gGridHeight; i++) {
        gWallGrid[i] = 1;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int gx = x * 2 + 1;
            int gy = y * 2 + 1;

            /* Mark exit cell */
            if (x == width - 1 && y == height - 1) {
                gWallGrid[gy * gGridWidth + gx] = 2;
            } else {
                gWallGrid[gy * gGridWidth + gx] = 0;
            }

            MazeCell *cell = &cells[y * width + x];
            if (!(cell->walls & WALL_E) && x < width - 1) {
                gWallGrid[gy * gGridWidth + gx + 1] = 0;
            }
            if (!(cell->walls & WALL_S) && y < height - 1) {
                gWallGrid[(gy + 1) * gGridWidth + gx] = 0;
            }
        }
    }

    free(cells);
    free(stack);

    /* Build wall list for rendering */
    if (gWalls) free(gWalls);
    gWalls = malloc(gGridWidth * gGridHeight * 4 * sizeof(Wall));
    gWallCount = 0;

    for (int y = 0; y < gGridHeight; y++) {
        for (int x = 0; x < gGridWidth; x++) {
            int cell = gWallGrid[y * gGridWidth + x];
            if (cell == 1) {
                /* Check each side for visible walls */
                float fx = (float)x;
                float fy = (float)y;

                /* North face */
                if (y > 0 && gWallGrid[(y-1) * gGridWidth + x] != 1) {
                    gWalls[gWallCount].x1 = fx;
                    gWalls[gWallCount].z1 = fy;
                    gWalls[gWallCount].x2 = fx + 1;
                    gWalls[gWallCount].z2 = fy;
                    gWalls[gWallCount].isExit = 0;
                    gWallCount++;
                }
                /* South face */
                if (y < gGridHeight - 1 && gWallGrid[(y+1) * gGridWidth + x] != 1) {
                    gWalls[gWallCount].x1 = fx + 1;
                    gWalls[gWallCount].z1 = fy + 1;
                    gWalls[gWallCount].x2 = fx;
                    gWalls[gWallCount].z2 = fy + 1;
                    gWalls[gWallCount].isExit = 0;
                    gWallCount++;
                }
                /* West face */
                if (x > 0 && gWallGrid[y * gGridWidth + x - 1] != 1) {
                    gWalls[gWallCount].x1 = fx;
                    gWalls[gWallCount].z1 = fy + 1;
                    gWalls[gWallCount].x2 = fx;
                    gWalls[gWallCount].z2 = fy;
                    gWalls[gWallCount].isExit = 0;
                    gWallCount++;
                }
                /* East face */
                if (x < gGridWidth - 1 && gWallGrid[y * gGridWidth + x + 1] != 1) {
                    gWalls[gWallCount].x1 = fx + 1;
                    gWalls[gWallCount].z1 = fy;
                    gWalls[gWallCount].x2 = fx + 1;
                    gWalls[gWallCount].z2 = fy + 1;
                    gWalls[gWallCount].isExit = 0;
                    gWallCount++;
                }
            }
            /* Exit cell walls are green */
            else if (cell == 2) {
                float fx = (float)x;
                float fy = (float)y;

                if (y > 0 && gWallGrid[(y-1) * gGridWidth + x] == 1) {
                    gWalls[gWallCount].x1 = fx;
                    gWalls[gWallCount].z1 = fy;
                    gWalls[gWallCount].x2 = fx + 1;
                    gWalls[gWallCount].z2 = fy;
                    gWalls[gWallCount].isExit = 1;
                    gWallCount++;
                }
                if (y < gGridHeight - 1 && gWallGrid[(y+1) * gGridWidth + x] == 1) {
                    gWalls[gWallCount].x1 = fx + 1;
                    gWalls[gWallCount].z1 = fy + 1;
                    gWalls[gWallCount].x2 = fx;
                    gWalls[gWallCount].z2 = fy + 1;
                    gWalls[gWallCount].isExit = 1;
                    gWallCount++;
                }
                if (x > 0 && gWallGrid[y * gGridWidth + x - 1] == 1) {
                    gWalls[gWallCount].x1 = fx;
                    gWalls[gWallCount].z1 = fy + 1;
                    gWalls[gWallCount].x2 = fx;
                    gWalls[gWallCount].z2 = fy;
                    gWalls[gWallCount].isExit = 1;
                    gWallCount++;
                }
                if (x < gGridWidth - 1 && gWallGrid[y * gGridWidth + x + 1] == 1) {
                    gWalls[gWallCount].x1 = fx + 1;
                    gWalls[gWallCount].z1 = fy;
                    gWalls[gWallCount].x2 = fx + 1;
                    gWalls[gWallCount].z2 = fy + 1;
                    gWalls[gWallCount].isExit = 1;
                    gWallCount++;
                }
            }
        }
    }
}

/* ============== Collision Detection ============== */

static int isWall(int x, int y)
{
    if (x < 0 || x >= gGridWidth || y < 0 || y >= gGridHeight) return 1;
    return gWallGrid[y * gGridWidth + x] == 1;
}

static int isExit(int x, int y)
{
    if (x < 0 || x >= gGridWidth || y < 0 || y >= gGridHeight) return 0;
    return gWallGrid[y * gGridWidth + x] == 2;
}

static int checkCollision(float x, float y)
{
    int minX = (int)(x - PLAYER_RADIUS);
    int maxX = (int)(x + PLAYER_RADIUS);
    int minY = (int)(y - PLAYER_RADIUS);
    int maxY = (int)(y + PLAYER_RADIUS);

    for (int cy = minY; cy <= maxY; cy++) {
        for (int cx = minX; cx <= maxX; cx++) {
            if (isWall(cx, cy)) {
                float closestX = fmaxf((float)cx, fminf(x, (float)(cx + 1)));
                float closestY = fmaxf((float)cy, fminf(y, (float)(cy + 1)));
                float dx = x - closestX;
                float dy = y - closestY;
                if (dx * dx + dy * dy < PLAYER_RADIUS * PLAYER_RADIUS) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

static void movePlayer(float dx, float dy)
{
    float newX = gPlayer.x + dx;
    float newY = gPlayer.y + dy;

    if (!checkCollision(newX, newY)) {
        gPlayer.x = newX;
        gPlayer.y = newY;
    } else if (!checkCollision(newX, gPlayer.y)) {
        gPlayer.x = newX;
    } else if (!checkCollision(gPlayer.x, newY)) {
        gPlayer.y = newY;
    }

    int px = (int)gPlayer.x;
    int py = (int)gPlayer.y;
    if (isExit(px, py)) {
        if (gWinSound) Mix_PlayChannel(-1, gWinSound, 0);
        if (gCurrentLevel < 2) {
            gState = STATE_LEVEL_COMPLETE;
        } else {
            gState = STATE_WIN;
        }
    }
}

/* ============== Level Management ============== */

static void loadLevel(int level)
{
    gCurrentLevel = level;
    LevelConfig *cfg = &gLevels[level];

    srand((unsigned int)time(NULL) + level);
    generateMaze(cfg->mazeWidth, cfg->mazeHeight);

    gPlayer.x = 1.5f;
    gPlayer.y = 1.5f;
    gPlayer.angle = 0;
}

/* ============== OpenGL Rendering ============== */

static void setupGL(void)
{
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 3.0f);
    glFogf(GL_FOG_END, 15.0f);
    float fogColor[] = {0.1f, 0.1f, 0.15f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
}

static void renderWalls(void)
{
    for (int i = 0; i < gWallCount; i++) {
        Wall *w = &gWalls[i];

        if (w->isExit) {
            glBindTexture(GL_TEXTURE_2D, gExitTexture);
        } else {
            glBindTexture(GL_TEXTURE_2D, gBrickTexture);
        }

        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f(w->x1, 0, w->z1);
        glTexCoord2f(1, 1); glVertex3f(w->x2, 0, w->z2);
        glTexCoord2f(1, 0); glVertex3f(w->x2, WALL_HEIGHT, w->z2);
        glTexCoord2f(0, 0); glVertex3f(w->x1, WALL_HEIGHT, w->z1);
        glEnd();
    }
}

static void renderFloorCeiling(void)
{
    float size = (float)(gGridWidth > gGridHeight ? gGridWidth : gGridHeight);

    /* Floor */
    glBindTexture(GL_TEXTURE_2D, gFloorTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);        glVertex3f(0, 0, 0);
    glTexCoord2f(size, 0);     glVertex3f(size, 0, 0);
    glTexCoord2f(size, size);  glVertex3f(size, 0, size);
    glTexCoord2f(0, size);     glVertex3f(0, 0, size);
    glEnd();

    /* Ceiling */
    glBindTexture(GL_TEXTURE_2D, gCeilingTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);        glVertex3f(0, WALL_HEIGHT, 0);
    glTexCoord2f(0, size);     glVertex3f(0, WALL_HEIGHT, size);
    glTexCoord2f(size, size);  glVertex3f(size, WALL_HEIGHT, size);
    glTexCoord2f(size, 0);     glVertex3f(size, WALL_HEIGHT, 0);
    glEnd();
}

static void renderScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Camera */
    float lookX = gPlayer.x + cosf(gPlayer.angle);
    float lookZ = gPlayer.y + sinf(gPlayer.angle);
    gluLookAt(gPlayer.x, PLAYER_HEIGHT, gPlayer.y,
              lookX, PLAYER_HEIGHT, lookZ,
              0, 1, 0);

    glColor3f(1, 1, 1);
    renderFloorCeiling();
    renderWalls();
}

/* ============== 2D Overlay Rendering ============== */

static void beginOrtho(void)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void endOrtho(void)
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_TEXTURE_2D);
}

static void drawRect(float x, float y, float w, float h, float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

/* Simple bar indicator instead of text (pspgl lacks bitmap fonts) */
static void drawBar(float x, float y, float w, float h, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

/* Draw a simple indicator triangle */
static void drawTriangle(float x, float y, float size, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y);
    glVertex2f(x + size, y + size/2);
    glVertex2f(x, y + size);
    glEnd();
}

/* ============== Game State Rendering ============== */

static void updateFPS(void)
{
    gFrameCount++;
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsed = currentTime - gLastTime;

    if (elapsed >= 1000) {
        gFPS = (gFrameCount * 1000) / elapsed;
        gFrameCount = 0;
        gLastTime = currentTime;
    }
}

static void renderHUD(void)
{
    beginOrtho();

    /* Level indicator - bars */
    for (int i = 0; i <= gCurrentLevel; i++) {
        drawBar(10 + i * 25, 10, 20, 15, 1, 1, 1);
    }

    /* FPS indicator - green/yellow/red based on performance */
    float fpsColor = gFPS >= 50 ? 1.0f : (gFPS >= 30 ? 0.5f : 0.0f);
    drawBar(SCREEN_WIDTH - 60, 10, 50 * (gFPS / 60.0f), 10, fpsColor, 1.0f - fpsColor, 0);

    /* Exit hint - green arrow at bottom */
    drawTriangle(10, SCREEN_HEIGHT - 30, 20, 0, 1, 0);

    endOrtho();
}

static void renderMenu(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    beginOrtho();

    drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.05f, 0.05f, 0.1f, 1);

    /* Title - large white bar */
    drawBar(140, 40, 200, 30, 1, 1, 1);

    /* Menu items - boxes with selection indicator */
    for (int i = 0; i < 2; i++) {
        float r = (i == gMenuSelection) ? 1.0f : 0.4f;
        float g = (i == gMenuSelection) ? 1.0f : 0.4f;
        float b = (i == gMenuSelection) ? 0.0f : 0.4f;

        drawBar(180, 110 + i * 40, 120, 25, r, g, b);

        /* Selection arrow */
        if (i == gMenuSelection) {
            drawTriangle(155, 112 + i * 40, 20, 1, 1, 0);
        }
    }

    /* Start = green tint, Quit = red tint inside the bars */
    drawBar(185, 115, 30, 15, 0, 0.8f, 0);  /* Start indicator */
    drawBar(185, 155, 30, 15, 0.8f, 0, 0);  /* Quit indicator */

    endOrtho();
}

static void renderPause(void)
{
    beginOrtho();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0.7f);
    glDisable(GL_BLEND);

    /* Pause title bar */
    drawBar(180, 60, 120, 25, 1, 1, 1);

    /* Menu items */
    for (int i = 0; i < 2; i++) {
        float r = (i == gPauseSelection) ? 1.0f : 0.4f;
        float g = (i == gPauseSelection) ? 1.0f : 0.4f;
        float b = (i == gPauseSelection) ? 0.0f : 0.4f;

        drawBar(180, 120 + i * 40, 120, 25, r, g, b);

        if (i == gPauseSelection) {
            drawTriangle(155, 122 + i * 40, 20, 1, 1, 0);
        }
    }

    /* Resume = green, Quit = red */
    drawBar(185, 125, 30, 15, 0, 0.8f, 0);
    drawBar(185, 165, 30, 15, 0.8f, 0, 0);

    endOrtho();
}

static void renderLevelComplete(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    beginOrtho();

    drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.05f, 0.2f, 0.05f, 1);

    /* Level complete - green bars showing level number */
    for (int i = 0; i <= gCurrentLevel; i++) {
        drawBar(160 + i * 60, 80, 50, 40, 0, 1, 0);
    }

    /* Checkmark shape */
    drawBar(200, 150, 80, 8, 1, 1, 1);

    /* X button prompt */
    drawBar(220, 190, 40, 30, 0.3f, 0.3f, 0.8f);

    endOrtho();
}

static void renderWin(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    beginOrtho();

    drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.2f, 0.1f, 0.2f, 1);

    /* Victory - gold star shape (triangle) */
    drawTriangle(200, 40, 80, 1, 0.85f, 0);

    /* All 3 level bars - completed */
    for (int i = 0; i < 3; i++) {
        drawBar(140 + i * 70, 130, 60, 30, 0, 1, 0);
    }

    /* X button prompt */
    drawBar(220, 200, 40, 30, 0.3f, 0.3f, 0.8f);

    endOrtho();
}

/* ============== Input Handling ============== */

static void handleMenuInput(void)
{
    sceCtrlReadBufferPositive(&gPad, 1);

    if (gPad.Buttons) {
        if (!gButtonPressed) {
            if (gPad.Buttons & PSP_CTRL_UP) {
                gMenuSelection = (gMenuSelection + 1) % 2;
                if (gSelectSound) Mix_PlayChannel(-1, gSelectSound, 0);
            }
            if (gPad.Buttons & PSP_CTRL_DOWN) {
                gMenuSelection = (gMenuSelection + 1) % 2;
                if (gSelectSound) Mix_PlayChannel(-1, gSelectSound, 0);
            }
            if (gPad.Buttons & PSP_CTRL_CROSS) {
                if (gMenuSelection == 0) {
                    loadLevel(0);
                    gState = STATE_GAME;
                    Mix_PlayMusic(gMusic, -1);
                } else {
                    gState = STATE_QUIT;
                }
            }
            gButtonPressed = 1;
        }
    } else {
        gButtonPressed = 0;
    }
}

static void handleGameInput(void)
{
    sceCtrlReadBufferPositive(&gPad, 1);

    float moveX = 0, moveY = 0;

    if (gPad.Buttons & PSP_CTRL_UP) {
        moveX += cosf(gPlayer.angle) * MOVE_SPEED;
        moveY += sinf(gPlayer.angle) * MOVE_SPEED;
    }
    if (gPad.Buttons & PSP_CTRL_DOWN) {
        moveX -= cosf(gPlayer.angle) * MOVE_SPEED;
        moveY -= sinf(gPlayer.angle) * MOVE_SPEED;
    }
    if (gPad.Buttons & PSP_CTRL_LEFT) {
        gPlayer.angle -= ROT_SPEED;
    }
    if (gPad.Buttons & PSP_CTRL_RIGHT) {
        gPlayer.angle += ROT_SPEED;
    }
    if (gPad.Buttons & PSP_CTRL_LTRIGGER) {
        float strafeAngle = gPlayer.angle - M_PI / 2;
        moveX += cosf(strafeAngle) * MOVE_SPEED;
        moveY += sinf(strafeAngle) * MOVE_SPEED;
    }
    if (gPad.Buttons & PSP_CTRL_RTRIGGER) {
        float strafeAngle = gPlayer.angle + M_PI / 2;
        moveX += cosf(strafeAngle) * MOVE_SPEED;
        moveY += sinf(strafeAngle) * MOVE_SPEED;
    }

    /* Analog stick */
    if (gPad.Lx != 128 || gPad.Ly != 128) {
        float axisX = (gPad.Lx - 128) / 128.0f;
        float axisY = (gPad.Ly - 128) / 128.0f;

        if (fabsf(axisX) > 0.2f) {
            gPlayer.angle += axisX * ROT_SPEED;
        }
        if (fabsf(axisY) > 0.2f) {
            moveX -= cosf(gPlayer.angle) * axisY * MOVE_SPEED;
            moveY -= sinf(gPlayer.angle) * axisY * MOVE_SPEED;
        }
    }

    while (gPlayer.angle < 0) gPlayer.angle += 2 * M_PI;
    while (gPlayer.angle >= 2 * M_PI) gPlayer.angle -= 2 * M_PI;

    movePlayer(moveX, moveY);

    if (gPad.Buttons & PSP_CTRL_START) {
        if (!gButtonPressed) {
            gState = STATE_PAUSE;
            gPauseSelection = 0;
            gButtonPressed = 1;
        }
    } else if (!(gPad.Buttons & (PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT |
                                  PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_CROSS))) {
        gButtonPressed = 0;
    }
}

static void handlePauseInput(void)
{
    sceCtrlReadBufferPositive(&gPad, 1);

    if (gPad.Buttons) {
        if (!gButtonPressed) {
            if (gPad.Buttons & PSP_CTRL_UP) {
                gPauseSelection = (gPauseSelection + 1) % 2;
                if (gSelectSound) Mix_PlayChannel(-1, gSelectSound, 0);
            }
            if (gPad.Buttons & PSP_CTRL_DOWN) {
                gPauseSelection = (gPauseSelection + 1) % 2;
                if (gSelectSound) Mix_PlayChannel(-1, gSelectSound, 0);
            }
            if (gPad.Buttons & PSP_CTRL_CROSS) {
                if (gPauseSelection == 0) {
                    gState = STATE_GAME;
                } else {
                    gState = STATE_MENU;
                    Mix_HaltMusic();
                }
            }
            if (gPad.Buttons & PSP_CTRL_START) {
                gState = STATE_GAME;
            }
            gButtonPressed = 1;
        }
    } else {
        gButtonPressed = 0;
    }
}

static void handleLevelCompleteInput(void)
{
    sceCtrlReadBufferPositive(&gPad, 1);

    if (gPad.Buttons & PSP_CTRL_CROSS) {
        if (!gButtonPressed) {
            gCurrentLevel++;
            loadLevel(gCurrentLevel);
            gState = STATE_GAME;
            gButtonPressed = 1;
        }
    } else {
        gButtonPressed = 0;
    }
}

static void handleWinInput(void)
{
    sceCtrlReadBufferPositive(&gPad, 1);

    if (gPad.Buttons & PSP_CTRL_CROSS) {
        if (!gButtonPressed) {
            gState = STATE_MENU;
            gMenuSelection = 0;
            Mix_HaltMusic();
            gButtonPressed = 1;
        }
    } else {
        gButtonPressed = 0;
    }
}

/* ============== Main ============== */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    /* Initialize SDL for audio only */
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        return 1;
    }

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        SDL_Quit();
        return 1;
    }

    /* Initialize GLUT/OpenGL */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("3D Maze");

    setupGL();
    initTextures();

    /* Generate and load audio */
    generateAudioFiles();
    gMusic = Mix_LoadMUS("bgmusic.wav");
    gWinSound = Mix_LoadWAV("win.wav");
    gSelectSound = Mix_LoadWAV("select.wav");
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    srand((unsigned int)time(NULL));
    gState = STATE_MENU;
    gLastTime = SDL_GetTicks();

    /* Main loop */
    while (gState != STATE_QUIT) {
        updateFPS();

        switch (gState) {
            case STATE_MENU:
                handleMenuInput();
                renderMenu();
                break;
            case STATE_GAME:
                handleGameInput();
                renderScene();
                renderHUD();
                break;
            case STATE_PAUSE:
                handlePauseInput();
                renderScene();
                renderPause();
                break;
            case STATE_LEVEL_COMPLETE:
                handleLevelCompleteInput();
                renderLevelComplete();
                break;
            case STATE_WIN:
                handleWinInput();
                renderWin();
                break;
            default:
                break;
        }

        glutSwapBuffers();
        sceDisplayWaitVblankStart();
    }

    /* Cleanup */
    if (gMusic) Mix_FreeMusic(gMusic);
    if (gWinSound) Mix_FreeChunk(gWinSound);
    if (gSelectSound) Mix_FreeChunk(gSelectSound);

    glDeleteTextures(1, &gBrickTexture);
    glDeleteTextures(1, &gExitTexture);
    glDeleteTextures(1, &gFloorTexture);
    glDeleteTextures(1, &gCeilingTexture);

    if (gWallGrid) free(gWallGrid);
    if (gWalls) free(gWalls);

    Mix_CloseAudio();
    SDL_Quit();

    sceKernelExitGame();
    return 0;
}
