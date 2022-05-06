
#ifndef PIANO_GAME_GAMEPLAY_H_
#define PIANO_GAME_GAMEPLAY_H_

void ShowMainMenu();
void ShowDifficultySelection();
void ShowSongSelection();

enum GameStatesEnum
{
	Main_Menu = 0,
	Difficulty_Select = 1,
	Playing = 2,
	Preplay = 3,
	Record = 4,
	Prerecord = 5,
	Song_Menu = 6
};

void UpdateInput();

void TransitionToState(GameStatesEnum _newState);

void StateUpdateMainMenu(float _delta);
void StateUpdateDifficultySelect(float _delta);
void StateUpdatePlaying(float _delta);
void StateUpdatePreplay(float _delta);
void StateUpdateRecord(float _delta);
void StateUpdatePrerecord(float _delta);
void StateUpdateSongMenu(float _delta);

#endif // !PIANO_GAME_GAMEPLAY_H_
