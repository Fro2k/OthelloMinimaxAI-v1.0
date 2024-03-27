#include "cMain.h"
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <string>

#define MAX_DEPTH 4
#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8
const unsigned int maxDepth = 4;
const unsigned int boardWidth = 8;
const unsigned int boardHeight = 8;
const unsigned int boardSize = boardWidth * boardHeight;
unsigned int board[8][8] = {};
unsigned short int pieceSize = 100;
bool gameOver = false;

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Othello", wxPoint(30, 30), wxSize(800,800))
{
	btn = new wxBitmapButton * [boardWidth * boardHeight];
	wxGridSizer* grid = new wxGridSizer(boardWidth, boardHeight, 0, 0);

	wxImage::AddHandler(new wxPNGHandler);
	wxBitmap bmpEmpty = wxBitmap(wxBitmap("Empty.png", wxBITMAP_TYPE_PNG).ConvertToImage().Scale(pieceSize, pieceSize));
	wxBitmap bmpWhite = wxBitmap(wxBitmap("White.png", wxBITMAP_TYPE_PNG).ConvertToImage().Scale(pieceSize, pieceSize));
	wxBitmap bmpBlack = wxBitmap(wxBitmap("Black.png", wxBITMAP_TYPE_PNG).ConvertToImage().Scale(pieceSize, pieceSize));

	for (int x = 0; x < boardWidth; x++)
	{
		for (int y = 0; y < boardHeight; y++)
		{
			const int btnIndex = y * boardWidth + x;
			btn[btnIndex] = new wxBitmapButton(this, 10000 + (btnIndex), bmpEmpty, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, wxButtonNameStr);
			grid->Add(btn[btnIndex], 1);
			btn[btnIndex]->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &cMain::OnButtonClicked, this);
			board[y][x] = 0;
		}
	}

	board[3][3] = 1;
	board[4][3] = 2;
	board[3][4] = 2;
	board[4][4] = 1;
	btn[27]->SetBitmap(bmpWhite);
	btn[28]->SetBitmap(bmpBlack);
	btn[35]->SetBitmap(bmpBlack);
	btn[36]->SetBitmap(bmpWhite);

	this->SetSizer(grid);
	grid->Layout();
}

cMain::~cMain()
{
	delete[]btn;
}

void cMain::OnButtonClicked(wxCommandEvent& evt)
{
	int x = (evt.GetId() - 10000) % boardWidth;
	int y = (evt.GetId() - 10000) / boardWidth;
	const int btnIndex = y * boardWidth + x;

	// BEGIN OTHELLO
	//
		
		// Update players actions on the board
		UpdateBoard(x, y, true);

		// Check for game over
		if(CheckForGameOver()) wxMessageBox("The game has ended.", "Game Over.");

		// Compute and update bot's actions on the board
		bool moveMade = BotUpdateBoard(maxDepth);

		// Check for game over
		if (!moveMade)
		{
			int white(0);
			for (int _x = 0; _x < boardWidth; _x++)
			{
				for (int _y = 0; _y < boardHeight; _y++)
				{
					if (board[_y][_x] == 1) white++;
					else if (board[_y][_x] == 2) white--;
				}
			}

			if (white > 0) wxMessageBox("The game has ended. White wins!", "Game Over.");
			if (white == 0) wxMessageBox("The game has ended. Stalemate.", "Game Over.");
			else wxMessageBox("The game has ended. Black wins!", "Game Over.");
		}

		//BEGIN DEBUGGING
		//
			std::string str;
			for (int x = 0; x < boardWidth; x++)
			{
				for (int y = 0; y < boardHeight; y++)
				{
					std::string brd;
					if (board[y][x] == 0) {
						brd = "_";
					}
					else
					{
						brd = std::to_string(board[y][x]);
					}
					str += brd + " ";
				}
				str += " \n";
			}
			wxString wxStr = str;
			wxLogDebug(wxStr);
		//
		//END DEBUGGING

	//
	// END OTHELLO
	evt.Skip();
}

