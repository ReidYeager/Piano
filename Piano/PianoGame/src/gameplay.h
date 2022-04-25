
#ifndef PIANO_GAME_GAMEPLAY_H_
#define PIANO_GAME_GAMEPLAY_H_

void ShowMainMenu();
void ShowDifficultySelection();

enum GameStatesEnum
{
	Main_Menu = 0,
	Difficulty_Select = 1,
	Playing = 2
};

void UpdateInput();

void TransitionToState(GameStatesEnum _newState);

void StateUpdateMainMenu(float _delta);
void StateUpdateDifficultySelect(float _delta);
void StateUpdatePlaying(float _delta);

#endif // !PIANO_GAME_GAMEPLAY_H_
