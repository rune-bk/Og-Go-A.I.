#ifndef __GOBOARD_H__
#define __GOBOARD_H__

#include "GoPoint.h"
#include "GoMove.h"
#include "Config.h"
#include "GoState.h"
#include "GoBlock.h"
#include <list>

typedef std::list<GoBlock*> BlockList;
typedef std::list<GoBlock*>::iterator BlockListIterator;

class GoBoard
{
public:
	GoBoard(int size = BOARD_DEFAULT_SIZE);
	virtual ~GoBoard();

		const int Size() const; ///< Returns board size. Ex: 19x19 board will return 19
		const bool Occupied(const GoPoint p) const; ///< Returns true if p is occupied on the board
		const bool Occupied(const int p) const; ///< Returns true if p is occupied on the board
		const bool IsEmpty(GoPoint p) const; ///< Returns !Occupied(p)
		const bool IsBorder(int pos) const; ///< Checks if pos is a border position on the board. Used to count liberties
		const bool IsCorner(int pos) const; ///< Checks if pos is a corner position on the board. Used to count liberties
		bool IsColor(GoPoint p, int c) const; ///< Checks a point for a given color
		static const int CurrentPlayer(); ///< Returns the current player color
		int NextPlayer() const; ///< Inverted CurrentPlayer

		int North(const GoPoint p) const; ///< Returns the position north of p
		int South(const GoPoint p) const; ///< Returns the position south of p
		int West(const GoPoint p) const; ///< Returns the position west of p
		int East(const GoPoint p) const; ///< Returns the position east of p
		int North(const int) const; ///< Returns the position north of p
		int South(const int) const; ///< Returns the position south of p
		int West(const int) const; ///< Returns the position west of p
		int East(const int) const; ///< Returns the position east of p
		int Pos(const GoPoint p) const; ///< Returns the numerical position in the 1D array of point p

		bool IsSuicide(const GoPoint p) const; ///< Checks if playing on point p is a suicide
		bool IsRealPoint(const GoPoint p) const; ///< Checks point validity
		bool IsRealPoint(const int p) const; ///< Checks point validity

		bool IsLegal(const GoPoint& p, int color); ///< Checks if playing p is legal for color

		const bool IsLibertyOfBlock(const int point, const int anchor) const; ///< Checks if a given point is a liberty for the block the anchor point belongs to.
		const int FindCommonLiberties(const int point, const int anchor) const; ///<Used when joining a single stone to a block
		const int FindUniqueLiberties(const int stone, const GoBlock* block) const; ///< Counts unique liberties for stone compared to block
		void UpdateBlocks(const GoPoint p); ///< Update block liberties, stones and do cleanup of dead blocks
		void KillSurroundingDeadBlocks(const int p); ///< Checks all blocks surrounding p, remove them if liberties == 0

		const bool CapturedStones() const; ///< Returns captured stones for this round
		const std::list<GoPoint> GetCapturedStones() const; ///< Returns a list of captured stones


		const int Liberties(const GoPoint p) const; ///< Finds number of liberties for all stones in the same group as p
		const int Liberties(int pos) const; ///< Finds number of liberties for all stones in the same group as p
		bool InAtari(GoPoint p) const; ///< Checks if point p is in atari(1 liberty)

		const GoMove* Play(GoPoint p, int color); ///< Attempt to play p with given color
		const GoMove* Play(GoPoint p); ///< Attempt to play p. Color is stored in p.

		void AddStone(int point, int color); ///< Add stone with color to board placed on point
		void RemoveStone(const int pos); ///< Remove stone at pos from the board
		void UpdateBlocks(int point, int color); ///< Update blocks based on placing a stone with the given parameters

		void DisplayCurrentState() const; ///< Used by GTP showboard command to output the current state to console

		GoState State; ///< Struct containing the most basic board information
		BlockList blocks; ///< List of active blocks on the board
		
private:
	int BoardSize; ///< Boardsize (19x19 will be stored as 19 here)
	
};




#endif //#ifndef __GOBOARD_H__