bool cMain::BotUpdateBoard(int maxDepth)
{
	int x = 0;
	int y = 0;

	unsigned int virtualBoard[BOARD_WIDTH][BOARD_HEIGHT] = {};
	for (int _x = 0; _x < boardWidth; _x++)
	{
		for (int _y = 0; _y < boardHeight; _y++)
		{
			virtualBoard[_y][_x] = board[_y][_x];
		}
	}

	int openSpaces(boardSize);
	for (int _x = 0; _x < boardWidth; _x++)
	{
		for (int _y = 0; _y < boardHeight; _y++)
		{
			if (virtualBoard[_y][_x] != 0) openSpaces--;
		}
	}

	unsigned int virtualBoard_0[BOARD_WIDTH][BOARD_HEIGHT] = {};
	unsigned int virtualBoard_1[BOARD_WIDTH][BOARD_HEIGHT] = {};
	unsigned int virtualBoard_2[BOARD_WIDTH][BOARD_HEIGHT] = {};
	unsigned int virtualBoard_3[BOARD_WIDTH][BOARD_HEIGHT] = {};
	auto backup = [&](int select)
	{
		for (int _x = 0; _x < boardWidth; _x++)
		{
			for (int _y = 0; _y < boardHeight; _y++)
			{
				switch (select)
				{
				case 0:
					virtualBoard_0[_y][_x] = virtualBoard[_y][_x];
					break;
				case 1:
					virtualBoard_1[_y][_x] = virtualBoard[_y][_x];
					break;
				case 2:
					virtualBoard_2[_y][_x] = virtualBoard[_y][_x];
					break;
				case 3:
					virtualBoard_3[_y][_x] = virtualBoard[_y][_x];
					break;
				}
			}
		}
	};
	auto restore = [&](int select)
	{
		for (int _x = 0; _x < boardWidth; _x++)
		{
			for (int _y = 0; _y < boardHeight; _y++)
			{
				switch (select)
				{
				case 0:
					virtualBoard[_y][_x] = virtualBoard_0[_y][_x];
					break;
				case 1:
					virtualBoard[_y][_x] = virtualBoard_1[_y][_x];
					break;
				case 2:
					virtualBoard[_y][_x] = virtualBoard_2[_y][_x];
					break;
				case 3:
					virtualBoard[_y][_x] = virtualBoard_3[_y][_x];
					break;
				}
			}
		}
	};

	auto UpdateVirtualDirection = [&](int x, int y, int _x, int _y)
	{
		bool flaggedPiece(false);
		if (_x < 0 || _x >= boardWidth || _y < 0 || _y >= boardHeight)
		{
			for (int __x = 0; __x < boardWidth; __x++)
			{
				for (int __y = 0; __y < boardHeight; __y++)
				{
					if (virtualBoard[__y][__x] > 10) virtualBoard[__y][__x] -= 10;
				}
			}
			return 1;
		}
		else if (virtualBoard[_y][_x] != virtualBoard[y][x] && virtualBoard[_y][_x] != 0)
		{
			virtualBoard[_y][_x] += 10;
			return 0;
		}
		else if (virtualBoard[_y][_x] == virtualBoard[y][x])
		{
			bool flaggedPiece(false);
			for (int __x = 0; __x < boardWidth; __x++)
			{
				for (int __y = 0; __y < boardHeight; __y++)
				{
					if (virtualBoard[__y][__x] > 10)
					{
						virtualBoard[__y][__x] = virtualBoard[y][x];
						flaggedPiece = true;
					}
				}
			}
			if (flaggedPiece) return 2;
			else return 1;
		}
		else
		{
			for (int __x = 0; __x < boardWidth; __x++)
			{
				for (int __y = 0; __y < boardHeight; __y++)
				{
					if (virtualBoard[__y][__x] > 10) virtualBoard[__y][__x] -= 10;
				}
			}
			return 1;
		}
	};
	auto UpdateVirtualBoard = [&](int x, int y, int playerToMove)
	{
		virtualBoard[y][x] = playerToMove;
		bool updatedSomething(false);

		// Detect and flip pieces according to Othello rules
		if (virtualBoard[y][x] != 0)
		{
			for (int _x = x + 1; _x <= boardWidth; _x++)
			{
				int _y = y;
				int uvd = UpdateVirtualDirection(x, y, _x, _y);
				if (uvd >= 1)
				{
					if (uvd == 2) updatedSomething = true;
					break;
				}
			}
			for (int _x = x - 1; _x >= -1; _x--)
			{
				int _y = y;
				int uvd = UpdateVirtualDirection(x, y, _x, _y);
				if (uvd >= 1)
				{
					if (uvd == 2) updatedSomething = true;
					break;
				}
			}
			for (int _y = y + 1; _y <= boardHeight; _y++)
			{
				int _x = x;
				int uvd = UpdateVirtualDirection(x, y, _x, _y);
				if (uvd >= 1)
				{
					if (uvd == 2) updatedSomething = true;
					break;
				}
			}
			for (int _y = y - 1; _y >= -1; _y--)
			{
				int _x = x;
				int uvd = UpdateVirtualDirection(x, y, _x, _y);
				if (uvd >= 1)
				{
					if (uvd == 2) updatedSomething = true;
					break;
				}
			}


			for (int _x = x + 1; _x <= boardWidth; _x++)
			{
				int _y = y + (_x - x);
				int uvd = UpdateVirtualDirection(x, y, _x, _y);
				if (uvd >= 1)
				{
					if (uvd == 2) updatedSomething = true;
					break;
				}
			}
			for (int _x = x - 1; _x >= -1; _x--)
			{
				int _y = y + (x - _x);
				int uvd = UpdateVirtualDirection(x, y, _x, _y);
				if (uvd >= 1)
				{
					if (uvd == 2) updatedSomething = true;
					break;
				}
			}
			for (int _x = x + 1; _x <= boardWidth; _x++)
			{
				int _y = y - (_x - x);
				int uvd = UpdateVirtualDirection(x, y, _x, _y);
				if (uvd >= 1)
				{
					if (uvd == 2) updatedSomething = true;
					break;
				}
			}
			for (int _x = x - 1; _x >= -1; _x--)
			{
				int _y = y - (x - _x);
				int uvd = UpdateVirtualDirection(x, y, _x, _y);
				if (uvd >= 1)
				{
					if (uvd == 2) updatedSomething = true;
					break;
				}
			}
		}

		return updatedSomething;
	};

	unsigned int xFirstSpace(0);
	unsigned int yFirstSpace(0);
	auto placeInFirstSpace = [&](int prefEmptySpaces, int playerToMove)
	{
		unsigned int numEmptySpaces(0);
		xFirstSpace = 0;
		yFirstSpace = 0;
		bool updatedSomething(false);
		for (int _x = 0; _x < boardWidth; _x++)
		{
			for (int _y = 0; _y < boardHeight; _y++)
			{
				if (virtualBoard[_y][_x] == 0)
				{
					if (numEmptySpaces == prefEmptySpaces)
					{
						if (UpdateVirtualBoard(_x, _y, playerToMove)) updatedSomething = true;
						xFirstSpace = _x;
						yFirstSpace = _y;
						_x = 1000;
						_y = 1000;
						break;
					}
					numEmptySpaces++;
				}
			}
		}
		return updatedSomething;
	};

	unsigned int best_x(0);
	unsigned int best_y(0);
	bool oneMoveFound(false);
	backup(0);
	unsigned int maxScore_i(0);
	for (int i = 0; i < openSpaces; i++)
	{
		restore(0);
		if(!placeInFirstSpace(i, 1)) continue;
		int xi = xFirstSpace;
		int yi = yFirstSpace;

		backup(1);
		unsigned int minScore_j(1000);
		for (int j = 0; j < openSpaces - 1; j++)
		{
			restore(1);
			if(!placeInFirstSpace(j, 2)) continue;

			backup(2);
			unsigned int maxScore_k(0);
			for (int k = 0; k < openSpaces - 2; k++)
			{
				restore(2);
				if(!placeInFirstSpace(k, 1)) continue;

				backup(3);
				unsigned int minScore_l(1000);
				for (int l = 0; l < openSpaces - 3; l++)
				{
					restore(3);
					if(!placeInFirstSpace(l, 2)) continue;
					oneMoveFound = true;
					unsigned int score(0);
					for (int _x = 0; _x < boardWidth; _x++)
					{
						for (int _y = 0; _y < boardHeight; _y++)
						{
							if (virtualBoard[_y][_x] == 1) score++;
							else if (virtualBoard[_y][_x] == 2) score--;
						}
					}

					if (score <= minScore_l) minScore_l = score;
				}
				if (minScore_l >= maxScore_k) maxScore_k = minScore_l;
			}
			if (maxScore_k <= minScore_j) minScore_j = maxScore_k;
		}
		if (minScore_j >= maxScore_i)
		{
			maxScore_i = minScore_j;
			best_x = xi;
			best_y = yi;
		}
		virtualBoard[yi][xi] = 0;
	}
	//wxString wxStr = std::to_string(best_x);
	//wxLogDebug(wxStr);

	//wxStr = std::to_string(best_y);
	//wxLogDebug(wxStr);
	if (oneMoveFound) UpdateBoard(best_x, best_y, false);
	return oneMoveFound;
}

