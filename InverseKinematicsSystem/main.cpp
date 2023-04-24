#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <queue>
#include <random>

const float PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062f;

struct Segment {
	Segment* next = NULL;
	Segment* prev = NULL;
	float radius = 4.0;
	float angle = 1.0;
	olc::vf2d center = olc::vf2d(1, 1);
	olc::vf2d pointA = olc::vf2d(1, 1);
	olc::vf2d pointB = olc::vf2d(1, 1);
	float length = 3.0;

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

	enum GrowthStage
	{
		STAGE1,
		STAGE2,
		STAGE3,
		STAGE4,
		STAGE5
	};

	int currentStage = STAGE1;
	int nextStage = currentStage + 1;

	int stage1GrowthRequirement = 10;
	int stage2GrowthRequirement = 20;
	int stage3GrowthRequirement = 30;
	int stage4GrowthRequirement = 40;
	int stage5GrowthRequirement = 50;
	

	Segment* head;
	Segment* tail;
	int size = 0;
	
	int foodEaten = 0;

	void AddFirst(Segment* node) 
	{
		if (size == 0) {
			head = node;
			tail = node;
			size++;
		} else if (head == tail) 
		{
			node->next = tail;
			tail->prev = node;
			head = node;
			size++;
		}
		else 
		{
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

		
public:
	Food(int x, int y)
	{

		position = { x, y };

		if (rand() % 2 == 1)
		{
			foodType = MEAT;
		}
		else foodType = PLANT;

		nutritionalValue = (rand() % 5) + 1;
		radius = nutritionalValue;
		eaten = false;

		if (foodType == MEAT)
		{
			color = olc::DARK_RED;
		}
		else
		{
			color = olc::DARK_GREEN;
		}

	}

	std::string ToString()
	{
		if (foodType == PLANT)
		{
			return "PLANT";
		}
		else return "MEAT";
	}


};


class CellStage : public olc::PixelGameEngine
{

	float timer = 3.0f;

	float eatenFoodDecayTimer = 10.0f;


	int maxFoodAmount = 10;
	int currentFoodAmount = 0;

	int foodEaten = 0;
	bool eatInput = false;


	float startTime = 0.0f;

	float moveSpeed = 50.0f;

	std::vector<Food> foodList;

	olc::vf2d basePoint = { 320.0f, 320.0f };

	olc::vf2d followPoint = { ScreenWidth() / 2.0f, ScreenHeight() / 2.0f};

	Worm player;

	Segment* baseSegment;
	Segment* grabSegment;

	
public:
	CellStage()
	{
		sAppName = "CellStage";
	}

public:
	bool OnUserCreate() override
	{
		// Main player segments
		Segment* startNode = new Segment(basePoint.x, basePoint.y, 1.0, 0.0);
		Segment* secondNode = new Segment(startNode, 2.0);
		//Segment* thirdNode = new Segment(secondNode, 4.0);
		//Segment* fourthNode = new Segment(thirdNode, 5.0);
		//Segment* fifthNode = new Segment(fourthNode, 3.0);

		// Add segments to list
		player.AddFirst(startNode);
		player.AddLast(secondNode);
		//player.AddLast(thirdNode);
		//player.AddLast(fourthNode);
		//player.AddLast(fifthNode);



		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<int> dist1(0, ScreenWidth());
		std::uniform_int_distribution<int> dist2(0, ScreenHeight());

		startTime += fElapsedTime;

		// Clear screen every frame
		Clear(olc::Pixel(0, 0, 20));

		// Handle player input
		Input(fElapsedTime);


		UpdateWorm(followPoint);

		DrawString(280, 10, "cellStage", olc::WHITE, 1.0f);

		if (startTime >= timer)
		{
			if (currentFoodAmount < maxFoodAmount)
			{
				SpawnFood(1, dist1(mt), dist2(mt));
			}

			PrintFoodList();
			startTime = 0;
		}

		for (auto& f : foodList)
		{
			HandlePlayerCollision(f);
		}

		RenderFood();

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
		else if (GetKey(olc::E).bHeld)
		{
			eatInput = false;
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


	// Updates all player segments and draws them to screen
	void UpdateWorm(olc::vf2d followPoint)
	{
		baseSegment = player.head;
		baseSegment->prev = NULL;
		grabSegment = player.tail;
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

	void PrintFoodList()
	{

		std::cout << "SIZE: " << foodList.size() << std::endl;

		std::string type;


		for (auto& f : foodList)
		{
			if (f.foodType == 0)
			{
				type = "PLANT";
			}
			else type = "MEAT";

			std::cout << f.id << " : " << type << ", " << f.position << std::endl;
		}
	}

	void SpawnFood(int numFood, int rnd1, int rnd2)
	{
		for (int i = 0; i < numFood; ++i)
		{
			
			Food food = { rnd1, rnd2 };
			food.id = foodList.size();

			foodList.push_back(food);
		}

		currentFoodAmount += numFood;
	}

	void CheckEatenFoodDecayed(Food& f)
	{
		if (f.eaten)
		{
			f.eatenDecayTimer += startTime;

			if (f.eatenDecayTimer >= eatenFoodDecayTimer)
			{
				// TODO: Respawn food to new location but same place in list.
			}
		}
	}

	void RenderFood()
	{
		for (auto& f : foodList)
		{
			FillCircle(f.position, f.radius, f.color);
		}
	}

	void HandleFoodCollision()
	{
		//TODO: Set up collision between food objects
	}


	// Checks if player is currently colliding with a food object
	void HandlePlayerCollision(Food& f)
	{
		//TODO: Set up collision between player and food (think about player and other entities as well)
		if (DoCirclesOverlap(player.tail->center.x, player.tail->center.y, player.tail->radius, f.position.x, f.position.y, f.radius))
		{
			if (eatInput)
			{
				EatFood(f);
			}
		}
	}

	bool DoCirclesOverlap(float x1, float y1, float r1, float x2, float y2, float r2)
	{
		return fabs((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= (r1 + r2) * (r1 + r2);
	}

	void EatFood(Food& f)
	{
		if (f.radius < 2)
		{
			f.eaten = true;
			f.color = olc::GREY;
		}
		else 
		{
			f.radius--;
		}

		player.foodEaten++;
	}


	void HandlePlayerGrowth()
	{
		if (player.currentStage == player.STAGE1)
		{
			
		}
	}

	bool CheckIfShouldGrow()
	{
		if (player.foodEaten > player.stage1GrowthRequirement)
		{

		}
	}
};



int main()
{
	CellStage demo;
	if (demo.Construct(640, 360, 2, 2))
		demo.Start();
	return 0;
}
