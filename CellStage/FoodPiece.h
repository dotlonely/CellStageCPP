#pragma once
#include "olcPixelGameEngine.h"


class FoodPiece
{
private:

	enum FoodType
	{
		PLANT,
		MEAT
	};

public:

	FoodPiece(int x, int y);


public:

	int GetFoodType();

	olc::vi2d GetPosition();

	void SetPosition(int x, int y);

	olc::vf2d GetVelocity();

	void SetVelocity(float x, float y);

	olc::vf2d GetAcceleration();

	void SetAcceleration(float x, float y);

	olc::Pixel GetColor();

	void SetColor(olc::Pixel c);

	int GetID();
	
	void SetID(int id);

	int GetNutritionalValue();

	int GetRadius();

	void SetRadius(int r);

	float GetEatenDecayTimer();

	void SetEatenDecayTimer(float timer);

	bool IsEaten();

	void SetIsEaten(bool value);

	std::string ToString();

private:


	int foodType;

	olc::vi2d position;
	olc::vf2d velocity;
	olc::vf2d acceleration;

	olc::Pixel color;

	int id;
	int nutritionalValue;
	int radius;

	float eatenDecayTimer;

	bool eaten;
	bool exists;
};


