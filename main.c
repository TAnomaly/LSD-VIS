#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define MAX_SEQUENCE_LENGTH 64
#define NUM_NOTES 8

// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Note frequencies (C4 to C5)
const float NOTE_FREQUENCIES[NUM_NOTES] = {
    261.63f, // C4
    293.66f, // D4
    329.63f, // E4
    349.23f, // F4
    392.00f, // G4
    440.00f, // A4
    493.88f, // B4
    523.25f  // C5
};

// Note colors
const float NOTE_COLORS[NUM_NOTES][3] = {
    {1.0f, 0.0f, 0.0f}, // C - Red
    {1.0f, 0.5f, 0.0f}, // D - Orange
    {1.0f, 1.0f, 0.0f}, // E - Yellow
    {0.0f, 1.0f, 0.0f}, // F - Green
    {0.0f, 1.0f, 1.0f}, // G - Cyan
    {0.0f, 0.0f, 1.0f}, // A - Blue
    {0.5f, 0.0f, 1.0f}, // B - Purple
    {1.0f, 0.0f, 0.5f}  // C - Pink
};

// Global state
typedef struct
{
    int sequence[MAX_SEQUENCE_LENGTH];
    int sequenceLength;
    int currentPlayingIndex;
    bool isPlaying;
    float startTime;
} State;

State state = {
    .sequenceLength = 0,
    .currentPlayingIndex = -1,
    .isPlaying = false,
    .startTime = 0.0f};

// Shader program
unsigned int shaderProgram;
unsigned int VBO, VAO;

// Function declarations
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
unsigned int createShaderProgram(const char *vertexPath, const char *fragmentPath);
void checkCompileErrors(unsigned int shader, const char *type);
char *readShaderFile(const char *filename);
void initializeBuffers(void);
int getNoteAtPosition(double xpos, double ypos);
void renderNoteBlocks(void);

int main(void)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Music Sequencer", NULL, NULL);
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

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }

    // Create and compile shaders
    shaderProgram = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    initializeBuffers();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderNoteBlocks();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int noteIndex = getNoteAtPosition(xpos, ypos);
        if (noteIndex >= 0 && state.sequenceLength < MAX_SEQUENCE_LENGTH)
        {
            state.sequence[state.sequenceLength++] = noteIndex;
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
            state.currentPlayingIndex = 0;
            state.startTime = glfwGetTime();
        }
        else
        {
            state.currentPlayingIndex = -1;
        }
    }
    else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

unsigned int createShaderProgram(const char *vertexPath, const char *fragmentPath)
{
    char *vertexCode = readShaderFile(vertexPath);
    char *fragmentCode = readShaderFile(fragmentPath);

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const char **)&vertexCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char **)&fragmentCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    checkCompileErrors(program, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    free(vertexCode);
    free(fragmentCode);

    return program;
}

void checkCompileErrors(unsigned int shader, const char *type)
{
    int success;
    char infoLog[1024];

    if (strcmp(type, "PROGRAM") != 0)
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n", type, infoLog);
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n", type, infoLog);
        }
    }
}

char *readShaderFile(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        fprintf(stderr, "Failed to open shader file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

void initializeBuffers(void)
{
    float vertices[] = {
        // positions        // texture coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

int getNoteAtPosition(double xpos, double ypos)
{
    // Convert screen coordinates to normalized coordinates
    float normalizedX = (float)(xpos / SCR_WIDTH * 2.0 - 1.0);
    float normalizedY = (float)(1.0 - ypos / SCR_HEIGHT * 2.0);

    // Calculate note block dimensions
    float blockWidth = 2.0f / NUM_NOTES;
    float blockHeight = 0.8f;
    float startX = -1.0f + blockWidth / 2.0f;
    float startY = blockHeight / 2.0f;

    // Check if click is within any note block
    for (int i = 0; i < NUM_NOTES; i++)
    {
        float blockX = startX + i * blockWidth;
        if (fabs(normalizedX - blockX) < blockWidth / 2.0f &&
            fabs(normalizedY - startY) < blockHeight / 2.0f)
        {
            return i;
        }
    }

    return -1;
}

void renderNoteBlocks(void)
{
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    float currentTime = glfwGetTime();
    int timeLocation = glGetUniformLocation(shaderProgram, "time");
    glUniform1f(timeLocation, currentTime);

    // Update playing state
    if (state.isPlaying && state.sequenceLength > 0)
    {
        float noteTime = 0.5f; // Time per note in seconds
        float elapsedTime = currentTime - state.startTime;
        int newIndex = (int)(elapsedTime / noteTime) % state.sequenceLength;

        if (newIndex != state.currentPlayingIndex)
        {
            state.currentPlayingIndex = newIndex;
            // Here we would trigger the sound for the current note
            // printf("Playing note: %f Hz\n", NOTE_FREQUENCIES[state.sequence[state.currentPlayingIndex]]);
        }
    }

    // Calculate note block dimensions
    float blockWidth = 2.0f / NUM_NOTES;
    float blockHeight = 0.8f;
    float startX = -1.0f + blockWidth / 2.0f;
    float startY = blockHeight / 2.0f;

    // Render each note block
    for (int i = 0; i < NUM_NOTES; i++)
    {
        float blockX = startX + i * blockWidth;

        // Set uniforms
        int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        float transform[16] = {
            blockWidth, 0.0f, 0.0f, 0.0f,
            0.0f, blockHeight, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            blockX, startY, 0.0f, 1.0f};
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);

        // Set color and state uniforms
        int colorLoc = glGetUniformLocation(shaderProgram, "baseColor");
        glUniform3fv(colorLoc, 1, NOTE_COLORS[i]);

        bool isSelected = false;
        for (int j = 0; j < state.sequenceLength; j++)
        {
            if (state.sequence[j] == i)
            {
                isSelected = true;
                break;
            }
        }

        bool isPlaying = (state.isPlaying &&
                          state.currentPlayingIndex >= 0 &&
                          state.sequence[state.currentPlayingIndex] == i);

        int selectedLoc = glGetUniformLocation(shaderProgram, "isSelected");
        glUniform1i(selectedLoc, isSelected);

        int playingLoc = glGetUniformLocation(shaderProgram, "isPlaying");
        glUniform1i(playingLoc, isPlaying);

        // Draw the note block
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}