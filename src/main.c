#include <stdlib.h> 
#include <math.h> 
#include <stdio.h> 
#include "raylib.h"

typedef enum {
	STATE_MENU,
	STATE_PLAYING,
	STATE_DEAD
} Scene;

typedef struct {	
	float delta;

	int screenWidth;
	int screenHeight;
	Scene scene;
	Scene nextScene;
	void *sceneData;
} GameState;

void exitGame(GameState *state) {
	if (state->sceneData != NULL) {
		free(state->sceneData);
		state->sceneData = NULL;
	}

	CloseWindow();
}

void setSceneData(GameState *state, void* data) {
	if (state->sceneData != NULL) {
		free(state->sceneData);
		state->sceneData = NULL;
	}
	state->sceneData = data;
}

void switchScene(GameState *state, Scene newScene) {
	state->nextScene = newScene; 
}


typedef struct {
	int selectedOption;
	int optionCount;
	char **options;
} MenuData;

MenuData* getMenuData(GameState *state) {
	MenuData *data = (MenuData*)state->sceneData;
	if (data == NULL) {
		data = malloc(sizeof(MenuData));
		data->selectedOption = 0;
		data->optionCount = 2;
		data->options = malloc(data->optionCount * sizeof(char*));

		data->options[0] = "Play";
		data->options[1] = "Quit";
		state->sceneData = data;
	}
	return data;
}

void updateMenu(GameState *state) {
	MenuData *data = getMenuData(state);
	
	if (IsKeyPressed(KEY_DOWN)) {
		data->selectedOption = (data->selectedOption+1)%data->optionCount;
	} else if (IsKeyPressed(KEY_UP)) {
		data->selectedOption = (data->selectedOption-1+data->optionCount)%data->optionCount;
	} else if (IsKeyPressed(KEY_C)) {
		switch (data->selectedOption) {
			case 0:
				switchScene(state, STATE_PLAYING);
				break;
			case 1:
				exitGame(state);
				break;
		}
	}
}

void drawMenu(GameState *state) {
	ClearBackground(RAYWHITE);

	const char *title = "Breakout";
	const int titleFontSize = 32;
	const int titleWidth = MeasureText(title, titleFontSize);
	DrawText(title, state->screenWidth/2-titleWidth/2, (float)state->screenHeight*0.4, titleFontSize, GRAY);

	MenuData *data = getMenuData(state);

	const int optionFontSize = 16;

	for (int i = 0; i < data->optionCount; i++) {
		const int optionWidth = MeasureText(data->options[i], optionFontSize);
		const float x = (float)state->screenWidth/2 - (float)optionWidth/2;
		const float y = (float)state->screenHeight/2 + (float)optionFontSize*1.5 * i;
		DrawText(data->options[i], x, y, optionFontSize, LIGHTGRAY);

		if (data->selectedOption == i) {
			DrawRectangle(x-4, y+optionFontSize+3, optionWidth+8, 1, LIGHTGRAY);
		}
	}

}

const int PADDLE_WIDTH = 96;
const int PADDLE_HEIGHT = 24;
const int PADDLE_SPEED = 200;
const int PADDLE_Y_OFFSET = 16;
const int BALL_RADIUS = 12;
const int BALL_SPEED = 300;

typedef struct {
	bool hasStarted;
	float paddleX;
	Vector2 ballPos;
	Vector2 ballVel;
} PlayingData;

PlayingData* getPlayingData(GameState *state) {
	PlayingData* data = (PlayingData*)state->sceneData;
	if (data == NULL) {
		data = malloc(sizeof(PlayingData));
		data->hasStarted = false;

		data->paddleX = (float)state->screenWidth/2-(float)PADDLE_WIDTH/2;

		data->ballPos.x = (float)state->screenWidth/2;
		data->ballPos.y = state->screenHeight-PADDLE_Y_OFFSET-PADDLE_HEIGHT-BALL_RADIUS-4;

		data->ballVel.x = 0;
		data->ballVel.y = 1;

		state->sceneData = data;
	}

	return data;
}

void updatePlaying(GameState *state) {
	PlayingData* data = getPlayingData(state);

	if (!data->hasStarted) {
		if (IsKeyPressed(KEY_SPACE)) data->hasStarted = true;
		return;
	}

	int movement_vector = 0;
	if (IsKeyDown(KEY_LEFT))  movement_vector-=1;
	if (IsKeyDown(KEY_RIGHT))  movement_vector+=1;

	float movement = movement_vector * PADDLE_SPEED * state->delta;
	data->paddleX += movement;

	const int LEFT_BOUNDARY = 8;
	const int RIGHT_BOUNDARY = state->screenWidth-PADDLE_WIDTH-8;
	if (data->paddleX < LEFT_BOUNDARY)  data->paddleX = LEFT_BOUNDARY;
	if (data->paddleX > RIGHT_BOUNDARY) data->paddleX = RIGHT_BOUNDARY;


	data->ballPos.x += data->ballVel.x * BALL_SPEED * state->delta;
	data->ballPos.y += data->ballVel.y * BALL_SPEED * state->delta;
}

void drawPlaying(GameState *state) {
	ClearBackground(RAYWHITE);

	PlayingData* data = getPlayingData(state);

	DrawRectangle(
		data->paddleX, state->screenHeight-16-PADDLE_HEIGHT,
		PADDLE_WIDTH, PADDLE_HEIGHT, GRAY
	);

	DrawEllipse(data->ballPos.x, data->ballPos.y, BALL_RADIUS, BALL_RADIUS, LIGHTGRAY);
}


void updateDead(GameState *state) {}

void updateGame(GameState *state) {
	switch (state->scene) {
		case STATE_MENU:
			updateMenu(state);
			break;
		case STATE_PLAYING:
			updatePlaying(state);
			break;
		case STATE_DEAD:
			updateDead(state);
			break;
	}
}

void drawDead(GameState *state) {

}

void drawGame(GameState *state) {
	BeginDrawing();

	switch (state->scene) {
		case STATE_MENU:
			drawMenu(state);
			break;
		case STATE_PLAYING:
			drawPlaying(state);
			break;
		case STATE_DEAD:
			drawDead(state);
			break;
	}

	EndDrawing();
}

int main() {
	GameState state = {
		.screenWidth = 800,
		.screenHeight = 450,
		.scene = STATE_MENU,
		.nextScene = -1
	};

	InitWindow(state.screenWidth, state.screenHeight, "Breakout");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		state.delta = GetFrameTime();
		updateGame(&state);
		drawGame(&state);

		if (state.nextScene != -1) {
			state.scene = state.nextScene;
			state.nextScene = -1;

			free(state.sceneData);
			state.sceneData = NULL;
		}
	}

	return 0;
}
