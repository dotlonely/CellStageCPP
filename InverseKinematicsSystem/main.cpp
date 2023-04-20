#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

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

class InverseKinematicsSystem : public olc::PixelGameEngine
{

	float cursorSpeed = 100;

	float fTargetFrameTime = 1.0f / 100.0f; // Virtual FPS of 100fps
	float fAccumulatedTime = 0.0f;

	float nextFollowPointTime = 3.0f;
	float startTime = 0;

	float moveSpeed = 2;

	bool movePointSet = false;

	std::vector<olc::vf2d> movePoints;

	olc::vf2d basePoint = olc::vf2d(320.0f, 320.0f);

	olc::vf2d p0 = olc::vf2d(100, 300);
	olc::vf2d p1 = olc::vf2d(0, 0);
	olc::vf2d p2 = olc::vf2d(0, 0);
	olc::vf2d x = olc::vf2d(0, 0);
	olc::vf2d x1 = olc::vf2d(0, 0);
	olc::vf2d x2 = olc::vf2d(0, 0);

	olc::vf2d clickPoint = olc::vf2d(0, 0);
	olc::vf2d followPoint = olc::vf2d(0, 0);

	olc::vf2d cursorPosition = olc::vf2d(ScreenWidth() / 2, ScreenHeight() / 2);

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

		// Handle player input
		Input(fElapsedTime);

		// Draw Player Cursor
		DrawCircle(cursorPosition, 2, olc::YELLOW);

		FillCircle(p0, 2, olc::WHITE);
		FillCircle(p1, 2, olc::WHITE);
		FillCircle(p2, 2, olc::WHITE);

		if (startTime >= nextFollowPointTime) 
		{
			srand(time(0));
			p0 = olc::vf2d(rand() % ScreenWidth(), rand() % ScreenHeight());
			srand(time(0) * 2);
			p1 = olc::vf2d(rand() % ScreenWidth(), rand() % ScreenHeight());
			srand(time(0) * 4);
			p2 = olc::vf2d(rand() % ScreenWidth(), rand() % ScreenHeight());

			startTime = 0;

			std::cout << "p0 " << p0 << std::endl;
			std::cout << "p1 " << p1 << std::endl;
			std::cout << "p2 " << p2 << std::endl;
		}
		else
		{
			
			//olc::vf2d x1 = p0.lerp(p1, fElapsedTime);
			//olc::vf2d x2 = p1.lerp(p2, fElapsedTime);

			x = p0.lerp(p2, fElapsedTime);

			FillCircle(p0, 2, olc::RED);
			FillCircle(p1, 2, olc::GREEN);
			FillCircle(p2, 2, olc::YELLOW);
			FillCircle(x, 2, olc::BLUE);


		}


		UpdateWorm(followPoint);

		PrintMovePoints();

		return true;
	}


	// Draws specific segments circle
	void RenderSegment(Segment* segment, olc::Pixel color) {
		FillCircle(segment->center, segment->radius, color);
	}

	// Handles User Input, called before any rendering is done
	void Input(float fElapsedTime) {
		
		if (GetKey(olc::W).bHeld) 
		{
			cursorPosition.y -= cursorSpeed * fElapsedTime;
		}

		if (GetKey(olc::S).bHeld)
		{
			cursorPosition.y += cursorSpeed * fElapsedTime;
		}

		if (GetKey(olc::A).bHeld)
		{
			cursorPosition.x -= cursorSpeed * fElapsedTime;
		}

		if (GetKey(olc::D).bHeld)
		{
			cursorPosition.x += cursorSpeed * fElapsedTime;
		}

		if (GetKey(olc::SPACE).bPressed)
		{
			std::cout << "PRESSED SPACE" << std::endl;
			SetMovePoint();
			MoveToPoint(fElapsedTime);
		}
	}

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

	void UpdateFollowPoint(olc::vf2d moveToPoint, float fElapsedTime)
	{
		followPoint = followPoint.lerp(moveToPoint, moveSpeed * fElapsedTime);
	}


	void SetMovePoint()
	{
		olc::vf2d point = olc::vf2d(cursorPosition.x, cursorPosition.y);
		movePoints.emplace_back(point);
	}


	void MoveToPoint(float fElapsedTime) 
	{
		if (!movePoints.empty()) 
		{

			UpdateFollowPoint(movePoints.back(), fElapsedTime);

			if (worm.head->pointB == movePoints.back()) 
			{
				movePoints.pop_back();
			}
		}
	}

	void PrintMovePoints()
	{
		for (std::vector<olc::vf2d>::iterator itr = movePoints.begin(); itr != movePoints.end(); ++itr)
		{
			std::cout << *itr << std::endl;
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
