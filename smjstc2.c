#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <gif_lib.h>
#include <unistd.h>
#include <math.h>



#define TEXTURE_SIZE 40 // Размер одной текстуры (в пикселях)
#define PACMAN_SIZE 70
#define WINDOW_WIDTH (WIDTH * TEXTURE_SIZE)
#define WINDOW_HEIGHT (HEIGHT * TEXTURE_SIZE)
#define BORDER_OFFSET (TEXTURE_SIZE) // Отступ на одну текстуру от границы
#define WIDTH 24
#define HEIGHT 15
#define DELAY 40000 // Пауза для плавности
#define MAX_GHOSTS 3 // Максимальное количество скелетов

// Основное игровое поле
char field[HEIGHT][WIDTH] = {
":----------------------)",
"]......................|",
":......................)",
"]......................|",
":......................)",
"]......................|",
":......................)",
"]......................|",
":......................)",
"]......................|",
":......................)",
"]......................|",
":......................)",
"]......................|",
";======================(",
};

// Позиции и скорости персонажей
float pacman1X = 5.0f, pacman1Y = 9.0f;
float pacman1SpeedX = 0.0f, pacman1SpeedY = 0.0f;

float pacman2X = 6.0f, pacman2Y = 10.0f;  
float pacman2SpeedX = 0.0f, pacman2SpeedY = 0.0f;  

typedef struct {
    float x, y;             // Позиция
    int isAlive;            // Жив ли скелет
    int currentFrame;       // Текущий кадр анимации
    int lastFrameTime;      // Время последнего обновления кадра
    int hits;               // Сколько ударов получил
    int isTakingDamage;     // Получает ли урон
    int damageFrame;        // Текущий кадр анимации урона
    int lastDamageTime;     // Время последнего обновления анимации урона
} Ghost;



Ghost ghosts[MAX_GHOSTS]; // Массив для скелетов
int activeGhosts = MAX_GHOSTS; // Текущее количество активных скелетов

int currentFrame = 0;
int gifFrameCount = 0;

// Переменные для анимации шипов
int spikeFrameCount = 0;
int spikeCurrentFrame = 0;
int spikeLastFrameTime = 0;
int spikeFrameDelay = 200; // Задержка кадров анимации шипов

int quit = 0;
int pacmanLastFrameTime = 0;
int ghostLastFrameTime = 0;
int pacmanFrameDelay = 100;
int ghostFrameDelay = 225;   

int pacmanIdleCurrentFrame = 0;
int pacmanIdleFrameCount = 0;
int pacmanIdleLastFrameTime = 0;
int pacmanIdleFrameDelay = 225; // Регулируемая задержка для idle анимации Пакмана 1

// Переменные для второго Pac-Man
SDL_Texture **pacman2RunTextures = NULL;
SDL_Texture **pacman2IdleTextures = NULL;
int pacman2RunFrameCount = 0;
int pacman2IdleFrameCount = 0;
int pacman2CurrentFrame = 0;
int pacman2LastFrameTime = 0;
int pacman2FrameDelay = 100;
int pacman2IdleCurrentFrame = 0;
int pacman2IdleLastFrameTime = 0;
int pacman2IdleFrameDelay = 200;
int buttonX = WIDTH / 2 - 19/10; // Центр экрана
int buttonY = HEIGHT / 2;
int isButtonPressed = 0; // Флаг нажатия кнопки

// Прототипы функций
void updatePacman1Position();
void updatePacman2Position();
void handleKeyDown1(SDL_Keycode key);
void handleKeyDown2(SDL_Keycode key);
void handleKeyUp1(SDL_Keycode key);
void handleKeyUp2(SDL_Keycode key);
void renderButton();






int isGhostAlive = 1; // Флаг: жив ли скелет
int ghostHitCount = 0; // Счётчик ударов по скелету
const int ghostMaxHits = 3; // Количество ударов для смерти скелета



SDL_Texture **pacmanAttackTextures = NULL;
int pacmanAttackFrameCount = 0;
int pacmanAttackCurrentFrame = 0;
int pacmanAttackLastFrameTime = 0;
int pacmanAttackFrameDelay = 75; // Задержка для кадров удара
int isPacmanAttacking = 0; // Флаг для проверки, выполняется ли удар


