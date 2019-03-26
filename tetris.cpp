#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <stdio.h>
#include <Windows.h>

struct Screen 
{
	int width = 80;
	int height = 30;
};

Screen nscreen;

struct Field 
{
	int width = 12;
	int height = 18;
	unsigned char *field_pointer = nullptr;
};
Field nfield;

std::wstring tetromino[7];

int Rotate(int px, int py, int r)
{
	int pi = 0;
	switch (r % 4)
	{
	case 0:
		pi = py * 4 + px;
		break;

	case 1:
		pi = 12 + py - (px * 4);
		break;

	case 2:
		pi = 15 - (py * 4) - px;
		break;

	case 3:
		pi = 3 - py + (px * 4);
		break;
	}
	return pi;
}

bool DoesPieceFit(int Tetrmino, int Rotation, int PosX, int PosY) 
{
	for(int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++) 
		{
			int pi = Rotate(px, py, Rotation); //get index into piece
			int fi = (PosY + py) * nfield.width + (PosX + px);  //get index into field

			if (PosX + px >= 0 && PosX + px < nfield.width)
			{
				if (PosY + py >= 0 && PosY + py < nfield.height) {
					if (tetromino[Tetrmino][pi] != L'.' && nfield.field_pointer[fi] != 0)
						return false;
				}
			}
		}
	return true;
}

int main(void) 
{
	wchar_t *screen = new wchar_t[nscreen.width*nscreen.height];
	for (int i = 0; i < nscreen.width * nscreen.height; i++) screen[i] = L' ';
	HANDLE Console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(Console);
	DWORD BytesWritten = 0;
	
	tetromino[0].append(L"..X...X...X...X."); // Tetronimos 4x4
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");
	
	nfield.field_pointer = new unsigned char[nfield.width*nfield.height];
	for (int x = 0; x < nfield.width; x++)
		for (int y = 0; y < nfield.height; y++)
			nfield.field_pointer[y*nfield.width + x] = (x == 0 || x == nfield.width - 1 || y == 0 || y == nfield.height - 1) ? 9 : 0;

	bool Key[4];
	int CurrentPiece = 0;
	int CurrentRotation = 0;
	int CurrentX = nfield.width / 2;
	int CurrentY = 0;
	int Speed = 20;
	int SpeedCount = 0;
	bool ForceDown = false;
	bool RotateHold = true;
	int PieceCount = 0;
	int Score = 0;
	std::vector<int> Lines;
	bool GameOver = false;

	while (!GameOver)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		SpeedCount++;
		ForceDown = (SpeedCount == Speed);

		for (int k = 0; k < 4; k++)
			Key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		CurrentX += (Key[0] && DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX + 1, CurrentY)) ? 1 : 0;
		CurrentX += (Key[1] && DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX - 1, CurrentY)) ? 1 : 0;
		CurrentX += (Key[2] && DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX, CurrentY + 1)) ? 1 : 0;
		
		if (Key[3])
		{
			CurrentRotation += (RotateHold && DoesPieceFit(CurrentPiece, CurrentRotation + 1, CurrentX, CurrentY)) ? 1 : 0;
			RotateHold = false;
		}
		else
			RotateHold = true;

		if (ForceDown) 
		{
			SpeedCount = 0;
			SpeedCount++;
			if (PieceCount % 50 == 0)
				if (Speed >= 10) Speed--;

			if (DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX, CurrentY + 1))
				CurrentY++;
			else
			{
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[CurrentPiece][Rotate(px, py, CurrentRotation)] != L'.')
							nfield.field_pointer[(CurrentY + py) * nfield.width + (CurrentX + px)] = CurrentPiece + 1;
				for(int py = 0; py < 4; py++)
					if (CurrentY + py < nfield.height - 1)
					{
						bool Line = true;
						for (int px = 1; px < nfield.width - 1; px++)
							Line &= (nfield.field_pointer[(CurrentY + py) * nfield.width + px]) != 0;

						if (Line)
						{
							for (int px = 1; px < nfield.width - 1; px++)
								nfield.field_pointer[(CurrentY + py) * nfield.width + px] = 8;
							Lines.push_back(CurrentY + py);
						}
					}
				Score += 25;
				if (!Lines.empty()) Score += (1 << Lines.size()) * 100;

				CurrentX = nfield.width / 2;
				CurrentY = 0;
				CurrentRotation = 0;
				CurrentPiece = rand() % 7;
				GameOver = !DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX, CurrentY);
			}
		}

		//Let's draw field
		for (int x = 0; x < nfield.width; x++)
			for (int y = 0; x < nfield.height; y++)
				screen[(y + 2)*nscreen.width + (x + 2)] = L" ABCDEFG=#"[nfield.field_pointer[y*nfield.width+ x]];


		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[CurrentPiece][Rotate(px, py, CurrentRotation)] != L'.')
					screen[(CurrentY + py + 2) * nscreen.width + (CurrentX + px + 2)] = CurrentPiece + 65;

		//Display Score
		swprintf_s(&screen[2 * nscreen.width + nfield.width + 6], 16, L"SCORE: %8d", Score);

		if (!Lines.empty())
		{
			WriteConsoleOutputCharacterW(Console, screen, nscreen.width * nscreen.height, { 0,0 }, &BytesWritten);
			std::this_thread::sleep_for(std::chrono::milliseconds(400));

			for(auto &v : Lines)
				for (int px = 1; px < nfield.width - 1; px++)
				{
					for (int py = v; py > 0; py--)
						nfield.field_pointer[py * nfield.width + px] = nfield.field_pointer[(py - 1) * nfield.width + px];
					nfield.field_pointer[px] = 0;
				}
			Lines.clear();
		}
		WriteConsoleOutputCharacterW(Console, screen, nscreen.width * nscreen.height , { 0,0 }, &BytesWritten);
	}
	CloseHandle(Console);
	printf("Game over, ur score is %d", Score);
	getchar();
	return 0;
}