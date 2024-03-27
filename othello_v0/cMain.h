#pragma once

#include "wx/wx.h"

class cMain : public wxFrame
{
public:
	cMain();
	~cMain();

public:
	wxBitmapButton** btn;

	void OnButtonClicked(wxCommandEvent& evt);
	void UpdateBoard(int x, int y, bool player);
	bool BotUpdateBoard(int maxDepth);
	int UpdateDirection(int x, int y, int _x, int _y);
	bool CheckForGameOver();

};