// Новые переменные для анимации урона
SDL_Texture **ghostDamageTextures = NULL;
int ghostDamageFrameCount = 0;
int ghostDamageCurrentFrame = 0;
int ghostDamageLastFrameTime = 0;
int ghostDamageFrameDelay = 100; // Задержка между кадрами урона
int isGhostTakingDamage = 0; // Флаг анимации урона


SDL_Texture *buttonPressedTexture = NULL; // Texture for the pressed button
SDL_Texture *buttonTexture = NULL;
SDL_Texture *barTexture = NULL;
SDL_Texture *horizontalWallTexture = NULL;
SDL_Texture *cornerLeftTexture = NULL; // Для :
SDL_Texture *cornerRightTexture = NULL; // Для ]
SDL_Texture *bottomLeftTexture = NULL; // Для ;
SDL_Texture *floorTexture = NULL; // Для .
SDL_Texture *bottomWallTexture = NULL; // Для =
SDL_Texture *leftParenthesisTexture = NULL; // Для (
SDL_Texture *rightParenthesisTexture = NULL; // Для )
SDL_Texture **spikeTextures = NULL;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture **pacmanRunTextures = NULL;
SDL_Texture **pacmanIdleTextures = NULL;
SDL_Texture **ghostRunTextures = NULL;
int ghostGifFrameCount = 0;
int ghostCurrentFrame = 0;



int isNear(float x1, float y1, float x2, float y2) {
    float distance = sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    return distance <= 1.0f; // Проверяем, находится ли в радиусе 1 клетки
}