void cMain::UpdateBoard(int x, int y, bool player)
{
	// Update board to reflect user/bot input
	if (player)
	{
		board[y][x] += 2;
		if (board[y][x] == 4) board[y][x] = 1;
		else if (board[y][x] == 3) board[y][x] = 0;
	}
	else board[y][x] = 1;

	// Detect and flip pieces according to Othello rules
	if (board[y][x] != 0)
	{
		for (int _x = x + 1; _x <= boardWidth; _x++)
		{
			int _y = y;
			if (UpdateDirection(x, y, _x, _y)) break;
		}
		for (int _x = x - 1; _x >= -1; _x--)
		{
			int _y = y;
			if (UpdateDirection(x, y, _x, _y)) break;
		}
		for (int _y = y + 1; _y <= boardHeight; _y++)
		{
			int _x = x;
			if (UpdateDirection(x, y, _x, _y)) break;
		}
		for (int _y = y - 1; _y >= -1; _y--)
		{
			int _x = x;
			if (UpdateDirection(x, y, _x, _y)) break;
		}


		for (int _x = x + 1; _x <= boardWidth; _x++)
		{
			int _y = y + (_x - x);
			if (UpdateDirection(x, y, _x, _y)) break;
		}
		for (int _x = x - 1; _x >= -1; _x--)
		{
			int _y = y + (x - _x);
			if (UpdateDirection(x, y, _x, _y)) break;
		}
		for (int _x = x + 1; _x <= boardWidth; _x++)
		{
			int _y = y - (_x - x);
			if (UpdateDirection(x, y, _x, _y)) break;
		}
		for (int _x = x - 1; _x >= -1; _x--)
		{
			int _y = y - (x - _x);
			if (UpdateDirection(x, y, _x, _y)) break;
		}
	}

	// Update BMP visuals
	wxBitmap bmpEmpty = wxBitmap(wxBitmap("Empty.png", wxBITMAP_TYPE_PNG).ConvertToImage().Scale(pieceSize, pieceSize));
	wxBitmap bmpWhite = wxBitmap(wxBitmap("White.png", wxBITMAP_TYPE_PNG).ConvertToImage().Scale(pieceSize, pieceSize));
	wxBitmap bmpBlack = wxBitmap(wxBitmap("Black.png", wxBITMAP_TYPE_PNG).ConvertToImage().Scale(pieceSize, pieceSize));
	for (int x = 0; x < boardWidth; x++)
	{
		for (int y = 0; y < boardHeight; y++)
		{
			const int _btnIndex = y * boardWidth + x;
			if (board[y][x] == 1)		btn[_btnIndex]->SetBitmap(bmpWhite);
			else if (board[y][x] == 2)	btn[_btnIndex]->SetBitmap(bmpBlack);
			else						btn[_btnIndex]->SetBitmap(bmpEmpty);
		}
	}
}

