#include "engine.h"
#include <raylib.h>
#include <raymath.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GRID_SPACING 30.0f
#define GRID_OFFSET_X 300.0f
#define GRID_OFFSET_Y 100.0f
#define GRID_OFFSET_Z 0.0f

// Structure to hold all piece textures
struct piece_textures {
    Texture2D w_king;
    Texture2D w_queen;
    Texture2D w_rook;
    Texture2D w_bishop;
    Texture2D w_knight;
    Texture2D w_pawn;
    Texture2D b_king;
    Texture2D b_queen;
    Texture2D b_rook;
    Texture2D b_bishop;
    Texture2D b_knight;
    Texture2D b_pawn;
};

// Add after the existing #defines
struct HighlightedBlock {
    bool active;
    int file;
    int rank;
    int tier;
};

// After struct HighlightedBlock, add a new struct for possible moves
struct MoveHighlights {
    struct block moves[64];  // Assuming max 64 possible moves
    int count;
};

// Load all piece textures
struct piece_textures load_piece_textures(void) {
    struct piece_textures textures = {
        .w_king = LoadTexture("assets/white_king.png"),
        .w_queen = LoadTexture("assets/white_queen.png"),
        .w_rook = LoadTexture("assets/white_rook.png"),
        .w_bishop = LoadTexture("assets/white_bishop.png"),
        .w_knight = LoadTexture("assets/white_knight.png"),
        .w_pawn = LoadTexture("assets/white_pawn.png"),
        .b_king = LoadTexture("assets/black_king.png"),
        .b_queen = LoadTexture("assets/black_queen.png"),
        .b_rook = LoadTexture("assets/black_rook.png"),
        .b_bishop = LoadTexture("assets/black_bishop.png"),
        .b_knight = LoadTexture("assets/black_knight.png"),
        .b_pawn = LoadTexture("assets/black_pawn.png")
    };
    return textures;
}