int loadGifAnimation(const char* filename, SDL_Texture*** textures, int* frameCount) {
    GifFileType *gifFile = DGifOpenFileName(filename, NULL);
    if (!gifFile) {
        fprintf(stderr, "Could not open GIF file.\n");
        return -1;
    }

    if (DGifSlurp(gifFile) != GIF_OK) {
        fprintf(stderr, "Failed to load GIF frames.\n");
        DGifCloseFile(gifFile, NULL);
        return -1;
    }
                        
    *frameCount = gifFile->ImageCount;
    *textures = (SDL_Texture**)malloc(sizeof(SDL_Texture*) * (*frameCount));

    for (int i = 0; i < *frameCount; ++i) {
        SavedImage *frame = &gifFile->SavedImages[i];
        ColorMapObject *colorMap = (gifFile->SColorMap) ? gifFile->SColorMap : frame->ImageDesc.ColorMap;

        SDL_Surface *surface = SDL_CreateRGBSurface(0, frame->ImageDesc.Width, frame->ImageDesc.Height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        for (int y = 0; y < frame->ImageDesc.Height; y++) {
            for (int x = 0; x < frame->ImageDesc.Width; x++) {
                int colorIndex = frame->RasterBits[y * frame->ImageDesc.Width + x];
                GifColorType color = colorMap->Colors[colorIndex];
                Uint32 pixelColor = SDL_MapRGBA(surface->format, color.Red, color.Green, color.Blue, colorIndex == gifFile->SBackGroundColor ? 0 : 255);
                ((Uint32*)surface->pixels)[y * frame->ImageDesc.Width + x] = pixelColor;
            }
        }
        (*textures)[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    DGifCloseFile(gifFile, NULL);
    return 0;
}

int loadGhostGifAnimation(const char* filename, SDL_Texture*** textures, int* frameCount) {
    return loadGifAnimation(filename, textures, frameCount);  
}


void renderGhosts() {
    for (int i = 0; i < MAX_GHOSTS; i++) {
        if (!ghosts[i].isAlive) continue;

        SDL_Rect dstRectGhost = { (int)(ghosts[i].x * TEXTURE_SIZE), (int)(ghosts[i].y * TEXTURE_SIZE), TEXTURE_SIZE, TEXTURE_SIZE };

        if (ghosts[i].isTakingDamage) {
            SDL_RenderCopy(renderer, ghostDamageTextures[ghosts[i].damageFrame], NULL, &dstRectGhost);
        } else {
            SDL_RenderCopy(renderer, ghostRunTextures[ghosts[i].currentFrame], NULL, &dstRectGhost);
        }
    }
}




void draw_field() {
    SDL_RenderClear(renderer);

    // Рендерим игровое поле
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            SDL_Rect dstRect = { x * TEXTURE_SIZE, y * TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE };
            switch (field[y][x]) {
                case '+':
                    SDL_RenderCopy(renderer, spikeTextures[spikeCurrentFrame], NULL, &dstRect);
                    break;
                case '|':
                    SDL_RenderCopy(renderer, barTexture, NULL, &dstRect);
                    break;
                case '-':
                    SDL_RenderCopy(renderer, horizontalWallTexture, NULL, &dstRect);
                    break;
                case ']':
                    SDL_RenderCopy(renderer, cornerRightTexture, NULL, &dstRect);
                    break;
                case ':':
                    SDL_RenderCopy(renderer, cornerLeftTexture, NULL, &dstRect);
                    break;
                case ';':
                    SDL_RenderCopy(renderer, bottomLeftTexture, NULL, &dstRect);
                    break;
                case '.':
                    SDL_RenderCopy(renderer, floorTexture, NULL, &dstRect);
                    break;
                case '=':
                    SDL_RenderCopy(renderer, bottomWallTexture, NULL, &dstRect);
                    break;
                case '(':
                    SDL_RenderCopy(renderer, leftParenthesisTexture, NULL, &dstRect);
                    break;
                case ')':
                    SDL_RenderCopy(renderer, rightParenthesisTexture, NULL, &dstRect);
                    break;
            }
        }
    }

    // Рендер кнопки
        renderButton();

    // Рендер Pac-Man 1
    SDL_Rect dstRect1 = { (int)(pacman1X * TEXTURE_SIZE), (int)(pacman1Y * TEXTURE_SIZE), TEXTURE_SIZE, TEXTURE_SIZE };

if (isPacmanAttacking) {
    // Если атакует, используем текстуры удара
    SDL_RenderCopy(renderer, pacmanAttackTextures[pacmanAttackCurrentFrame], NULL, &dstRect1);
} else if (pacman1SpeedX != 0 || pacman1SpeedY != 0) {
    // Если двигается, используем текстуры движения
    SDL_RenderCopy(renderer, pacmanRunTextures[currentFrame], NULL, &dstRect1);
} else {
    // Иначе отображаем текстуры покоя
    SDL_RenderCopy(renderer, pacmanIdleTextures[pacmanIdleCurrentFrame], NULL, &dstRect1);
}



    // Рендер Pac-Man 2
    SDL_Rect dstRect2 = { (int)(pacman2X * TEXTURE_SIZE), (int)(pacman2Y * TEXTURE_SIZE), TEXTURE_SIZE, TEXTURE_SIZE };
if (pacman2SpeedX != 0 || pacman2SpeedY != 0) {
    SDL_RenderCopy(renderer, pacman2RunTextures[pacman2CurrentFrame], NULL, &dstRect2);
} else {
    SDL_RenderCopy(renderer, pacman2IdleTextures[pacman2IdleCurrentFrame], NULL, &dstRect2);
}


    // Рендер скелета
    if (isButtonPressed && isGhostAlive) {
        for (int i = 0; i < MAX_GHOSTS; i++) { // Объявляем переменную i
        if (!ghosts[i].isAlive) continue;
        SDL_Rect dstRectGhost = { (int)(ghosts[i].x * TEXTURE_SIZE), (int)(ghosts[i].y * TEXTURE_SIZE), TEXTURE_SIZE, TEXTURE_SIZE };
        SDL_RenderCopy(renderer, ghostRunTextures[ghosts[i].currentFrame], NULL, &dstRectGhost);
    }
    }

    SDL_RenderPresent(renderer);
}
    

void updateGhostDamageAnimation() {
    int currentTime = SDL_GetTicks();

    for (int i = 0; i < MAX_GHOSTS; i++) {
        if (!ghosts[i].isAlive || !ghosts[i].isTakingDamage) continue;

        if (currentTime - ghosts[i].lastDamageTime >= ghostDamageFrameDelay) {
            ghosts[i].damageFrame++; // Переключаем кадр анимации
            ghosts[i].lastDamageTime = currentTime;

            if (ghosts[i].damageFrame >= ghostDamageFrameCount) {
                ghosts[i].damageFrame = 0; // Сбрасываем кадр анимации
                ghosts[i].isTakingDamage = 0; // Завершаем анимацию урона
            }
        }
    }
}

void updatePacman1Animation() {
    int currentTime = SDL_GetTicks();

    if (pacman1SpeedX != 0 || pacman1SpeedY != 0) {
        // Если двигается, переключаем анимацию бега
        if (currentTime - pacmanLastFrameTime >= pacmanFrameDelay) {
            currentFrame = (currentFrame + 1) % gifFrameCount;
            pacmanLastFrameTime = currentTime;
        }
    } else {
        // Если не двигается, переключаем анимацию покоя
        if (currentTime - pacmanIdleLastFrameTime >= pacmanIdleFrameDelay) {
            pacmanIdleCurrentFrame = (pacmanIdleCurrentFrame + 1) % pacmanIdleFrameCount;
            pacmanIdleLastFrameTime = currentTime;
        }
    }
}



void updatePacman2Animation() {
    int currentTime = SDL_GetTicks();

    if (pacman2SpeedX != 0 || pacman2SpeedY != 0) {
        // Анимация бега
        if (currentTime - pacman2LastFrameTime >= pacman2FrameDelay) {
            pacman2CurrentFrame = (pacman2CurrentFrame + 1) % pacman2RunFrameCount;
            pacman2LastFrameTime = currentTime;
        }
    } else {
        // Анимация покоя
        if (currentTime - pacman2IdleLastFrameTime >= pacman2IdleFrameDelay) {
            pacman2IdleCurrentFrame = (pacman2IdleCurrentFrame + 1) % pacman2IdleFrameCount;
            pacman2IdleLastFrameTime = currentTime;
        }
    }
}





void updatePacman1Position() {
    pacman1X += pacman1SpeedX;
    pacman1Y += pacman1SpeedY;

    // Ограничение с увеличенным радиусом блокировки
    if (pacman1X * TEXTURE_SIZE < BORDER_OFFSET) pacman1X = BORDER_OFFSET / TEXTURE_SIZE; // Левая граница
    if (pacman1Y * TEXTURE_SIZE < BORDER_OFFSET) pacman1Y = BORDER_OFFSET / TEXTURE_SIZE; // Верхняя граница
    if ((pacman1X + 1) * TEXTURE_SIZE > WINDOW_WIDTH - BORDER_OFFSET)
        pacman1X = (WINDOW_WIDTH - BORDER_OFFSET) / TEXTURE_SIZE - 1; // Правая граница
    if ((pacman1Y + 1) * TEXTURE_SIZE > WINDOW_HEIGHT - BORDER_OFFSET)
        pacman1Y = (WINDOW_HEIGHT - BORDER_OFFSET) / TEXTURE_SIZE - 1; // Нижняя граница
}

void updatePacman2Position() {
    pacman2X += pacman2SpeedX;
    pacman2Y += pacman2SpeedY;

    // Ограничение с увеличенным радиусом блокировки
    if (pacman2X * TEXTURE_SIZE < BORDER_OFFSET) pacman2X = BORDER_OFFSET / TEXTURE_SIZE; // Левая граница
    if (pacman2Y * TEXTURE_SIZE < BORDER_OFFSET) pacman2Y = BORDER_OFFSET / TEXTURE_SIZE; // Верхняя граница
    if ((pacman2X + 1) * TEXTURE_SIZE > WINDOW_WIDTH - BORDER_OFFSET)
        pacman2X = (WINDOW_WIDTH - BORDER_OFFSET) / TEXTURE_SIZE - 1; // Правая граница
    if ((pacman2Y + 1) * TEXTURE_SIZE > WINDOW_HEIGHT - BORDER_OFFSET)
        pacman2Y = (WINDOW_HEIGHT - BORDER_OFFSET) / TEXTURE_SIZE - 1; // Нижняя граница
}

void handleKeyDown1(SDL_Keycode key) {
    switch (key) {
        case SDLK_w: pacman1SpeedY = -0.1f; break;
        case SDLK_s: pacman1SpeedY = 0.1f; break;
        case SDLK_a: pacman1SpeedX = -0.1f; break;
        case SDLK_d: pacman1SpeedX = 0.1f; break;
    }
}

void handleKeyDown2(SDL_Keycode key) {
    switch (key) {
        case SDLK_UP: pacman2SpeedY = -0.1f; break;
        case SDLK_DOWN: pacman2SpeedY = 0.1f; break;
        case SDLK_LEFT: pacman2SpeedX = -0.1f; break;
        case SDLK_RIGHT: pacman2SpeedX = 0.1f; break;
    }
}

void handleKeyUp1(SDL_Keycode key) {
    switch (key) {
        case SDLK_w:
        case SDLK_s:
            pacman1SpeedY = 0.0f; // Сброс вертикальной скорости
            break;

        case SDLK_a:
        case SDLK_d:
            pacman1SpeedX = 0.0f; // Сброс горизонтальной скорости
            break;

        case SDLK_c: // Нажатие 'C' для удара
    if (!isPacmanAttacking) { // Если атака не активна
        isPacmanAttacking = 1; // Устанавливаем флаг атаки
        pacmanAttackCurrentFrame = 0; // Сбрасываем на первый кадр анимации
        pacmanAttackLastFrameTime = SDL_GetTicks(); // Устанавливаем текущее время

        printf("Pacman1 started attack animation.\n"); // Отладочная информация

        // Проверяем, есть ли рядом враги
        for (int i = 0; i < MAX_GHOSTS; i++) {
            if (!ghosts[i].isAlive) continue; // Пропускаем мертвых призраков

            if (isNear(pacman1X, pacman1Y, ghosts[i].x, ghosts[i].y)) {
                ghosts[i].hits++;
                if (ghosts[i].hits >= ghostMaxHits) {
                    ghosts[i].isAlive = 0; // Убиваем призрака
                    printf("Ghost %d killed.\n", i);
                } else {
                    ghosts[i].isTakingDamage = 1; // Активируем анимацию урона
                    ghosts[i].damageFrame = 0; // Сбрасываем на первый кадр урона
                    ghosts[i].lastDamageTime = SDL_GetTicks(); // Устанавливаем текущее время
                }
            }
        }
    }
    break;

        default:
            break; // Остальные клавиши не обрабатываются
    }
}




void handleKeyUp2(SDL_Keycode key) {
    switch (key) {
        case SDLK_UP:
        case SDLK_DOWN:
            pacman2SpeedY = 0.0f; // Сброс вертикальной скорости
            break;
        case SDLK_LEFT:
        case SDLK_RIGHT:
            pacman2SpeedX = 0.0f; // Сброс горизонтальной скорости
            break;
    }
}


int isColliding(float objX, float objY, int targetX, int targetY, int size) {
    // Проверяем пересечение прямоугольников (игрока и кнопки)
    SDL_Rect objRect = { (int)(objX * TEXTURE_SIZE), (int)(objY * TEXTURE_SIZE), TEXTURE_SIZE, TEXTURE_SIZE };
    SDL_Rect targetRect = { targetX * TEXTURE_SIZE, targetY * TEXTURE_SIZE, size, size };

    return SDL_HasIntersection(&objRect, &targetRect);
}


void checkButtonPress() {
    if (isColliding(pacman1X, pacman1Y, buttonX, buttonY, TEXTURE_SIZE) &&
        isColliding(pacman2X, pacman2Y, buttonX, buttonY, TEXTURE_SIZE)) {
        if (!isButtonPressed) { // Обновляем только при первом нажатии
            isButtonPressed = 1;
            printf("Button pressed! Game activated.\n"); // Отладка
        }
    }
}




void renderButton() {
    SDL_Rect buttonRect = { buttonX * TEXTURE_SIZE, buttonY * TEXTURE_SIZE, 91, TEXTURE_SIZE };

    if (isButtonPressed) {
        SDL_RenderCopy(renderer, buttonPressedTexture, NULL, &buttonRect);
    } else {
        SDL_RenderCopy(renderer, buttonTexture, NULL, &buttonRect);
    }
}







int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

window = SDL_CreateWindow("Pacman", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * 40, HEIGHT * 40, SDL_WINDOW_FULLSCREEN);
//
//     window = SDL_CreateWindow("Pacman", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080, SDL_WINDOW_FULLSCREEN_DESKTOP);
// if (!window) {
//     fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
//     return -1;
// }

// Создаём рендерер
renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);