int cMain::UpdateDirection(int x, int y, int _x, int _y)
{
	if (_x < 0 || _x >= boardWidth || _y < 0 || _y >= boardHeight)
	{
		for (int __x = 0; __x < boardWidth; __x++)
		{
			for (int __y = 0; __y < boardHeight; __y++)
			{
				if (board[__y][__x] > 10) board[__y][__x] -= 10;
			}
		}
		return 1;
	}
	else if (board[_y][_x] != board[y][x] && board[_y][_x] != 0)
	{
		board[_y][_x] += 10;
		return 0;
	}
	else if (board[_y][_x] == board[y][x])
	{
		for (int __x = 0; __x < boardWidth; __x++)
		{
			for (int __y = 0; __y < boardHeight; __y++)
			{
				if (board[__y][__x] > 10) board[__y][__x] = board[y][x];
			}
		}
		return 1;
	}
	else
	{
		for (int __x = 0; __x < boardWidth; __x++)
		{
			for (int __y = 0; __y < boardHeight; __y++)
			{
				if (board[__y][__x] > 10) board[__y][__x] -= 10;
			}
		}
		return 1;
	}
}

bool cMain::CheckForGameOver()
{
	bool flag = true;
	for (int _x = 0; _x < boardWidth; _x++)
	{
		for (int _y = 0; _y < boardHeight; _y++)
		{
			if (board[_y][_x] == 0) flag = false;
		}
	}
	return flag;
}





// BEGIN OTHELLO// END OTHELLO