// Unload all piece textures
void unload_piece_textures(struct piece_textures textures) {
    UnloadTexture(textures.w_king);
    UnloadTexture(textures.w_queen);
    UnloadTexture(textures.w_rook);
    UnloadTexture(textures.w_bishop);
    UnloadTexture(textures.w_knight);
    UnloadTexture(textures.w_pawn);
    UnloadTexture(textures.b_king);
    UnloadTexture(textures.b_queen);
    UnloadTexture(textures.b_rook);
    UnloadTexture(textures.b_bishop);
    UnloadTexture(textures.b_knight);
    UnloadTexture(textures.b_pawn);
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ZChess");
    
    // After InitWindow(), add viewport tracking
    int currentWidth = WINDOW_WIDTH;
    int currentHeight = WINDOW_HEIGHT;

    // Setup camera
    Camera3D camera = {0};
    camera.position = (Vector3){ 20.0f, 20.0f, 20.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // After camera initialization
    camera.position = (Vector3){ 20.0f, 20.0f, 20.0f };  // Start further back
    float moveSpeed = 50.0f;                             // Movement speed multiplier

    // Load all textures
    struct piece_textures textures = load_piece_textures();

    // Create the arena with default piece positions
    struct arena arena = create_arena();

    SetTargetFPS(60);

    // After camera initialization
    struct HighlightedBlock highlighted = {
        .active = false,
        .file = 0,
        .rank = 0,
        .tier = 0
    };

    // In main(), after highlighted declaration
    struct MoveHighlights possible_moves = {
        .count = 0
    };

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        
        // Custom camera movement
        if (IsKeyDown(KEY_W)) camera.position.z -= moveSpeed * deltaTime;
        if (IsKeyDown(KEY_S)) camera.position.z += moveSpeed * deltaTime;
        if (IsKeyDown(KEY_A)) camera.position.x -= moveSpeed * deltaTime;
        if (IsKeyDown(KEY_D)) camera.position.x += moveSpeed * deltaTime;
        if (IsKeyDown(KEY_E)) camera.position.y += moveSpeed * deltaTime;
        if (IsKeyDown(KEY_Q)) camera.position.y -= moveSpeed * deltaTime;

        // Even faster when holding shift
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            UpdateCamera(&camera, CAMERA_FREE);
        } else {
            UpdateCamera(&camera, CAMERA_FREE);
        }

        // Add mouse ray-casting for block selection
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Ray ray = GetMouseRay(GetMousePosition(), camera);
            
            // Check intersection with each block's position
            float closest_hit = INFINITY;
            for (int t = 0; t < BLOCK_COLUMN_LENGTH; t++) {
                for (int r = 0; r < BLOCK_COLUMN_LENGTH; r++) {
                    for (int f = 0; f < BLOCK_COLUMN_LENGTH; f++) {
                        Vector3 position = {
                            f * GRID_SPACING - GRID_OFFSET_X,
                            t * GRID_SPACING - GRID_OFFSET_Y,
                            r * GRID_SPACING - GRID_OFFSET_Z
                        };
                        
                        BoundingBox box = {
                            (Vector3){ position.x - 2.0f, position.y - 2.0f, position.z - 2.0f },
                            (Vector3){ position.x + 2.0f, position.y + 2.0f, position.z + 2.0f }
                        };
                        
                        RayCollision collision = GetRayCollisionBox(ray, box);
                        if (collision.hit && collision.distance < closest_hit) {
                            closest_hit = collision.distance;
                            highlighted.active = true;
                            highlighted.file = f;
                            highlighted.rank = r;
                            highlighted.tier = t;
                            
                            // Get possible moves for the clicked piece
                            struct moves moves = obtain_arena_moves(arena, f, r, t);
                            possible_moves.count = moves.length;
                            for (int i = 0; i < moves.length; i++) {
                                possible_moves.moves[i] = moves.blocks[i];
                            }
                        }
                    }
                }
            }
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);

                // Draw grid for reference
                DrawGrid(10, 10.0f);

                // Draw pieces
                for (int tier = 0; tier < BLOCK_COLUMN_LENGTH; tier++) {
                    for (int rank = 0; rank < BLOCK_COLUMN_LENGTH; rank++) {
                        for (int file = 0; file < BLOCK_COLUMN_LENGTH; file++) {
                            Vector3 position = {
                                file * GRID_SPACING - GRID_OFFSET_X,
                                tier * GRID_SPACING - GRID_OFFSET_Y,
                                rank * GRID_SPACING - GRID_OFFSET_Z
                            };

                            struct token token = obtain_arena_token(arena, file, rank, tier);
                            if (token.piece != PIECE_NONE) {
                                Texture2D* texture = NULL;
                                
                                // Select the appropriate texture based on color and piece
                                if (token.color == COLOR_WHITE) {
                                    switch(token.piece) {
                                        case PIECE_KING: texture = &textures.w_king; break;
                                        case PIECE_QUEEN: texture = &textures.w_queen; break;
                                        case PIECE_ROOK: texture = &textures.w_rook; break;
                                        case PIECE_BISHOP: texture = &textures.w_bishop; break;
                                        case PIECE_KNIGHT: texture = &textures.w_knight; break;
                                        case PIECE_PAWN: texture = &textures.w_pawn; break;
                                        default: break;
                                    }
                                } else {
                                    switch(token.piece) {
                                        case PIECE_KING: texture = &textures.b_king; break;
                                        case PIECE_QUEEN: texture = &textures.b_queen; break;
                                        case PIECE_ROOK: texture = &textures.b_rook; break;
                                        case PIECE_BISHOP: texture = &textures.b_bishop; break;
                                        case PIECE_KNIGHT: texture = &textures.b_knight; break;
                                        case PIECE_PAWN: texture = &textures.b_pawn; break;
                                        default: break;
                                    }
                                }
                                
                                if (texture) {
                                    DrawBillboard(camera, *texture, position, 5.0f, WHITE);
                                }
                            }
                            else {
                                DrawSphere(position, 0.5f, BLACK);
                            }
                        }
                    }
                }

                // Draw highlighted block
                if (highlighted.active) {
                    Vector3 position = {
                        highlighted.file * GRID_SPACING - GRID_OFFSET_X,
                        highlighted.tier * GRID_SPACING - GRID_OFFSET_Y,
                        highlighted.rank * GRID_SPACING - GRID_OFFSET_Z
                    };
                    
                    DrawCubeWires(position, 5.0f, 5.0f, 5.0f, RED);
                }

                // Draw possible move highlights
                for (int i = 0; i < possible_moves.count; i++) {
                    Vector3 position = {
                        possible_moves.moves[i].file * GRID_SPACING - GRID_OFFSET_X,
                        possible_moves.moves[i].tier * GRID_SPACING - GRID_OFFSET_Y,
                        possible_moves.moves[i].rank * GRID_SPACING - GRID_OFFSET_Z
                    };
                    
                    DrawCubeWires(position, 5.0f, 5.0f, 5.0f, GREEN);  // Use different color for possible moves
                }

            EndMode3D();
            DrawFPS(10, 10);
        EndDrawing();
    }

    unload_piece_textures(textures);
    CloseWindow();
    return 0;
}
