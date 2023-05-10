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

struct Creature
{

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

	Segment* head = NULL;
	Segment* tail = NULL;

	int size = 0;

	olc::vf2d followPoint;

	float moveSpeed = 50;

	float foodDetectionRadius = 100.0f;

	olc::Pixel foodDetectionColor;

	std::vector<FoodPiece> detectedFood;

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

	void SetFollowPoint(olc::vf2d point)
	{
		followPoint = point;
	}

	void MoveUp()
	{
		followPoint.y -= 1;
	}

	void MoveDown()
	{
		followPoint.y += 1;
	}

	void MoveLeft()
	{
		followPoint.x -= 1;
	}

	void MoveRight()
	{
		followPoint.x += 1;
	}

	void EatFood(FoodPiece& f)
	{
		if ((omnivore || herbivore) && f.GetFoodType() == 0)
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

			plantsEaten++;
			foodEaten++;
		}


		if ((omnivore || carnivore) && f.GetFoodType() == 1)
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

			meatEaten++;
			foodEaten++;
		}
	}

	void HandleCollision(FoodPiece& f)
	{
		// TODO: Set up collision between player and food (think about player and other entities as well)
		if (DoCirclesOverlap(tail->center.x, tail->center.y, tail->radius, f.GetPosition().x, f.GetPosition().y, f.GetRadius()))
		{
			// TODO: Might need timer to ensure AI cant instant eat every stage of food
			EatFood(f);
		}
	}


	// Updates segments of creature
	void UpdateCore(olc::PixelGameEngine* pge, olc::vf2d followPoint)
	{
		tail->Follow(followPoint);
		tail->CalculatePointB();
		tail->CalculateCenter();

		RenderSegment(pge, tail);

		Segment* current = head;

		while (current->next != NULL)
		{
			current->Follow();
			current->CalculatePointB();
			current->CalculateCenter();

			RenderSegment(pge, current);

			current = current->next;
		}
	}

	// TODO: Method where we will handle all AI related tasks and call for each creature in main game loop
	void UpdateAI()
	{

	}

	void RenderSegment(olc::PixelGameEngine* pge, Segment* segment)
	{
		pge->FillCircle(segment->center, segment->radius, color);
	}

	void RenderDetectionRadius(olc::PixelGameEngine* pge)
	{
		pge->DrawCircle(tail->center, foodDetectionRadius, foodDetectionColor);
	}

	void HandleDetectingFood(FoodPiece& f)
	{
			
		if (DoCirclesOverlap(tail->center.x, tail->center.y, foodDetectionRadius, f.GetPosition().x, f.GetPosition().y, f.GetRadius()))
		{
			foodDetectionColor = olc::GREEN;
		}
		else
		{
			foodDetectionColor = olc::YELLOW;
		}
	}

	bool DoCirclesOverlap(float x1, float y1, float r1, float x2, float y2, float r2)
	{
		return fabs((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= (r1 + r2) * (r1 + r2);
	}
};


struct Player : Creature
{

	bool eatInput = false;

	Player()
	{
		Segment* startNode = new Segment(200, 200, 1.0, 0.0);
		Segment* secondNode = new Segment(startNode, 2.0);

		AddFirst(startNode);
		AddLast(secondNode);
	}

	void HandleCollision(FoodPiece& f)
	{
		if (DoCirclesOverlap(tail->center.x, tail->center.y, tail->radius, f.GetPosition().x, f.GetPosition().y, f.GetRadius()))
		{
			if (eatInput)
			{
				EatFood(f);
			}
		}
	}

	void HandlePlayerGrowth()
	{
		GetCurrentStageGrowthRequirement();

		if (ShouldGrow())
		{

			if (currentStage == maxStage)
			{
				return;
			}
			else
			{
				currentStage = currentStage + 1;
			}

			UpdatePlayerLooks();
			UpdatePlayerStats();

			std::cout << "Player has grown from stage: " << currentStage - 1 << " to stage: " << currentStage << std::endl;

			// Creates new segment (on the heap)
			Segment* newSegment = new Segment(0, 0, 2, 0);

			// Adjust new Segment's attributes
			newSegment->length = head->length;

			// Makes new segment  the "head" of the player so its size should always be two, "tail should always be 1
			AddLast(newSegment);

			// Adjusts head and tail relative to new changes. (aka makes tail slightly bigger, and increases head length so we can still see the 1 radius circle
			tail->radius++;
			head->length = tail->length;
		}
	}

	void GetCurrentStageGrowthRequirement()
	{
		if (currentStage == 0)
		{
			currentGrowthRequirement = stage2GrowthRequirement;
		}
		else if (currentStage == 1)
		{
			currentGrowthRequirement = stage3GrowthRequirement;
		}
		else if (currentStage == 2)
		{
			currentGrowthRequirement = stage4GrowthRequirement;
		}
		else if (currentStage == 3)
		{
			currentGrowthRequirement = stage5GrowthRequirement;
		}
		else
		{
			currentGrowthRequirement = 999;
		}
	}

	bool ShouldGrow()
	{
		if (currentStage == maxStage)
			return false;

		if (foodEaten >= currentGrowthRequirement)
			return true;
		else return false;
	}

	void UpdatePlayerStats()
	{
		if (plantsEaten > meatEaten)
		{
			herbivore = true;
			omnivore = false;
			carnivore = false;
		}
		else if (meatEaten > plantsEaten)
		{
			carnivore = true;
			omnivore = false;
			herbivore = false;
		}
		else
		{
			omnivore = true;
			carnivore = false;
			herbivore = false;
		}
	}


	void UpdatePlayerLooks()
	{

		// TODO: Draw extra things on player based on diet, kills, etc...

		if (plantsEaten > meatEaten)
		{
			color = olc::DARK_GREEN;
		}
		else if (meatEaten > plantsEaten)
		{
			color = olc::DARK_RED;
		}
		else
		{
			color = olc::WHITE;
		}
	}

	// INPUT -------------------------------------------------------------- 

	void HandleInput(olc::PixelGameEngine* pge, float fElapsedTime) {

		if (pge->GetKey(olc::E).bPressed)
		{
			eatInput = true;
		}
		else if (pge->GetKey(olc::E).bHeld)
		{
			eatInput = false;
		}
		else if (pge->GetKey(olc::E).bReleased)
		{
			eatInput = false;
		}


		if (pge->GetKey(olc::W).bHeld)
		{
			followPoint.y -= moveSpeed * fElapsedTime;
		}

		if (pge->GetKey(olc::S).bHeld)
		{
			 followPoint.y += moveSpeed * fElapsedTime;
		}

		if (pge->GetKey(olc::A).bHeld)
		{
			followPoint.x -= moveSpeed * fElapsedTime;
		}

		if (pge->GetKey(olc::D).bHeld)
		{
			followPoint.x += moveSpeed * fElapsedTime;
		}
	}

	// --------------------------------------------------------


};


class CellStage : public olc::PixelGameEngine
{
	float timer = 1.0f;

	float eatenFoodDecayTimer = 10.0f;

	int maxFoodAmount = 30;
	int currentFoodAmount = 0;

	float startTime = 0.0f;
	float aiMoveTime = 0.0f;

	std::vector<FoodPiece> foodList;
	std::vector<Creature> creatures;

	olc::vf2d followPoint = { ScreenWidth() / 2.0f, ScreenHeight() / 2.0f};

	Player player;

	Creature c1 = Creature(2);

public:
	CellStage()
	{
		sAppName = "CellStage";
	}

public:
	bool OnUserCreate() override
	{

		creatures.push_back(c1);

		return true;
	}

	// GAME UPDATE -------------------------------------------------------------------------------------------

	bool OnUserUpdate(float fElapsedTime) override
	{

		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<int> dist1(0, ScreenWidth());
		std::uniform_int_distribution<int> dist2(0, ScreenHeight());

		std::random_device rd2;
		std::mt19937 mt2(rd2());
		std::uniform_int_distribution<int> rndMoveDirection(0, 7);

		startTime += fElapsedTime;

		aiMoveTime += fElapsedTime; 

		// Clear screen every frame
		Clear(olc::Pixel(0, 0, 20));

		// Handle player input
		player.HandleInput(this, fElapsedTime);
		player.UpdateCore(this, player.followPoint);
		player.HandlePlayerGrowth();
		
		for (auto& f : foodList)
		{
			player.HandleCollision(f);
		}


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

			startTime = 0;

		}

		if (aiMoveTime >= .05f)
		{
			for (auto& c : creatures)
			{
				if (rndMoveDirection(mt2) == 0)
				{
					c.MoveUp();
				}
				else if(rndMoveDirection(mt2) == 1)
				{
					c.MoveDown();
				}
				else if (rndMoveDirection(mt2) == 2)
				{
					c.MoveLeft();
				}
				else if (rndMoveDirection(mt2) == 3)
				{
					c.MoveRight();
				}
				else if (rndMoveDirection(mt2) == 4)
				{
					c.MoveUp();
					c.MoveLeft();
				}
				else if (rndMoveDirection(mt2) == 5)
				{
					c.MoveUp();
					c.MoveRight();
				}
				else if (rndMoveDirection(mt2) == 6)
				{
					c.MoveDown();
					c.MoveLeft();
				}
				else if (rndMoveDirection(mt2) == 7)
				{
					c.MoveUp();
					c.MoveRight();
				}

				std::cout << rndMoveDirection(mt2) << std::endl;
			}

			aiMoveTime = 0;
		}
		

		RenderFood();

		for (auto& c : creatures)
		{
			c.UpdateCore(this, c.followPoint);
			c.RenderDetectionRadius(this);
			
			for(auto& f : foodList)
			{
				c.HandleCollision(f);
				c.HandleDetectingFood(f);
			}

		}

		// End Program On Escape Input

		if (GetKey(olc::ESCAPE).bPressed)
		{
			this->olc_Terminate();
		}
	
		return true;
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
