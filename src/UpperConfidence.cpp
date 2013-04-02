/*
 * UpperConfidence.cpp
 *
 *  Created on: Mar 16, 2013
 *      Author: rune
 */
#include <math.h>
#include "UpperConfidence.h"
#include "Log.h"
#include "GoBoard.h"
#include "PlayPolicy.h"
#include "SimpleRandomAI.h"
UpperConfidence::UpperConfidence(void):numRandMoves(20),numSimulations(1000),expRatio(1.5),workingBoard(0)
{

}

UpperConfidence::UpperConfidence(int randMoves, int numSim):numRandMoves(randMoves),numSimulations(numSim),expRatio(1.5),workingBoard(0)
{

}

UpperConfidence::UpperConfidence(std::vector<int> randMoves, int numSim):numRandMoves(0),numSimulations(numSim),expRatio(1.5),workingBoard(0)
{
	preselRandMoves = randMoves;
}

UpperConfidence::~UpperConfidence()
{
	if(workingBoard != 0)
		delete workingBoard;
}

std::vector<int> UpperConfidence::getPossibleMoves(int color , GoGame* game)
{
	LOG_VERBOSE << "Generating move list"<<std::endl;

	std::vector<int> moves = PlayPolicy().FindPossibleLocalMoves(game->Board);
	LOG_VERBOSE<< "Found "<<moves.size()<< " possible local moves"<<std::endl;
	//Add other moves not covered by plays
	for(int i = 0; i<numRandMoves;++i)
		moves.push_back(game->Board->Pos(SimpleRandomAI().generateMove(color,game)));

	LOG_VERBOSE<<"Generated random moves: "<<numRandMoves<<std::endl;
	return moves;
}
std::vector<UCBrow> UpperConfidence::generateUCBTable(int color, GoGame* game)
{
	LOG_DEBUG<<"Generating UCB table"<<std::endl;
	std::vector<UCBrow> ucbtable;
	LOG_VERBOSE<<"Began UCB"<<std::endl;
	workingBoard = new GoBoard(game->Board->Size());
	workingBoard->resetAndReplayMoves(game->Board);
	this->color = color;
	LOG_VERBOSE<<"SD: "<<color<<std::endl;
	this->game = game;
	LOG_VERBOSE<<"SD: "<<game<<std::endl;
	LOG_VERBOSE << "Generating UCB move for "<<color<<std::endl;
	//Generate possible moves
	moves = preselRandMoves;
	if(moves.size() == 0)
		moves = getPossibleMoves(color, game);

	float expected[moves.size()];
	int numPlayed[moves.size()];
	int totalNumPlayed = 0;

	int initialPlayNum = 1;
	LOG_VERBOSE << "Play all 1 time";
	//Play all moves "once"
	for(size_t j = 0; j<moves.size(); ++j)
	{
		numPlayed[j] = 0;
		expected[j] = 0;
		for(int i = 0; i<initialPlayNum; ++i)
		{
			float result = simulateMove(moves[j]);
			if(result > 0)
				result = 1;
			else
				result = 0;

			float oldWins = expected[j] * numPlayed[j];
			++numPlayed[j];
			++totalNumPlayed;
			expected[j] = ((float)(result+oldWins)/(float)numPlayed[j]);

		}
	}

	float maximisedVal = 0.0;
	int nextToPlay = 0;
	while(totalNumPlayed<numSimulations)
	{
		int firstClock = clock();
		//Maximise for all following plays
		for(size_t i = 0; i<moves.size(); ++i)
		{
			float ucbVal = expected[i] + sqrt( expRatio * log(totalNumPlayed) / numPlayed[i]);
			//          LOG_VERBOSE <<i<< " " <<expected[i]<< " "<< sqrt( expRatio * log(totalNumPlayed) / numPlayed[i]) << std::endl;
			if(ucbVal > maximisedVal)
			{
				maximisedVal = ucbVal;
				nextToPlay = i;
			}
		}
		//      LOG_VERBOSE <<nextToPlay<< " " <<expected[nextToPlay]<< " "<< sqrt( expRatio * log(totalNumPlayed) / numPlayed[nextToPlay]) << std::endl;
		//Play best move and update expected return
		float result = simulateMove(moves[nextToPlay]);
		if(result > 0)
			result = 1;
		else
			result = 0;
		++numPlayed[nextToPlay];
		++totalNumPlayed;

		if(totalNumPlayed%100==0)
			std::cerr<<"Simulated "<<totalNumPlayed<<" games "<<(firstClock - clock())<<std::endl;

		float oldWins = expected[nextToPlay] * numPlayed[nextToPlay];

		expected[nextToPlay] = ((float)(result+oldWins)/(float)numPlayed[nextToPlay]);

		maximisedVal = 0;
		nextToPlay = 0;

	}
	for(size_t i = 0; i<moves.size(); ++i)
	{
		UCBrow u;
		u.pos = moves[i];
		u.expected = expected[i];
		u.timesPlayed = numPlayed[i];
		ucbtable.push_back(u);
	}
	LOG_VERBOSE<<"Completed UCB table"<<std::endl;
	return ucbtable;
}
GoPoint UpperConfidence::generateMove(int color, GoGame* game)
{

	std::vector<UCBrow> ucb = generateUCBTable(color,game);
	//Select best move after search completion
	int bestMove = -1;
	float bestExpected = 0;
	for(size_t i = 0; i<ucb.size(); ++i)
	{
		LOG_VERBOSE<<game->Board->ReadablePosition(ucb[i].pos)<<" E:"<<ucb[i].expected<<" Played:"<<ucb[i].timesPlayed<<std::endl;
		if(ucb[i].expected>bestExpected)
		{
			LOG_VERBOSE<<"BE is now"<<game->Board->ReadablePosition(ucb[i].pos)<<std::endl;
			bestExpected = ucb[i].expected;
			bestMove = i;
		}
	}

	LOG_VERBOSE<<"Best move ("<<game->Board->ReadablePosition(ucb[bestMove].pos)<<") : "<<bestExpected<<std::endl;
	return game->Board->ReversePos(ucb[bestMove].pos,color);
}

const float UpperConfidence::simulateMove(int move)
{
	float expected = 0;
	workingBoard->resetAndReplayMoves(game->Board);
	workingBoard->Play(move, workingBoard->NextPlayer());
	SimpleRandomAI().simulateGame(workingBoard);
	float score = workingBoard->GetScore();
	if(color == S_BLACK && score>0)
		expected += score;
	else if(color == S_WHITE && score<0)
		expected -= score;
	return expected;
}
