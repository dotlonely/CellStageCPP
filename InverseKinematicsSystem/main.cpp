#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <queue>

const float PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062f;

struct Segment {
	Segment* next = NULL;
	Segment* prev = NULL;
	float radius = 4.0;
	float angle = 1.0;
	olc::vf2d center = olc::vf2d(1, 1);
	olc::vf2d pointA = olc::vf2d(1, 1);
	olc::vf2d pointB = olc::vf2d(1, 1);
	float length = 5.0;

	Segment(float x, float y, float radius, float angle) {
		pointA.x = x;
		pointA.y = y;
		this->radius = radius;
		this->angle = angle;
		CalculatePointB();
	}

	Segment(Segment* prev, float radius) {
		this->prev = prev;
		this->radius = radius;
		pointA = prev->pointB;
		CalculatePointB();
	}

	void CalculatePointB() {
		float dx = length * std::cos(angle * (PI / 180.0));
		float dy = length * std::sin(angle * (PI / 180.0));
		pointB.x = pointA.x - dx;
		pointB.y = pointA.y - dy;
	}

	void CalculateCenter() {
		center = (pointA + pointB) / 2.0;
	}

	void Follow(olc::vf2d target) {
		olc::vf2d dir = (target - pointA).norm();
		angle = std::atan2(dir.y, dir.x) * (180.0 / PI);
		dir = dir * length;
		dir *= -1.0;

		pointA = target + dir;
	}

	void Follow(Segment* node) {
		olc::vf2d target = olc::vf2d(node->pointA.x, node->pointA.y);
		Follow(target);
	}

	void Follow() {
		float targetX = next->pointA.x;
		float targetY = next->pointA.y;
		Follow(olc::vf2d(targetX, targetY));
	}

	void SetPointA(olc::vf2d position) {
		pointA = position;
		CalculatePointB();
	}

};

struct Worm{
	Segment* head;
	Segment* tail;
	int size = 0;

	void AddFirst(Segment* node) {
		if (size == 0) {
			head = node;
			tail = node;
			size++;
		} else if (head == tail) {
			node->next = tail;
			tail->prev = node;
			head = node;
			size++;
		}
		else {
			node->next = head;
			head->prev = node;
			head = node;
			size++;
		}
	}

	void AddLast(Segment* node) {
		if (size == 0) {
			node->next = NULL;
			node->prev = NULL;
			head = node;
			tail = node;
			size++;
		} else if (head == tail) {
			head->next = node;
			node->prev = head;
			node->next = NULL;
			tail = node;
			size++;
		}
		else {
			node->prev = tail;
			tail->next = node;
			node->next = NULL;
			tail = node;
			size++;   
		}
	}

	int Size() {
		if (head == NULL && tail == NULL) 
			return 0;

		if (head == tail)
			return 1;
		
		int count = 0;

		Segment* current = head;

		while (current->next != NULL) {
			current = current->next;
			count++;
		}

		return count;
	}

};

struct Food
{
	enum FoodType 
	{
		PLANT,
		MEAT
	};

	FoodType foodType;
	olc::vf2d position;
	olc::Pixel color;

	int nutritionalValue;
	int radius;

public:
	Food(olc::vf2d pos, FoodType type, int nutrition, int r)
	{
		position = pos;
		foodType = type;
		nutritionalValue = nutrition;
		radius = r;

		if (type == PLANT)
		{
			color = olc::GREEN;
		}
		else
		{
			color = olc::DARK_RED;
		}
	}
};


class InverseKinematicsSystem : public olc::PixelGameEngine
{

	int foodEaten = 0;
	bool eatInput = false;

	Food* currentFood = NULL;
	int currentFoodIndex = 0;

	float nextFollowPointTime = 3.0f;
	float startTime = 0.0f;

	float moveSpeed = 20.0f;

	std::list<Food> foodList;
	
	std::string foodString = "Food: " + foodEaten;

	olc::vf2d basePoint = olc::vf2d(320.0f, 320.0f);

	olc::vf2d followPoint = olc::vf2d(0, 0);

	Worm worm;

