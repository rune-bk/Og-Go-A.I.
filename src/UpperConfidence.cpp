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

UpperConfidence::UpperConfidence(int searchWidth, int searchDepth):searchDepth(searchDepth),searchWidth(searchWidth),expRatio(1.5)
{
}

UpperConfidence::~UpperConfidence()
{
}

GoPoint UpperConfidence::generateMove(int color, GoGame* game)
{
  this->color = color;
  this->game = game;
  LOG_VERBOSE << "Generating UCB move for "<<color;
  GoPoint moves[searchWidth];
  float expected[searchWidth];
  int numPlayed[searchWidth];
  int totalNumPlayed = 0;

  //Generate possible moves
  LOG_VERBOSE << "Generating move list";
  for(int j = 0; j<searchWidth; ++j)
    {
      moves[j] = SimpleRandomAI().generateMove(color,game);
      expected[j] = 0;
      numPlayed[j] = 0;
    }

  int initialPlayNum = 1;
  LOG_VERBOSE << "Play all 1 time";
  //Play all moves "once"
  for(int j = 0; j<searchWidth; ++j)
    {
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
//      LOG_OUT << " " << moves[j].x << " " << moves[j].y << " E: "<<expected[j];
    }

  float maximisedVal = 0.0;
  int nextToPlay = 0;
  for (int j = 0; j<searchDepth; ++j)
    {
      //Maximise for all following plays
      for(int i = 0; i<searchWidth; ++i)
        {
          float ucbVal = expected[i] + sqrt( expRatio * log(totalNumPlayed) / numPlayed[i]);
          //          std::cerr <<i<< " " <<expected[i]<< " "<< sqrt( expRatio * log(totalNumPlayed) / numPlayed[i]) << std::endl;
          if(ucbVal > maximisedVal)
            {
              maximisedVal = ucbVal;
              nextToPlay = i;
            }
        }
//      std::cerr <<nextToPlay<< " " <<expected[nextToPlay]<< " "<< sqrt( expRatio * log(totalNumPlayed) / numPlayed[nextToPlay]) << std::endl;
      //Play best move and update expected return
      float result = simulateMove(moves[nextToPlay]);
      if(result > 0)
        result = 1;
      else
        result = -1;
      ++numPlayed[nextToPlay];
      ++totalNumPlayed;

      float oldWins = expected[nextToPlay] * numPlayed[nextToPlay];
      ++numPlayed[nextToPlay];
      ++totalNumPlayed;
      expected[nextToPlay] = ((float)(result+oldWins)/(float)numPlayed[nextToPlay]);

      maximisedVal = 0;
      nextToPlay = 0;
    }

  //Select best move after search completion
  int bestMove = -1;
  float bestExpected = 0;
  for(int i = 0; i<searchWidth; ++i)
    {
      if(expected[i]>bestExpected)
        {
          bestExpected = expected[i];
          bestMove = i;
        }
    }

  LOG_DEBUG<<"Best move ("<<moves[bestMove].x<<","<<moves[bestMove].y<<") : "<<bestExpected<<std::endl;
  return moves[bestMove];
}

const float UpperConfidence::simulateMove(const GoPoint& move)
{
  float expected = 0;
  GoBoard* nboard =game->Board->copyBoard();
  nboard->Play(move);
  SimpleRandomAI().simulateGame(nboard);
  float score = nboard ->GetScore();
  if(color == S_BLACK && score>0)
    expected += score;
  else if(color == S_WHITE && score<0)
    expected -= score;
  delete nboard;
  return expected;
}
