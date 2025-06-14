#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define GRID_COLS 32       // Timeline length
#define GRID_ROWS 8        // Number of notes
#define CELL_SIZE 30       // Pixel size of each grid cell
#define TIMELINE_HEIGHT 40 // Height of timeline in pixels

// Window dimensions
const unsigned int SCR_WIDTH = GRID_COLS * CELL_SIZE + 200; // Extra space for labels
const unsigned int SCR_HEIGHT = GRID_ROWS * CELL_SIZE + TIMELINE_HEIGHT + 60;

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

// Grid state
typedef struct
{
    bool cells[GRID_ROWS][GRID_COLS];
    int currentPlayColumn;
    bool isPlaying;
    float startTime;
    float tempo; // Beats per minute
} State;

State state = {
    .currentPlayColumn = -1,
    .isPlaying = false,
    .startTime = 0.0f,
    .tempo = 120.0f};

void playNoteSound(int row)
{
    char filename[256];
    snprintf(filename, sizeof(filename), "sounds\\%s.wav", NOTE_NAMES[row]);
    PlaySound(filename, NULL, SND_FILENAME | SND_ASYNC);
}

void playCurrentColumn()
{
    if (state.currentPlayColumn >= 0)
    {
        // Play all active notes in current column
        for (int row = 0; row < GRID_ROWS; row++)
        {
            if (state.cells[row][state.currentPlayColumn])
            {
                playNoteSound(row);
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

void drawGrid()
{
    float gridStartX = 100.0f; // Space for labels
    float gridStartY = 50.0f;  // Space for timeline

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
            drawText(number, gridStartX + col * CELL_SIZE, 20.0f, 1.0f);
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
            if (state.cells[row][col])
            {
                float x = gridStartX + col * CELL_SIZE;
                float y = gridStartY + row * CELL_SIZE;

                glColor3f(NOTE_COLORS[row][0],
                          NOTE_COLORS[row][1],
                          NOTE_COLORS[row][2]);

                glBegin(GL_QUADS);
                glVertex2f(x + 2, y + 2);
                glVertex2f(x + CELL_SIZE - 2, y + 2);
                glVertex2f(x + CELL_SIZE - 2, y + CELL_SIZE - 2);
                glVertex2f(x + 2, y + CELL_SIZE - 2);
                glEnd();
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
    if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Convert mouse coordinates to grid coordinates
        float gridStartX = 100.0f;
        float gridStartY = 50.0f;

        int col = (int)((xpos - gridStartX) / CELL_SIZE);
        int row = (int)((ypos - gridStartY) / CELL_SIZE);

        if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS)
        {
            // Toggle cell state
            state.cells[row][col] = !state.cells[row][col];

            // If toggled on, play the note
            if (state.cells[row][col])
            {
                playNoteSound(row);
            }
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
    glfwSetKeyCallback(window, key_callback);

    printf("Controls:\n");
    printf("- Click grid cells to toggle notes\n");
    printf("- Space: Play/Pause\n");
    printf("- Up/Down: Adjust tempo\n");
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
        drawGrid();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}