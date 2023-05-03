#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "FoodPiece.h"
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

	olc::Pixel color = { 255, 255, 255, 1 }; 

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

struct Creature{

	enum GrowthStage
	{
		STAGE1, // 0
		STAGE2, // 1
		STAGE3, // 2
		STAGE4, // 3
		STAGE5  // 4
	};

	int currentStage = STAGE1;

	int stage2GrowthRequirement = 10;
	int stage3GrowthRequirement = 20;
	int stage4GrowthRequirement = 30;
	int stage5GrowthRequirement = 40;

	int maxStage = STAGE5;
	
	int currentGrowthRequirement = 0;

	int foodEaten = 0;
	int meatEaten = 0;
	int plantsEaten = 0;

	bool herbivore = false;
	bool carnivore = false;
	bool omnivore = true;

	olc::Pixel color = { 255, 255, 255, 255};

	Segment* head;
	Segment* tail;
	int size = 0;

	Creature(){ }

	Creature(int size)
	{
		for (int i = 0; i < size; ++i)
		{
			Segment* seg = new Segment(100, 100, 3, 0);
			AddLast(seg);
		}
	}
	

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

	std::string CurrentStageToString()
	{

		if (currentStage == STAGE1)
		{
			return "Growth Stage 1";
		}
		else if (currentStage == STAGE2)
		{
			return "Growth Stage 2";
		}
		else if (currentStage == STAGE3)
		{
			return "Growth Stage 3";
		}
		else if (currentStage == STAGE4)
		{
			return "Growth Stage 4";
		}
		else
		{
			return "Growth Stage Max";
		}
	}
};

struct Player : Creature
{
	Player()
	{
		Segment* startNode = new Segment(200, 200, 1.0, 0.0);
		Segment* secondNode = new Segment(startNode, 2.0);

		AddFirst(startNode);
		AddLast(secondNode);
	}
};

class CellStage : public olc::PixelGameEngine
{
	float timer = 2.0f;

	float eatenFoodDecayTimer = 10.0f;

	int maxFoodAmount = 30;
	int currentFoodAmount = 0;

	int foodEaten = 0;
	bool eatInput = false;

	float startTime = 0.0f;

	float moveSpeed = 50.0f;

	std::vector<FoodPiece> foodList;
	std::vector<Creature> creatures;

	olc::vf2d basePoint = { 320.0f, 320.0f };

	olc::vf2d followPoint = { ScreenWidth() / 2.0f, ScreenHeight() / 2.0f};

	olc::vf2d creatureFollowPoint = { ScreenWidth() / 2.0f, ScreenHeight() / 2.0f };

	Player player;

	Creature c1 = Creature(5);
	Creature c2;


	
public:
	CellStage()
	{
		sAppName = "CellStage";
	}

public:
	bool OnUserCreate() override
	{
		// Main player segments
		//Segment* startNode = new Segment(basePoint.x, basePoint.y, 1.0, 0.0);
		//Segment* secondNode = new Segment(startNode, 2.0);

		// Add segments to list
		//player.AddFirst(startNode);
		//player.AddLast(secondNode);

		creatures.push_back(c1);
		creatures.push_back(c2);

		return true;
	}

	// GAME UPDATE -------------------------------------------------------------------------------------------

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

		//UpdateCreatureCore(creatures, creatureFollowPoint);
		UpdatePlayerCore(followPoint);

		DrawString(280, 10, "cellStage", olc::WHITE, 1.0f);

		DrawString(10, 10, "Press 'e' to Eat food!", olc::WHITE, 1.0f);

		DrawString(10, 350, player.CurrentStageToString(), olc::WHITE, 1.0f);

		DrawString(600, 350, std::to_string(player.foodEaten), olc::WHITE, 1.0f);

		if (startTime >= timer)
		{
			if (currentFoodAmount < maxFoodAmount)
			{
				SpawnFood(1, dist1(mt), dist2(mt));

			}

			UpdateCreatureCore(creatures, creatureFollowPoint);

			//PrintFoodList();

			//std::cout << player.currentStage << std::endl;
			
			startTime = 0;
		}

		for (auto& f : foodList)
		{
			HandlePlayerCollision(f);
		}

		RenderFood();

