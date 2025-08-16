#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef enum {
	STATE_MENU,
	STATE_PLAYING,
	STATE_DEAD,
	STATE_COUNT
} StateType;

struct Game;

typedef struct {
	StateType type;
	void (*entry)  (struct Game *game);
	void (*exit)   (struct Game *game);
	void (*update) (struct Game *game);
	void (*draw)   (struct Game *game);
	void *data;
} State;

typedef struct {
	int selectedOption;
	int optionCount;
	const char **options;
} MenuData;

#define BLOCK_ROWS 5
#define BLOCK_COLS 10
#define BLOCK_PADDING 4

typedef struct {
	Vector2 pos;
	Vector2 size;
	bool active;
} Block;

typedef struct {
	bool hasStarted;
	int lifes;

	Vector2 paddlePos;
	Vector2 ballPos;
	Vector2 ballVel;

	int blockCount;
	Block blocks[BLOCK_ROWS * BLOCK_COLS];
} PlayingData;

typedef struct {

} DeadData;

typedef struct Game {
	int width;
	int height;

	float delta;

	State* states[STATE_COUNT];
	State* currentState;

	StateType nextState;
} Game;

void menuEntry(Game *game) {}
void menuExit(Game *game) {}
void menuUpdate(Game *game) {
	MenuData *data = (MenuData*)game->currentState->data;
	if (IsKeyPressed(KEY_DOWN)) {
		data->selectedOption = (data->selectedOption+1)%data->optionCount;
	} else if (IsKeyPressed(KEY_UP)) {
		data->selectedOption = (data->selectedOption-1+data->optionCount)%data->optionCount;
	} else if (IsKeyPressed(KEY_C)) {
		switch (data->selectedOption) {
			case 0:
				game->nextState=STATE_PLAYING;
				break;
			case 1:
				CloseWindow();
				return;
		}
	}
}

void menuDraw(Game *game) {
	ClearBackground(RAYWHITE);

	const char *title = "Breakout";
	const int titleFontSize = 32;
	const int titleWidth = MeasureText(title, titleFontSize);
	DrawText(title, game->width/2-titleWidth/2, game->height*0.4, titleFontSize, GRAY);

	MenuData *data = (MenuData*)game->currentState->data;

	const int optionFontSize = 16;

	for (int i = 0; i < data->optionCount; i++) {
		const int optionWidth = MeasureText(data->options[i], optionFontSize);
		const float x = game->width/2. - optionWidth/2.;
		const float y = game->height/2. + optionFontSize*1.5 * i;
		DrawText(data->options[i], x, y, optionFontSize, LIGHTGRAY);

		if (data->selectedOption == i) {
			DrawRectangle(x-4, y+optionFontSize+3, optionWidth+8, 1, LIGHTGRAY);
		}
	}
}

const int PADDLE_WIDTH = 96;
const int PADDLE_HEIGHT = 16;
const int PADDLE_SPEED = 300;
const int PADDLE_Y_OFFSET = 16;
const int BALL_RADIUS = 8;
const int BALL_SPEED = 300;

void resetBall(Game *game) {
	PlayingData *data = game->currentState->data;
	data->ballVel = (Vector2){ 0, 0 };
	data->ballPos = (Vector2){ game->width/2., game->height - PADDLE_Y_OFFSET - PADDLE_HEIGHT - BALL_RADIUS - 4 };
}

void playingEntry(Game *game) {
	PlayingData *data = game->currentState->data;
	data->hasStarted = false;
	data->lifes = 3;

	data->paddlePos = (Vector2){game->width/2. - PADDLE_WIDTH/2., game->height-PADDLE_Y_OFFSET-PADDLE_HEIGHT};
	resetBall(game);

	int blockWidth  = (game->width  - (BLOCK_COLS+1)   * BLOCK_PADDING) / BLOCK_COLS;
	int blockHeight = (game->height/3 - (BLOCK_ROWS+1) * BLOCK_PADDING) / BLOCK_ROWS;

	data->blockCount = 0;
	for (int row = 0; row < BLOCK_ROWS; row++) {
		for (int col = 0; col < BLOCK_COLS; col++) {
			Block *b = &data->blocks[data->blockCount++];
			b->pos.x = BLOCK_PADDING + col * (blockWidth  + BLOCK_PADDING);
			b->pos.y = BLOCK_PADDING + row * (blockHeight + BLOCK_PADDING);

			b->size.x = blockWidth;
			b->size.y = blockHeight;

			b->active = true;
		}
	}
}
void playingExit(Game *game) {}

