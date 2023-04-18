#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const float PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062f;

struct Segment {
	Segment* next = NULL;
	Segment* prev = NULL;
	float radius = 4.0;
	float angle = 1.0;
	olc::vf2d center = olc::vf2d(100, 100);
	olc::vf2d pointA = olc::vf2d(1, 1);
	olc::vf2d pointB = olc::vf2d(1, 1);
	float length = 10.0;

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

class InverseKinematicsSystem : public olc::PixelGameEngine
{

	float fTargetFrameTime = 1.0f / 100.0f; // Virtual FPS of 100fps
	float fAccumulatedTime = 0.0f;
	float nextFollowPointTime = 3.0f;
	float startTime;

	olc::vf2d basePoint = olc::vf2d(320.0f, 320.0f);

	olc::vf2d followPoint = olc::vf2d(100,100);
	olc::vf2d nextFollowPoint;

	Worm worm;

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
		//Main worm segments
		Segment* startNode = new Segment(basePoint.x, basePoint.y, 2.0, 0.0);
		Segment* secondNode = new Segment(startNode, 3.0);
		Segment* thirdNode = new Segment(secondNode, 4.0);
		Segment* fourthNode = new Segment(thirdNode, 5.0);
		Segment* fifthNode = new Segment(fourthNode, 3.0);

		//Add segments to list
		worm.AddFirst(startNode);
		worm.AddLast(secondNode);
		worm.AddLast(thirdNode);
		worm.AddLast(fourthNode);
		worm.AddLast(fifthNode);


		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		startTime += fElapsedTime;

		//Handles framerate locking
		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= fTargetFrameTime)
		{
			fAccumulatedTime -= fTargetFrameTime;
			fElapsedTime = fTargetFrameTime;
		}
		else
			return true;

		// Clear screen every frame
		Clear(olc::BLACK);

		//Handle player input
		Input(fElapsedTime);

		if (startTime >= nextFollowPointTime) {
			olc::vf2d nextPoint = olc::vf2d(rand() % ScreenWidth(), rand() % ScreenHeight());
			nextFollowPoint = nextPoint;
			startTime = 0;
			std::cout << "Drew next point at" << nextPoint << std::endl;
		}else{
			followPoint = followPoint.lerp(nextFollowPoint, fElapsedTime);
			FillCircle(nextFollowPoint, 2, olc::DARK_GREEN);
		}

		UpdateWorm();

		return true;
	}


	// Draws specific segments circle
	void RenderSegment(Segment* segment, olc::Pixel color) {
		FillCircle(segment->center, segment->radius, color);
	}

	// Handles User Input, called before any rendering is done
	void Input(float fElapsedTime) {


	}

	void UpdateWorm() 
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
};


int main()
{
	InverseKinematicsSystem demo;
	if (demo.Construct(640, 360, 2, 2))
		demo.Start();
	return 0;
}
