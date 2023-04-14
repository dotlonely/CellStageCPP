#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const float PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062f;

struct Segment {
	Segment* next = NULL;
	Segment* prev = NULL;
	float radius = 4.0;
	//float offset = (radius * 2.0);
	float angle = 1.0;
	olc::vf2d center = olc::vf2d(100, 100);
	olc::vf2d pointA = olc::vf2d(1, 1);
	olc::vf2d pointB = olc::vf2d(1, 1);
	float length = 10.0;
	bool hasLegs = false;
	Segment* legOne = NULL;
	Segment* legTwo = NULL;
	float legOneOffset = 10.0f;
	float legTwoOffset = -10.0f;

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

	void AddLegs() {
		hasLegs = true;
		legOne = new Segment(this, 1.0f);
		legTwo = new Segment(this, 1.0f);
		legOne->next = this;
		legTwo->next = this;
		legOne->prev = NULL;
		legTwo->prev = NULL;
		legOne->pointA = this->pointB;
		legTwo->pointA = this->pointB;
		legOne->CalculateLegOnePointB();
		legTwo->CalculateLegTwoPointB();
	}

	void CalculateLegOnePointB() {
		float dx = length * std::cos(angle * (PI / 180.0));
		float dy = length * std::sin(angle * (PI / 180.0));
		pointB.x = pointA.x - dx + legOneOffset;
		pointB.y = pointA.y - dy + legOneOffset;
	}

	void CalculateLegTwoPointB() {
		float dx = length * std::cos(angle * (PI / 180.0));
		float dy = length * std::sin(angle * (PI / 180.0));
		pointB.x = pointA.x - dx + legTwoOffset;
		pointB.y = pointA.y - dy + legTwoOffset;
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

class InverseKinematicsSystem : public olc::PixelGameEngine
{

	float fTargetFrameTime = 1.0f / 100.0f; // Virtual FPS of 100fps
	float fAccumulatedTime = 0.0f;

	olc::vf2d basePoint = olc::vf2d(320.0f, 320.0f);

	olc::vf2d followPoint = olc::vf2d(320.0f, 130.0f);

	Worm arm;
	
	Segment* baseSegment;
	Segment* grabSegment;

	int numNodes = 5;
	int numLegs = 2;
	int maxIterations = 10;
	int iterations = 0;

	float initialVelocity = 100.0f;
	float acceleration = 9.8f;
	float decceleration = -9.8f;
	float velocity = 0.0f;
	float maxVelocity = 10.0f;

public:
	InverseKinematicsSystem()
	{
		sAppName = "InverseKinematicsSystem";
	}

public:
	bool OnUserCreate() override
	{
		Segment* startNode = new Segment(basePoint.x, basePoint.y, 2.0, 0.0);
		Segment* secondNode = new Segment(startNode, 3.0);
		Segment* thirdNode = new Segment(secondNode, 4.0);
		Segment* fourthNode = new Segment(thirdNode, 5.0);
		Segment* fifthNode = new Segment(fourthNode, 3.0);

		//Add first segment to list
		arm.AddFirst(startNode);
		if (arm.size == 1) {
			std::cout << "Added Segment \n";
		}

		//Add second segment to list as we need a reference segment to create new nodes from
		arm.AddLast(secondNode);
		if (arm.size == 2) {
			std::cout << "Added Second Segment \n";
		}

		arm.AddLast(thirdNode);
		if (arm.size == 3) {
			std::cout << "Added Third Segment \n";
		}
		
		arm.AddLast(fourthNode);
		if (arm.size == 4) {
			std::cout << "Added Fourth Segment \n";
		}
		
		arm.AddLast(fifthNode);
		if (arm.size == 5) {
			std::cout << "Added Fifth Segment \n";
		}

		fourthNode->AddLegs();
	
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		//Handles framerate locking
		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= fTargetFrameTime)
		{
			fAccumulatedTime -= fTargetFrameTime;
			fElapsedTime = fTargetFrameTime;
		}
		else
			return true;


		Clear(olc::BLACK);


		//Handle player input
		Input(fElapsedTime);


		//Set first and last references every update
		baseSegment = arm.head;
		baseSegment->prev = NULL;
		grabSegment = arm.tail;
		grabSegment->next = NULL;

		grabSegment->Follow(followPoint);
		grabSegment->CalculatePointB();
		grabSegment->CalculateCenter();

		DrawCircle(followPoint.x, followPoint.y, 1.0, olc::YELLOW);
		DrawCircle(grabSegment->pointA, 1, olc::RED);
		DrawCircle(grabSegment->pointB, 1, olc::BLUE);
		DrawCircle(grabSegment->center, grabSegment->radius, olc::WHITE);
		
		baseSegment->Follow();
		baseSegment->CalculatePointB();
		baseSegment->CalculateCenter();

		DrawCircle(baseSegment->pointA, 1, olc::RED);
		DrawCircle(baseSegment->pointB, 1, olc::BLUE);
		DrawCircle(baseSegment->center, baseSegment->radius, olc::WHITE);


		Segment* current = baseSegment->next;

		while (current->next != NULL) {

		
			current->Follow();
			current->CalculatePointB();
			current->CalculateCenter();

			DrawCircle(current->pointA, 1, olc::RED);
			DrawCircle(current->pointB, 1, olc::BLUE);
			RenderSegment(current, olc::WHITE);

			if (current->hasLegs) {
				current->legOne->Follow();
				current->legOne->CalculateLegOnePointB();
				current->legOne->CalculateCenter();
				RenderSegment(current->legOne, olc::YELLOW);
				current->legTwo->Follow();
				current->legTwo->CalculateLegTwoPointB();
				current->legTwo->CalculateCenter();
				RenderSegment(current->legTwo, olc::YELLOW);
			}

			current = current->next;
		}

		
		return true;
	}

	void RenderSegment(Segment* segment, olc::Pixel color) {
		DrawCircle(segment->center, segment->radius, color);
	}

	void Input(float fElapsedTime) {

		
		if (GetKey(olc::Key::W).bHeld) {
			velocity = initialVelocity + acceleration * fElapsedTime;
			followPoint.y -= velocity * fElapsedTime;
		}

		if (GetKey(olc::Key::S).bHeld) {
			velocity = initialVelocity + acceleration * fElapsedTime;
			followPoint.y += velocity * fElapsedTime;
		}

		if (GetKey(olc::Key::A).bHeld) {
			velocity = initialVelocity + acceleration * fElapsedTime;
			followPoint.x -= velocity * fElapsedTime;
		}

		if (GetKey(olc::Key::D).bHeld) {
			velocity = initialVelocity + acceleration * fElapsedTime;
			followPoint.x += velocity * fElapsedTime;
		}

		if (velocity > 0.0) {
			velocity += decceleration * fElapsedTime;
		}
		
	}

	float CalculateVelocity(float currentVelocity, float fElapsedTime) {
		return currentVelocity + (acceleration * fElapsedTime);
	}
};

int main()
{
	InverseKinematicsSystem demo;
	if (demo.Construct(640, 360, 2, 2))
		demo.Start();
	return 0;
}
