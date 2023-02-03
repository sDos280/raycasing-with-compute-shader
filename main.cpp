#include "raylib.h"
#include "rlgl.h"
#include <cmath>

#define RAYS_COUNT 1200
#define FOV PI / 3
#define ANGLE_OFFSET FOV / RAYS_COUNT

// Shader Input Data structure
typedef struct ShaderInputData {
    Vector2 playerPosition;
    float playerAngle;
    float fov;
    float raysCount;
    float angleOffset;
    int screenWidth;
    int screenHeight;
} ShaderInputData;

// Shader Output OUT structure
typedef struct ShaderOutputData {
    float height;
    float color;
} ShaderOutputData;

int main() {
    const int screenWidth = RAYS_COUNT;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "ray-casting");
    SetWindowPosition(100, 100);

    ShaderInputData gameData = {{screenWidth * 0.5f, screenHeight * 0.5f}, 0, FOV, RAYS_COUNT, ANGLE_OFFSET, screenWidth, screenHeight};
    ShaderOutputData *columnsHeight = new ShaderOutputData[RAYS_COUNT];

    char *rayIntersectionCode = LoadFileText("../resources/shaders/rayIntersectionShader.comp");
    unsigned int rayIntersectionShader = rlCompileShader(rayIntersectionCode, RL_COMPUTE_SHADER);
    unsigned int rayIntersectionProgram = rlLoadComputeShaderProgram(rayIntersectionShader);
    UnloadFileText(rayIntersectionCode);

    unsigned int gameDataBuffer = rlLoadShaderBuffer(sizeof(ShaderInputData), &gameData, RL_STATIC_COPY);
    unsigned int columnsHeightBuffer = rlLoadShaderBuffer(RAYS_COUNT * sizeof(ShaderOutputData), nullptr, RL_DYNAMIC_COPY);

    SetTargetFPS(60);

    DisableCursor();

    while (!WindowShouldClose()) {
        Vector2 mousePositionDelta = GetMouseDelta();
        float deltaTime = GetFrameTime();

        if (IsKeyDown(KEY_W)){
            gameData.playerPosition.x += cos(-gameData.playerAngle) * deltaTime * 200;
            gameData.playerPosition.y -= sin(-gameData.playerAngle) * deltaTime * 200;
        }
        else if (IsKeyDown(KEY_S)){
            gameData.playerPosition.x -= cos(-gameData.playerAngle) * deltaTime * 200;
            gameData.playerPosition.y += sin(-gameData.playerAngle) * deltaTime * 200;

        }
        if (IsKeyDown(KEY_A)){
            float useAngle = gameData.playerAngle - PI * 0.5;
            gameData.playerPosition.x += cos(-useAngle) * deltaTime * 200;
            gameData.playerPosition.y -= sin(-useAngle) * deltaTime * 200;
        }
        else if (IsKeyDown(KEY_D)){
            float useAngle = gameData.playerAngle + PI * 0.5;
            gameData.playerPosition.x += cos(-useAngle) * deltaTime * 200;
            gameData.playerPosition.y -= sin(-useAngle) * deltaTime * 200;
        }

        gameData.playerAngle += (mousePositionDelta.x / screenHeight);

        rlUpdateShaderBuffer(gameDataBuffer, &gameData, sizeof(ShaderInputData), 0);

        rlEnableShader(rayIntersectionProgram);
        rlBindShaderBuffer(gameDataBuffer, 1);
        rlBindShaderBuffer(columnsHeightBuffer, 2);
        rlComputeShaderDispatch(RAYS_COUNT, 1, 1); // Each GPU unit will process a ray!
        rlDisableShader();

        rlReadShaderBuffer(columnsHeightBuffer, columnsHeight, RAYS_COUNT * sizeof(ShaderOutputData), 0);

        BeginDrawing();

        ClearBackground(BLACK);

        for (int i = 0; i < RAYS_COUNT; i++) {
            float height = columnsHeight[i].height;
            float color = columnsHeight[i].color;
            float startX = i * (screenWidth / RAYS_COUNT);
            float minScreenHeight = screenHeight * 0.5f;
            DrawLine(startX, minScreenHeight - height*0.5f, startX, minScreenHeight + height*0.5f, {static_cast<unsigned char>(color * 255), static_cast<unsigned char>(color * 255), static_cast<unsigned char>(color * 255), 255});
        }

        EndDrawing();
    }

    rlUnloadShaderBuffer(gameDataBuffer);
    rlUnloadShaderBuffer(columnsHeightBuffer);

    rlUnloadShaderProgram(rayIntersectionProgram);
    CloseWindow();

    delete[] columnsHeight;

    return 0;
}

