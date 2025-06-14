#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define GRID_COLS 32       // Timeline length
#define GRID_ROWS 8        // Number of notes
#define CELL_SIZE 30       // Pixel size of each grid cell
#define TIMELINE_HEIGHT 40 // Height of timeline in pixels
#define MENU_HEIGHT 30     // Height of instrument menu

// Window dimensions
const unsigned int SCR_WIDTH = GRID_COLS * CELL_SIZE + 200; // Extra space for labels
const unsigned int SCR_HEIGHT = GRID_ROWS * CELL_SIZE + TIMELINE_HEIGHT + MENU_HEIGHT + 60;

// Note names and frequencies
const char *NOTE_NAMES[GRID_ROWS] = {
    "C5", "B4", "A4", "G4", "F4", "E4", "D4", "C4"};

const float NOTE_FREQUENCIES[GRID_ROWS] = {
    523.25f, // C5
    493.88f, // B4
    440.00f, // A4
    392.00f, // G4
    349.23f, // F4
    329.63f, // E4
    293.66f, // D4
    261.63f  // C4
};

// Colors
const float NOTE_COLORS[GRID_ROWS][3] = {
    {1.0f, 0.0f, 0.5f}, // C5 - Pink
    {0.5f, 0.0f, 1.0f}, // B4 - Purple
    {0.0f, 0.0f, 1.0f}, // A4 - Blue
    {0.0f, 1.0f, 1.0f}, // G4 - Cyan
    {0.0f, 1.0f, 0.0f}, // F4 - Green
    {1.0f, 1.0f, 0.0f}, // E4 - Yellow
    {1.0f, 0.5f, 0.0f}, // D4 - Orange
    {1.0f, 0.0f, 0.0f}  // C4 - Red
};

// Instruments
typedef enum
{
    PIANO,
    SYNTH,
    BELL,
    NUM_INSTRUMENTS
} Instrument;

const char *INSTRUMENT_NAMES[NUM_INSTRUMENTS] = {
    "Piano",
    "Synth",
    "Bell"};

// Note cell structure
typedef struct
{
    bool active;
    Instrument instrument;
} NoteCell;

// Grid state
typedef struct
{
    NoteCell cells[GRID_ROWS][GRID_COLS];
    int currentPlayColumn;
    bool isPlaying;
    float startTime;
    float tempo; // Beats per minute
    Instrument currentInstrument;
    bool showInstrumentMenu;
    int menuHoverItem;
} State;

State state = {
    .currentPlayColumn = -1,
    .isPlaying = false,
    .startTime = 0.0f,
    .tempo = 120.0f,
    .currentInstrument = PIANO,
    .showInstrumentMenu = false,
    .menuHoverItem = -1};

void playNoteSound(int row, Instrument instrument)
{
    char filename[256];
    snprintf(filename, sizeof(filename), "sounds\\%s\\%s.wav",
             INSTRUMENT_NAMES[instrument], NOTE_NAMES[row]);
    PlaySound(filename, NULL, SND_FILENAME | SND_ASYNC);
}

