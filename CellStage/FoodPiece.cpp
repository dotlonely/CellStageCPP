#include "FoodPiece.h"


FoodPiece::FoodPiece(int x, int y)
{
	position = { x, y };

	if (rand() % 2 == 0)
	{
		foodType = MEAT;
	}
	else foodType = PLANT;

	nutritionalValue = (rand() % 5) + 1;
	radius = nutritionalValue;
	eaten = false;

	if (foodType == MEAT)
	{
		color = olc::VERY_DARK_RED;
	}
	else
	{
		color = olc::VERY_DARK_GREEN;
	}
}

int FoodPiece::GetFoodType()
{
	return foodType;
}

olc::vi2d FoodPiece::GetPosition()
{
	return position;
}

void FoodPiece::SetPosition(int x, int y)
{
	position.x = x;
	position.y = y;
}

olc::vf2d FoodPiece::GetVelocity()
{
	return velocity;
}

void FoodPiece::SetVelocity(float x, float y)
{
	velocity.x = x;
	velocity.y = y;
}

olc::vf2d FoodPiece::GetAcceleration()
{
	return acceleration;
}

void FoodPiece::SetAcceleration(float x, float y)
{
	acceleration.x = x;
	acceleration.y = y;
}

olc::Pixel FoodPiece::GetColor()
{
	return color;
}

void FoodPiece::SetColor(olc::Pixel c)
{
	color = c;
}

int FoodPiece::GetID()
{
	return id;
}

void FoodPiece::SetID(int value)
{
	id = value;
}

int FoodPiece::GetNutritionalValue()
{
	return nutritionalValue;
}

int FoodPiece::GetRadius()
{
	return radius;
}

void FoodPiece::SetRadius(int r)
{
	radius = r;
}

float FoodPiece::GetEatenDecayTimer()
{
	return eatenDecayTimer;
}

void FoodPiece::SetEatenDecayTimer(float timer)
{
	eatenDecayTimer = timer;
}

bool FoodPiece::IsEaten()
{
	return eaten;
}

void FoodPiece::SetIsEaten(bool value)
{
	eaten = value;
}

std::string FoodPiece::ToString()
{
	if (foodType == PLANT)
	{
		return "PLANT";
	}
	else return "MEAT";
}