bool aab(Vector2 aPos, Vector2 aSize, Vector2 bPos, Vector2 bSize) {
	return
		aPos.x+aSize.x > bPos.x &&
		aPos.x-aSize.x < bPos.x + bSize.x &&
		aPos.y+aSize.y > bPos.y &&
		aPos.y-aSize.y < bPos.y + bSize.y;
}

void playingUpdate(Game *game) {
	PlayingData *data = game->currentState->data;

	if (!data->hasStarted) {
		if (IsKeyPressed(KEY_C)) {
			data->hasStarted = true;
			data->ballVel = (Vector2){ 1, 1 };
		}
		else { return; }
	}

	int movement = 0;
	if (IsKeyDown(KEY_LEFT))  movement -= 1;
	if (IsKeyDown(KEY_RIGHT)) movement += 1;

	data->paddlePos.x += movement * PADDLE_SPEED * game->delta;

	const int LEFT_LIMIT  = 8;
	const int RIGHT_LIMIT = game->width-8-PADDLE_WIDTH;
	if (data->paddlePos.x < LEFT_LIMIT)  data->paddlePos.x = LEFT_LIMIT;
	if (data->paddlePos.x > RIGHT_LIMIT) data->paddlePos.x = RIGHT_LIMIT; 

	data->ballPos.x += data->ballVel.x * BALL_SPEED * game->delta;
	data->ballPos.y += data->ballVel.y * BALL_SPEED * game->delta;

	if (data->ballPos.x < BALL_RADIUS) {
		data->ballPos.x = BALL_RADIUS;
		data->ballVel.x*=-1;
	}
	else if (data->ballPos.x > game->width-BALL_RADIUS) {
		data->ballPos.x = game->width-BALL_RADIUS;
		data->ballVel.x*=-1;
	}

	if (data->ballPos.y < BALL_RADIUS) {
		data->ballPos.y = BALL_RADIUS;
		data->ballVel.y*=-1;
	}
	else if (data->ballPos.y > game->height-BALL_RADIUS) {
		data->lifes -= 1;
		if (data->lifes <= 0) {
			game->nextState = STATE_DEAD;
			return;
		};
		resetBall(game);
		data->hasStarted = false;
	}

	if (aab(data->ballPos, (Vector2){BALL_RADIUS, BALL_RADIUS}, data->paddlePos, (Vector2){PADDLE_WIDTH, PADDLE_HEIGHT})) {
		float deltaX = fabs(data->ballPos.x - data->paddlePos.x + PADDLE_WIDTH/2.);
		float deltaY = fabs(data->ballPos.y - data->paddlePos.y + PADDLE_HEIGHT/2.);

		if (deltaX > deltaY) {
			if (data->ballVel.y > 0) data->ballPos.y = data->paddlePos.y - BALL_RADIUS;
			else data->ballPos.y = data->paddlePos.y + PADDLE_HEIGHT + BALL_RADIUS;
			data->ballVel.y *= -1;
		} else {
			if (data->ballVel.x > 0) data->ballPos.x = data->paddlePos.x - BALL_RADIUS;
			else data->ballPos.x = data->paddlePos.x + PADDLE_WIDTH + BALL_RADIUS;
			data->ballVel.x *= -1;
		}
	}

	for (int i = 0; i < data->blockCount; i++) {
		Block *block = &data->blocks[i];
		if (!block->active) continue;
		if (!aab(data->ballPos, (Vector2){BALL_RADIUS, BALL_RADIUS}, block->pos, block->size)) continue;

		float overlapLeft  = fabs((data->ballPos.x + BALL_RADIUS) - block->pos.x);
		float overlapRight = fabs((block->pos.x + block->size.x) - (data->ballPos.x - BALL_RADIUS));

		float overlapTop    = fabs((data->ballPos.y + BALL_RADIUS) - block->pos.y);
		float overlapBottom = fabs((block->pos.y + block->size.y) - (data->ballPos.y - BALL_RADIUS));

		float minOverlap = fmin(fmin(overlapLeft, overlapRight), fmin(overlapTop, overlapBottom));

		if 			  (minOverlap == overlapLeft) {
			data->ballPos.x = block->pos.x - BALL_RADIUS;
			data->ballVel.x *= -1;
		} else if (minOverlap == overlapRight) {
			data->ballPos.x = block->pos.x + block->size.x + BALL_RADIUS;
			data->ballVel.x *= -1;
		} else if (minOverlap == overlapTop) {
			data->ballPos.y = block->pos.y - BALL_RADIUS;
			data->ballVel.y *= -1;
		} else if (minOverlap == overlapBottom) {
			data->ballPos.y = block->pos.y + block->size.y + BALL_RADIUS;
			data->ballVel.y *= -1;
		}
		block->active = false;
	}
}