void playCurrentColumn()
{
    if (state.currentPlayColumn >= 0)
    {
        // Play all active notes in current column
        for (int row = 0; row < GRID_ROWS; row++)
        {
            if (state.cells[row][state.currentPlayColumn].active)
            {
                playNoteSound(row, state.cells[row][state.currentPlayColumn].instrument);
            }
        }
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void drawText(const char *text, float x, float y, float scale)
{
    // Simple rectangle-based character rendering
    // This is a placeholder - in a real app, you'd want proper text rendering
    glColor3f(1.0f, 1.0f, 1.0f);
    float len = strlen(text) * 8.0f * scale;
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x + len, y);
    glEnd();
}

void drawInstrumentMenu()
{
    float menuX = 10.0f;
    float menuY = MENU_HEIGHT;
    float menuWidth = 100.0f;
    float itemHeight = 25.0f;

    // Draw current instrument
    char currentInst[64];
    snprintf(currentInst, sizeof(currentInst), "Instrument: %s",
             INSTRUMENT_NAMES[state.currentInstrument]);
    drawText(currentInst, menuX, menuY - 20, 1.0f);

    if (state.showInstrumentMenu)
    {
        // Draw menu background
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(menuX, menuY);
        glVertex2f(menuX + menuWidth, menuY);
        glVertex2f(menuX + menuWidth, menuY + itemHeight * NUM_INSTRUMENTS);
        glVertex2f(menuX, menuY + itemHeight * NUM_INSTRUMENTS);
        glEnd();

        // Draw menu items
        for (int i = 0; i < NUM_INSTRUMENTS; i++)
        {
            float itemY = menuY + i * itemHeight;

            // Highlight if hovered
            if (i == state.menuHoverItem)
            {
                glColor3f(0.4f, 0.4f, 0.4f);
                glBegin(GL_QUADS);
                glVertex2f(menuX, itemY);
                glVertex2f(menuX + menuWidth, itemY);
                glVertex2f(menuX + menuWidth, itemY + itemHeight);
                glVertex2f(menuX, itemY + itemHeight);
                glEnd();
            }

            drawText(INSTRUMENT_NAMES[i], menuX + 5, itemY + itemHeight / 2, 1.0f);
        }
    }
}

void drawGrid()
{
    float gridStartX = 100.0f;              // Space for labels
    float gridStartY = 50.0f + MENU_HEIGHT; // Space for timeline and menu

    // Draw note labels
    for (int row = 0; row < GRID_ROWS; row++)
    {
        float y = gridStartY + row * CELL_SIZE;
        drawText(NOTE_NAMES[row], 10.0f, y + CELL_SIZE / 2, 1.0f);
    }

    // Draw timeline numbers
    for (int col = 0; col < GRID_COLS; col++)
    {
        if (col % 4 == 0)
        { // Draw number every 4 beats
            char number[4];
            sprintf(number, "%d", col + 1);
            drawText(number, gridStartX + col * CELL_SIZE, 20.0f + MENU_HEIGHT, 1.0f);
        }
    }

    // Draw grid lines
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINES);

    // Vertical lines
    for (int col = 0; col <= GRID_COLS; col++)
    {
        float x = gridStartX + col * CELL_SIZE;
        glVertex2f(x, gridStartY);
        glVertex2f(x, gridStartY + GRID_ROWS * CELL_SIZE);

        // Make beat lines brighter
        if (col % 4 == 0)
        {
            glColor3f(0.5f, 0.5f, 0.5f);
            glVertex2f(x, gridStartY);
            glVertex2f(x, gridStartY + GRID_ROWS * CELL_SIZE);
            glColor3f(0.3f, 0.3f, 0.3f);
        }
    }

    // Horizontal lines
    for (int row = 0; row <= GRID_ROWS; row++)
    {
        float y = gridStartY + row * CELL_SIZE;
        glVertex2f(gridStartX, y);
        glVertex2f(gridStartX + GRID_COLS * CELL_SIZE, y);
    }
    glEnd();

    // Draw filled cells
    for (int row = 0; row < GRID_ROWS; row++)
    {
        for (int col = 0; col < GRID_COLS; col++)
        {
            if (state.cells[row][col].active)
            {
                float x = gridStartX + col * CELL_SIZE;
                float y = gridStartY + row * CELL_SIZE;

                // Base color from note
                float r = NOTE_COLORS[row][0];
                float g = NOTE_COLORS[row][1];
                float b = NOTE_COLORS[row][2];

                // Modify color based on instrument
                switch (state.cells[row][col].instrument)
                {
                case PIANO:
                    // Keep original colors
                    break;
                case SYNTH:
                    // Make more neon
                    r = (r + 0.5f) * 0.8f;
                    g = (g + 0.5f) * 0.8f;
                    b = (b + 0.8f) * 0.8f;
                    break;
                case BELL:
                    // Make more metallic
                    r = (r + 0.7f) * 0.7f;
                    g = (g + 0.7f) * 0.7f;
                    b = (b + 0.7f) * 0.7f;
                    break;
                }

                glColor3f(r, g, b);
                glBegin(GL_QUADS);
                glVertex2f(x + 2, y + 2);
                glVertex2f(x + CELL_SIZE - 2, y + 2);
                glVertex2f(x + CELL_SIZE - 2, y + CELL_SIZE - 2);
                glVertex2f(x + 2, y + CELL_SIZE - 2);
                glEnd();

                // Draw a small indicator for the instrument
                float indicatorSize = 6.0f;
                glColor3f(1.0f, 1.0f, 1.0f);
                switch (state.cells[row][col].instrument)
                {
                case PIANO:
                    // Draw a small rectangle
                    glBegin(GL_LINE_LOOP);
                    glVertex2f(x + 4, y + 4);
                    glVertex2f(x + 4 + indicatorSize, y + 4);
                    glVertex2f(x + 4 + indicatorSize, y + 4 + indicatorSize);
                    glVertex2f(x + 4, y + 4 + indicatorSize);
                    glEnd();
                    break;
                case SYNTH:
                    // Draw a triangle
                    glBegin(GL_LINE_LOOP);
                    glVertex2f(x + 4, y + 4 + indicatorSize);
                    glVertex2f(x + 4 + indicatorSize / 2, y + 4);
                    glVertex2f(x + 4 + indicatorSize, y + 4 + indicatorSize);
                    glEnd();
                    break;
                case BELL:
                    // Draw a circle
                    const int segments = 8;
                    glBegin(GL_LINE_LOOP);
                    for (int i = 0; i < segments; i++)
                    {
                        float angle = 2.0f * 3.1415926f * i / segments;
                        float cx = x + 4 + indicatorSize / 2 + cosf(angle) * indicatorSize / 2;
                        float cy = y + 4 + indicatorSize / 2 + sinf(angle) * indicatorSize / 2;
                        glVertex2f(cx, cy);
                    }
                    glEnd();
                    break;
                }
            }
        }
    }

    // Draw playhead
    if (state.currentPlayColumn >= 0)
    {
        float x = gridStartX + state.currentPlayColumn * CELL_SIZE;
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(x, gridStartY);
        glVertex2f(x, gridStartY + GRID_ROWS * CELL_SIZE);
        glEnd();
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Check if clicking on instrument menu area
        if (xpos < 110.0f && ypos < MENU_HEIGHT + NUM_INSTRUMENTS * 25.0f)
        {
            if (ypos < MENU_HEIGHT)
            {
                // Toggle menu
                state.showInstrumentMenu = !state.showInstrumentMenu;
            }
            else if (state.showInstrumentMenu && state.menuHoverItem >= 0)
            {
                // Select instrument
                state.currentInstrument = state.menuHoverItem;
                state.showInstrumentMenu = false;
            }
            return;
        }

        // Convert mouse coordinates to grid coordinates
        float gridStartX = 100.0f;
        float gridStartY = 50.0f + MENU_HEIGHT;

        int col = (int)((xpos - gridStartX) / CELL_SIZE);
        int row = (int)((ypos - gridStartY) / CELL_SIZE);

        if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS)
        {
            // Toggle cell state
            state.cells[row][col].active = !state.cells[row][col].active;

            // If toggled on, set instrument and play the note
            if (state.cells[row][col].active)
            {
                state.cells[row][col].instrument = state.currentInstrument;
                playNoteSound(row, state.currentInstrument);
            }
        }
    }
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (state.showInstrumentMenu)
    {
        // Update menu hover state
        if (xpos < 110.0f && ypos >= MENU_HEIGHT && ypos < MENU_HEIGHT + NUM_INSTRUMENTS * 25.0f)
        {
            state.menuHoverItem = (int)((ypos - MENU_HEIGHT) / 25.0f);
        }
        else
        {
            state.menuHoverItem = -1;
        }
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        state.isPlaying = !state.isPlaying;
        if (state.isPlaying)
        {
            state.currentPlayColumn = 0;
            state.startTime = (float)glfwGetTime();
            playCurrentColumn();
        }
        else
        {
            state.currentPlayColumn = -1;
        }
    }
    else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    // Tempo control
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        state.tempo = fmin(state.tempo + 5.0f, 240.0f);
        printf("Tempo: %.1f BPM\n", state.tempo);
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        state.tempo = fmax(state.tempo - 5.0f, 60.0f);
        printf("Tempo: %.1f BPM\n", state.tempo);
    }
    // Instrument selection with number keys
    else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_3 && action == GLFW_PRESS)
    {
        int instrument = key - GLFW_KEY_1;
        if (instrument < NUM_INSTRUMENTS)
        {
            state.currentInstrument = instrument;
            printf("Selected instrument: %s\n", INSTRUMENT_NAMES[instrument]);
        }
    }
}

void updatePlayback()
{
    if (state.isPlaying)
    {
        float currentTime = (float)glfwGetTime() - state.startTime;
        float beatsPerSecond = state.tempo / 60.0f;
        float beatTime = 1.0f / beatsPerSecond;

        int newColumn = ((int)(currentTime / beatTime)) % GRID_COLS;

        if (newColumn != state.currentPlayColumn)
        {
            state.currentPlayColumn = newColumn;
            playCurrentColumn();
        }
    }
}

int main(void)
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Music Grid Sequencer", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    printf("Controls:\n");
    printf("- Click grid cells to toggle notes\n");
    printf("- Space: Play/Pause\n");
    printf("- Up/Down: Adjust tempo\n");
    printf("- Click 'Instrument' or press 1-3: Change instrument\n");
    printf("- ESC: Quit\n");

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Set up 2D projection
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        updatePlayback();
        drawInstrumentMenu();
        drawGrid();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}