	Segment* baseSegment;
	Segment* grabSegment;


public:
	InverseKinematicsSystem()
	{
		sAppName = "InverseKinematicsSystem";
	}

public:
	bool OnUserCreate() override
	{
		// Main worm segments
		Segment* startNode = new Segment(basePoint.x, basePoint.y, 2.0, 0.0);
		Segment* secondNode = new Segment(startNode, 3.0);
		Segment* thirdNode = new Segment(secondNode, 4.0);
		Segment* fourthNode = new Segment(thirdNode, 5.0);
		Segment* fifthNode = new Segment(fourthNode, 3.0);

		// Add segments to list
		worm.AddFirst(startNode);
		worm.AddLast(secondNode);
		worm.AddLast(thirdNode);
		worm.AddLast(fourthNode);
		worm.AddLast(fifthNode);

		Food plantOne = Food(olc::vf2d(rand() % ScreenWidth(), rand() % ScreenHeight()), Food::PLANT, 1, 2);
		Food plantTwo = Food(olc::vf2d(rand() % ScreenWidth() * 2, rand() % ScreenHeight() * 2), Food::MEAT, 2, 3);

		foodList.push_back(plantOne);
		foodList.push_back(plantTwo);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		srand(time(0));

		startTime += fElapsedTime;

		// Clear screen every frame
		Clear(olc::BLACK);

		// Handle player input
		Input(fElapsedTime);

		if (startTime >= nextFollowPointTime) 
		{
			startTime = 0;
		}
	

	
		UpdateWorm(followPoint);

		EatFood();

		RenderFood();

		DrawString(280, 10, "SPORE CLONE", olc::WHITE, 1);

		return true;
	}


	// Draws specific segments circle
	void RenderSegment(Segment* segment, olc::Pixel color) {
		FillCircle(segment->center, segment->radius, color);
	}

	// Handles User Input, called before any rendering is done
	void Input(float fElapsedTime) {
		
		if (GetKey(olc::E).bPressed)
		{
			eatInput = true;
		}
		else if (GetKey(olc::E).bReleased)
		{
			eatInput = false;
		}


		if (GetKey(olc::W).bHeld) 
		{
			followPoint.y -= moveSpeed * fElapsedTime;
		}

		if (GetKey(olc::S).bHeld)
		{
			followPoint.y += moveSpeed * fElapsedTime;
		}

		if (GetKey(olc::A).bHeld)
		{
			followPoint.x -= moveSpeed * fElapsedTime;
		}

		if (GetKey(olc::D).bHeld)
		{
			followPoint.x += moveSpeed * fElapsedTime;
		}


	}


	// Updates all worm segments and draws them to screen
	void UpdateWorm(olc::vf2d followPoint)
	{
		baseSegment = worm.head;
		baseSegment->prev = NULL;
		grabSegment = worm.tail;
		grabSegment->next = NULL;

		grabSegment->Follow(followPoint);
		grabSegment->CalculatePointB();
		grabSegment->CalculateCenter();

		FillCircle(grabSegment->center, grabSegment->radius, olc::WHITE);

		baseSegment->Follow();
		baseSegment->CalculatePointB();
		baseSegment->CalculateCenter();

		FillCircle(baseSegment->center, baseSegment->radius, olc::WHITE);

		Segment* current = baseSegment->next;

		while (current->next != NULL) {

			current->Follow();
			current->CalculatePointB();
			current->CalculateCenter();

			RenderSegment(current, olc::WHITE);

			current = current->next;
		}
	}


	// Moves follow point to next point
	void UpdateFollowPoint(olc::vf2d moveToPoint, float fElapsedTime)
	{
		followPoint = ((1 - fElapsedTime) * followPoint + (fElapsedTime * moveToPoint));
	}

	// Draws each food circle in list
	void RenderFood()
	{
		for (auto& f : foodList)
		{
			DrawCircle(f.position, f.radius, f.color);
		}
	}

	void EatFood()
	{

		for (auto& f : foodList)
		{
			if (f.position.x - followPoint.x <= 2 && f.position.y - followPoint.y <= 2)
			{
				if (eatInput)
				{
					foodEaten++;
					foodList.pop_back();
					std::cout << "Touching food" << std::endl;
				}
			}
		}
	}

};



int main()
{
	InverseKinematicsSystem demo;
	if (demo.Construct(640, 360, 2, 2))
		demo.Start();
	return 0;
}