		return true;
	}


	// -------------------------------------------------------------------------------------------------------


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


		if (GetKey(olc::ESCAPE).bPressed)
		{

		}

	}

	void UpdateCreatureCore(std::vector<Creature> list, olc::vf2d followPoint)
	{
		for (auto& c : creatures)
		{
			std::cout << c.size << std::endl;
		}
	}


	// Updates all player segments and draws them to screen
	void UpdatePlayerCore(olc::vf2d followPoint)
	{

		player.tail->Follow(followPoint);
		player.tail->CalculatePointB();
		player.tail->CalculateCenter();

		FillCircle(player.tail->center, player.tail->radius, player.color); 

		Segment* current = player.head;

		while (current->next != NULL) {

			current->Follow();
			current->CalculatePointB();
			current->CalculateCenter();

			RenderSegment(current, player.color);

			current = current->next;
		}

		GetCurrentStageGrowthRequirement();
		HandlePlayerGrowth();
	}


	void UpdatePlayerStats()
	{
		if (player.plantsEaten > player.meatEaten)
		{
			player.herbivore = true;
			player.omnivore = false;
			player.carnivore = false;
		}
		else if (player.meatEaten > player.plantsEaten)
		{
			player.carnivore = true;
			player.omnivore = false;
			player.herbivore = false;
		}
		else
		{
			player.omnivore = true;
			player.carnivore = false;
			player.herbivore = false;
		}
	}

	void UpdatePlayerLooks()
	{
		
		// TODO: Draw extra things on player based on diet, kills, etc...

		if (player.plantsEaten > player.meatEaten)
		{
			player.color = olc::DARK_GREEN;
		}
		else if (player.meatEaten > player.plantsEaten)
		{
			player.color = olc::DARK_RED;
		}
		else
		{
			player.color = olc::WHITE;
		}
	}

	void PrintFoodList()
	{

		std::cout << "SIZE: " << foodList.size() << std::endl;

		std::string type;


		for (auto& f : foodList)
		{
			if (f.GetFoodType() == 0)
			{
				type = "PLANT";
			}
			else type = "MEAT";

			std::cout << f.GetID() << " : " << type << ", " << f.GetPosition() << std::endl;
		}
	}

	void SpawnFood(int numFood, int rnd1, int rnd2)
	{
		for (int i = 0; i < numFood; ++i)
		{
			
			FoodPiece food = { rnd1, rnd2 };
			food.SetID(foodList.size());

			foodList.push_back(food);
		}

		currentFoodAmount += numFood;
	}

	void CheckEatenFoodDecayed(FoodPiece& f)
	{
		if (f.IsEaten())
		{
			f.SetEatenDecayTimer(f.GetEatenDecayTimer() + startTime);

			if (f.GetEatenDecayTimer()  >= eatenFoodDecayTimer)
			{
				// TODO: Respawn food to new location but same place in list.
			}
		}
	}

	void RenderFood()
	{
		for (auto& f : foodList)
		{
			FillCircle(f.GetPosition(), f.GetRadius(), f.GetColor());
		}
	}

	void HandleFoodCollision()
	{
		// TODO: Set up collision between food objects
	}


	// Checks if player is currently colliding with a food object
	void HandlePlayerCollision(FoodPiece& f)
	{
		// TODO: Set up collision between player and food (think about player and other entities as well)
		if (DoCirclesOverlap(player.tail->center.x, player.tail->center.y, player.tail->radius, f.GetPosition().x, f.GetPosition().y, f.GetRadius()))
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

	void EatFood(FoodPiece& f)
	{

		if ((player.omnivore || player.herbivore) && f.GetFoodType() == 0)
		{

			if (f.GetRadius() < 2)
			{
				f.SetIsEaten(true);
				f.SetColor(olc::GREY);
			}
			else
			{
				f.SetRadius(f.GetRadius() - 1);
			}

			player.plantsEaten++;
			player.foodEaten++;
		}
		
		
		if ((player.omnivore || player.carnivore) && f.GetFoodType() == 1)
		{

			if (f.GetRadius() < 2)
			{
				f.SetIsEaten(true);
				f.SetColor(olc::GREY);
			}
			else
			{
				f.SetRadius(f.GetRadius() - 1);
			}

			player.meatEaten++;
			player.foodEaten++;
		}
	}


	void HandlePlayerGrowth()
	{
		if (CheckIfShouldGrow())
		{

			if (player.currentStage == player.maxStage)
			{
				return;
			}
			else
			{
				player.currentStage = player.currentStage + 1;
			}

			UpdatePlayerLooks();
			UpdatePlayerStats();
		
			std::cout << "Player has grown from stage: " << player.currentStage - 1 << " to stage: " << player.currentStage << std::endl;
			
			// Creates new segment (on the heap)
			Segment* newSegment = new Segment(0, 0, 2, 0);

			// Adjust new Segment's attributes
			newSegment->length = player.head->length;

			// Makes new segment  the "head" of the player so its size should always be two, "tail should always be 1
			player.AddLast(newSegment);

			// Adjusts head and tail relative to new changes. (aka makes tail slightly bigger, and increases head length so we can still see the 1 radius circle
			player.tail->radius++;
			player.head->length = player.tail->length;
		}
	}

	bool CheckIfShouldGrow()
	{
		if (player.currentStage == player.maxStage)
			return false;

		if (player.foodEaten >= player.currentGrowthRequirement)
			return true;
		else return false;
	}

	void GetCurrentStageGrowthRequirement()
	{
		if (player.currentStage == 0)
		{
			player.currentGrowthRequirement = player.stage2GrowthRequirement;
		}
		else if (player.currentStage == 1)
		{
			player.currentGrowthRequirement = player.stage3GrowthRequirement;
		}
		else if (player.currentStage == 2)
		{
			player.currentGrowthRequirement = player.stage4GrowthRequirement;
		}
		else if (player.currentStage == 3)
		{
			player.currentGrowthRequirement = player.stage5GrowthRequirement;
		}
		else
		{
			player.currentGrowthRequirement = 999;
		}
	}
};

// MAIN ----------------------------------------

int main()
{
	CellStage demo;
	if (demo.Construct(640, 360, 2, 2))
		demo.Start();
	return 0;
}

// --------------------------------------------- 