void playingDraw(Game *game) {
	ClearBackground(RAYWHITE);

	PlayingData *data = game->currentState->data;

	const int LIFE_RADIUS = 8;
	const int LIFE_PADDING = 4;
	const int LIFE_Y = game->height-LIFE_RADIUS-LIFE_PADDING;

	for (int i = 0; i < data->lifes; i++) {
		DrawEllipse(
			LIFE_PADDING+LIFE_RADIUS+(i*(LIFE_RADIUS*2+LIFE_PADDING)),
			LIFE_Y,
			LIFE_RADIUS, LIFE_RADIUS,
			GRAY
		);
	}

	DrawRectangle(data->paddlePos.x, data->paddlePos.y, PADDLE_WIDTH, PADDLE_HEIGHT, GRAY);

	DrawEllipse(data->ballPos.x, data->ballPos.y, BALL_RADIUS, BALL_RADIUS, LIGHTGRAY);

	for (int i = 0; i < data->blockCount; i++) {
		Block block = data->blocks[i];
		if (!block.active) continue;
		DrawRectangle(block.pos.x, block.pos.y, block.size.x, block.size.y, GRAY);
	}
}

void deadEntry(Game *game) {}
void deadExit(Game *game) {}
void deadUpdate(Game *game) {}

void deadDraw(Game *game) {
	ClearBackground(RAYWHITE);
	DrawText("DEAD", 16, 16, 32, GRAY);
}

void changeState(Game *game, StateType newState) {
	if (game->currentState && game->currentState->exit) {
		game->currentState->exit(game);
	}
	game->currentState = game->states[newState];
	if (game->currentState->entry) {
		game->currentState->entry(game);
	}
}

int main() {
	Game game = {
		.width = 800, .height = 600,
		.currentState = NULL,
		.nextState = -1,
	};

	const char *menuOptions[] = { "Play", "Quit" };
	MenuData menuData = {
		.selectedOption = 0,
		.optionCount = 2,
		.options = menuOptions,
	};
	State menuState = { STATE_MENU,    menuEntry, menuExit, menuUpdate, menuDraw, &menuData};
	game.states[STATE_MENU] = &menuState;

	PlayingData playingData = {
		.hasStarted = false,
	};
	State playingState = { STATE_PLAYING, playingEntry, playingExit, playingUpdate, playingDraw, &playingData};
	game.states[STATE_PLAYING] = &playingState;

	DeadData deadData;
	State deadState = (State){ STATE_DEAD, deadEntry, deadExit, deadUpdate, deadDraw, &deadData};
	game.states[STATE_DEAD] = &deadState;


	changeState(&game, STATE_MENU);

	InitWindow(game.width, game.height, "Breakout");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		game.delta = GetFrameTime();

		if (game.currentState->update) game.currentState->update(&game);

		BeginDrawing();
		if (game.currentState->draw)   game.currentState->draw(&game);
		EndDrawing();

		if (game.nextState != -1) {
			changeState(&game, game.nextState);
			game.nextState = -1;
		}
	}

	return 0;
}
