#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

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

typedef struct {
	bool hasStarted;
	float paddleX;
	Vector2 ballPos;
	Vector2 ballVel;
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
				exit(0);
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
const int PADDLE_SPEED = 200;
const int PADDLE_Y_OFFSET = 16;
const int BALL_RADIUS = 8;
const int BALL_SPEED = 300;

void playingEntry(Game *game) {
	PlayingData *data = game->currentState->data;
	data->hasStarted = false;
	data->paddleX = game->width/2. - PADDLE_WIDTH/2.;
	data->ballVel = (Vector2){ 0, 0 };
	data->ballPos = (Vector2){ game->width/2., game->height - PADDLE_Y_OFFSET - PADDLE_HEIGHT - BALL_RADIUS - 4 };
}
void playingExit(Game *game) {}

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

	data->paddleX += movement * PADDLE_SPEED * game->delta;

	const int LEFT_LIMIT  = 8;
	const int RIGHT_LIMIT = game->width-8-PADDLE_WIDTH;
	if (data->paddleX < LEFT_LIMIT)  data->paddleX = LEFT_LIMIT;
	if (data->paddleX > RIGHT_LIMIT) data->paddleX = RIGHT_LIMIT; 

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
		data->ballPos.y = game->height-BALL_RADIUS;
		data->ballVel.y*=-1;
	}
}

void playingDraw(Game *game) {
	ClearBackground(RAYWHITE);

	PlayingData *data = game->currentState->data;

	DrawRectangle(data->paddleX, game->height-PADDLE_Y_OFFSET-PADDLE_HEIGHT, PADDLE_WIDTH, PADDLE_HEIGHT, GRAY);

	DrawEllipse(data->ballPos.x, data->ballPos.y, BALL_RADIUS, BALL_RADIUS, LIGHTGRAY);
}

void deadEntry(Game *game) {}
void deadExit(Game *game) {}
void deadUpdate(Game *game) {}
void deadDraw(Game *game) {}

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
		.paddleX = game.width/2. - PADDLE_WIDTH/2.,
		.ballVel = { 0, 0 },
		.ballPos = { game.width/2., game.height - PADDLE_Y_OFFSET - PADDLE_HEIGHT - BALL_RADIUS - 4 }
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