// Настраиваем масштабирование
SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear"); // Линейное сглаживание
SDL_RenderSetLogicalSize(renderer, WIDTH * TEXTURE_SIZE, HEIGHT * TEXTURE_SIZE);
//

    // Загрузка текстур для анимаций
    if (loadGifAnimation("animation1.gif", &pacmanRunTextures, &gifFrameCount) != 0) {
        fprintf(stderr, "Failed to load run animation.\n");
        return -1;
    }
    if (loadGifAnimation("pacman1.gif", &pacmanIdleTextures, &pacmanIdleFrameCount) != 0) {
        fprintf(stderr, "Failed to load pacman1 idle animation.\n");
        return -1;
    }
    if (loadGhostGifAnimation("skeleton.gif", &ghostRunTextures, &ghostGifFrameCount) != 0) {
        fprintf(stderr, "Failed to load ghost run animation.\n");
        return -1;
    }
    if (loadGifAnimation("ships.gif", &spikeTextures, &spikeFrameCount) != 0) {
        fprintf(stderr, "Failed to load spike animation.\n");
        return -1;
    }
    if (loadGifAnimation("animation.gif", &pacman2RunTextures, &pacman2RunFrameCount) != 0) {
        fprintf(stderr, "Failed to load pacman2 run animation.\n");
        return -1;
    }
    if (loadGifAnimation("pacman2.gif", &pacman2IdleTextures, &pacman2IdleFrameCount) != 0) {
        fprintf(stderr, "Failed to load pacman2 idle animation.\n");
        return -1;
    }
    
    // Загрузка текстур для символов
    barTexture = IMG_LoadTexture(renderer, "levayastena.png");
    if (!barTexture) {
        fprintf(stderr, "Failed to load bar texture. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    horizontalWallTexture = IMG_LoadTexture(renderer, "floor.png");
    if (!horizontalWallTexture) {
        fprintf(stderr, "Failed to load horizontal wall texture. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    cornerLeftTexture = IMG_LoadTexture(renderer, "ugol.png");
    if (!cornerLeftTexture) {
        fprintf(stderr, "Failed to load corner left texture. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    cornerRightTexture = IMG_LoadTexture(renderer, "pravayastena.png");
    if (!cornerRightTexture) {
        fprintf(stderr, "Failed to load corner right texture. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    bottomLeftTexture = IMG_LoadTexture(renderer, "nizhnelevyy.png");
    if (!bottomLeftTexture) {
        fprintf(stderr, "Failed to load bottom left texture. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    floorTexture = IMG_LoadTexture(renderer, "pol.png");
    if (!floorTexture) {
        fprintf(stderr, "Failed to load floor texture. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    bottomWallTexture = IMG_LoadTexture(renderer, "niz.png");
    if (!bottomWallTexture) {
        fprintf(stderr, "Failed to load bottom wall texture. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    leftParenthesisTexture = IMG_LoadTexture(renderer, "nizhnepr.png");
    if (!leftParenthesisTexture) {
        fprintf(stderr, "Failed to load left parenthesis texture. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    rightParenthesisTexture = IMG_LoadTexture(renderer, "ugolpr.png");
    if (!rightParenthesisTexture) {
        fprintf(stderr, "Failed to load right parenthesis texture. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }



    if (loadGifAnimation("pacman1atack.gif", &pacmanAttackTextures, &pacmanAttackFrameCount) != 0) {
    fprintf(stderr, "Failed to load pacman1 attack animation.\n");
    return -1;
    }


    if (loadGifAnimation("uronskeletu.gif", &ghostDamageTextures, &ghostDamageFrameCount) != 0) {
    fprintf(stderr, "Failed to load ghost damage animation.\n");
    return -1;
    } else {
    printf("Ghost damage animation loaded successfully. Frames: %d\n", ghostDamageFrameCount);
    }
    
    buttonTexture = IMG_LoadTexture(renderer, "knopka.png");
    if (!buttonTexture) {
    fprintf(stderr, "Failed to load knopka.png. SDL_Error: %s\n", SDL_GetError());
    } else {
    printf("knopka.png loaded successfully.\n");
    }
    
    buttonPressedTexture = IMG_LoadTexture(renderer, "knopka2.png");
    if (!buttonPressedTexture) {
        fprintf(stderr, "Failed to load knopka2.png. SDL_Error: %s\n", SDL_GetError());
    } else {
        printf("knopka2.png loaded successfully.\n");
    }   




for (int i = 0; i < MAX_GHOSTS; i++) {
    ghosts[i].x = rand() % WIDTH;
    ghosts[i].y = rand() % HEIGHT;
    ghosts[i].isAlive = 1;
    ghosts[i].currentFrame = 0;
    ghosts[i].lastFrameTime = 0;
    ghosts[i].hits = 0;
    ghosts[i].isTakingDamage = 0;  // Изначально урон не наносится
    ghosts[i].damageFrame = 0;    // Начальный кадр анимации урона
    ghosts[i].lastDamageTime = 0; // Время обновления урона
}


while (!quit) {
    int currentTime = SDL_GetTicks();

    // Проверяем, была ли нажата кнопка
    checkButtonPress();

    if (isButtonPressed) {
        updateGhostDamageAnimation();

        if (currentTime - ghostLastFrameTime >= ghostFrameDelay) {
            for (int i = 0; i < MAX_GHOSTS; i++) {
                if (ghosts[i].isAlive) {
                    ghosts[i].currentFrame = (ghosts[i].currentFrame + 1) % ghostGifFrameCount;
                }
            }
            ghostLastFrameTime = currentTime;
        }
    }

    // Обновляем позиции Pac-Man
    updatePacman1Position();
    updatePacman2Position();
    updatePacman1Animation();
    updatePacman2Animation();

    // Обновляем анимацию Pac-Man 1
    if (currentTime - pacmanLastFrameTime >= pacmanFrameDelay) {
        currentFrame = (currentFrame + 1) % gifFrameCount;
        pacmanLastFrameTime = currentTime;
    }

    // Обновляем анимацию шипов
    if (currentTime - spikeLastFrameTime >= spikeFrameDelay) {
        spikeCurrentFrame = (spikeCurrentFrame + 1) % spikeFrameCount;
        spikeLastFrameTime = currentTime;
    }

    // Обновляем анимацию удара Pac-Man
    if (isPacmanAttacking) {
    if (currentTime - pacmanAttackLastFrameTime >= pacmanAttackFrameDelay) {
        pacmanAttackCurrentFrame++; // Переход к следующему кадру
        pacmanAttackLastFrameTime = currentTime;

        
        if (pacmanAttackCurrentFrame >= pacmanAttackFrameCount) {
            pacmanAttackCurrentFrame = 0;
            isPacmanAttacking = 0; // Завершаем атаку
        }
    }
}


    // Отрисовка игрового поля
    draw_field();

    // Отрисовка скелетов только если кнопка нажата
    if (isButtonPressed) {
        renderGhosts();
    }

    // Обработка событий
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = 1;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_q) {
                quit = 1; // Завершаем игру при нажатии 'q'
            } else {
                handleKeyDown1(e.key.keysym.sym);
                handleKeyDown2(e.key.keysym.sym);
            }
        } else if (e.type == SDL_KEYUP) {
            handleKeyUp1(e.key.keysym.sym);
            handleKeyUp2(e.key.keysym.sym);
        }
    }

    usleep(DELAY); // Для плавности
}


    
    // Освобождение памяти
    for (int i = 0; i < gifFrameCount; ++i) {
        SDL_DestroyTexture(pacmanRunTextures[i]);
    }
    free(pacmanRunTextures);

    for (int i = 0; i < pacmanIdleFrameCount; ++i) {
        SDL_DestroyTexture(pacmanIdleTextures[i]);
    }
    free(pacmanIdleTextures);

    for (int i = 0; i < ghostGifFrameCount; ++i) {
        SDL_DestroyTexture(ghostRunTextures[i]);
    }
    free(ghostRunTextures);

    for (int i = 0; i < spikeFrameCount; ++i) {
        SDL_DestroyTexture(spikeTextures[i]);
    }
    free(spikeTextures);

    for (int i = 0; i < pacman2RunFrameCount; ++i) {
        SDL_DestroyTexture(pacman2RunTextures[i]);
    }
    free(pacman2RunTextures);

    for (int i = 0; i < pacman2IdleFrameCount; ++i) {
        SDL_DestroyTexture(pacman2IdleTextures[i]);
    }
    for (int i = 0; i < pacmanAttackFrameCount; ++i) {
        SDL_DestroyTexture(pacmanAttackTextures[i]);
}
free(pacmanAttackTextures);
    free(pacman2IdleTextures);
    SDL_DestroyTexture(buttonPressedTexture);
    SDL_DestroyTexture(buttonTexture);
    SDL_DestroyTexture(barTexture);
    SDL_DestroyTexture(horizontalWallTexture);
    SDL_DestroyTexture(cornerLeftTexture);
    SDL_DestroyTexture(cornerRightTexture);
    SDL_DestroyTexture(bottomLeftTexture);
    SDL_DestroyTexture(floorTexture);
    SDL_DestroyTexture(bottomWallTexture);
    SDL_DestroyTexture(leftParenthesisTexture);
    SDL_DestroyTexture(rightParenthesisTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}