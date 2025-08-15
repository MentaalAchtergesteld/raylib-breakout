#include <stdlib.h> 
#include <stdio.h> 
#include "raylib.h"

typedef enum {
	STATE_MENU,
	STATE_PLAYING,
	STATE_DEAD
} Scene;

typedef struct {
	int selectedOption;
	int optionCount;
	char **options;
} MenuData;

typedef struct {	
	int screenWidth;
	int screenHeight;
	Scene scene;
	Scene nextScene;
	void *sceneData;
} GameState;

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

MenuData* getDefaultMenuData(GameState *state) {
		MenuData *data = malloc(sizeof(MenuData));
		data->selectedOption = 0;
		data->optionCount = 2;
		data->options = malloc(data->optionCount * sizeof(char*));

		data->options[0] = "Play";
		data->options[1] = "Quit";

		return data;
}

void updateMenu(GameState *state) {
	MenuData *data = (MenuData *)state->sceneData;
	if (data == NULL) {
		printf("update");
		data = getDefaultMenuData(state);
		state->sceneData = data;
	}

	if (IsKeyPressed(KEY_DOWN)) {
		data->selectedOption = (data->selectedOption+1)%data->optionCount;
	} else if (IsKeyPressed(KEY_UP)) {
		data->selectedOption = (data->selectedOption-1+data->optionCount)%data->optionCount;
	} else if (IsKeyPressed(KEY_C)) {
		switchScene(state, STATE_PLAYING);
	}
}

void updatePlaying(GameState *state) {}
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

void drawMenu(GameState *state) {
	ClearBackground(RAYWHITE);

	const char *title = "Breakout";
	const int titleFontSize = 32;
	const int titleWidth = MeasureText(title, titleFontSize);
	DrawText(title, state->screenWidth/2-titleWidth/2, (float)state->screenHeight*0.4, titleFontSize, GRAY);

	MenuData *data = (MenuData *)state->sceneData;

	if (data == NULL) {
		data = getDefaultMenuData(state);
		state->sceneData = data;
	}

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

void drawPlaying(GameState *state) {
	ClearBackground(RAYWHITE);
	DrawText("Game", 0, 0, 32, GRAY);
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
