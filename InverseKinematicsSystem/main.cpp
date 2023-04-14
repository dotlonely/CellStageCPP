#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const float PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062;

struct Node {
	Node* next = nullptr;
	Node* prev = nullptr;
	float radius = 4;
	//float offset = (radius * 2.0);
	float angle = 0;
	olc::vf2d center;
	olc::vf2d pointA;
	olc::vf2d pointB;
	float length = 10;

	Node(float x, float y, float radius, float angle) {
		pointA.x = x;
		pointA.y = y;
		this->radius = radius;
		this->angle = angle;
		CalculatePointB();
	}

	Node(Node* prev, float radius) {
		this->radius = radius;
		angle = 0;
		pointA = prev->pointB;
		CalculatePointB();
	}


	void CalculatePointB() {
		float dx = length * std::cos(angle * (PI / 180));
		float dy = length * std::sin(angle * (PI / 180));
		pointB.x = pointA.x - dx;
		pointB.y = pointA.y - dy;
	}

	void CalculateCenter() {
		center = (pointA + pointB) / 2.0;
	}

	void Follow(olc::vf2d target) {
		olc::vf2d dir = (target - pointA).norm();
		angle = std::atan2(dir.y, dir.x) * (180 / PI);
		dir = dir * length;
		dir *= -1;

		pointA = target + dir;
	}

	void Follow(Node* node) {
		olc::vf2d target = olc::vf2d(node->pointA.x, node->pointA.y);
		Follow(target);
	}

	void SetPointA(olc::vf2d position) {
		pointA = position;
		CalculatePointB();
	}
};

struct Arm{
	Node* head;
	Node* tail;
	int size = 0;

	void AddFirst(Node* node) {
		if (size == 0) {
			head = node;
			tail = node;
		}

		if (head == tail) {
			node->next = tail;
			tail->prev = node;
			head = node;
		}

		node->next = head;
		head->prev = node;
		head = node;
		size++;
	}


	void AddLast(Node* node) {
		if (size == 0) {
			head = node;
			tail = node;
		}

		if (head == tail) {
			head->next = node;
			node->prev = head;
			tail = node;
		}

		node->prev = tail;
		tail->next = node;
		tail = node;
		size++;   
	}

	int Size() {
		if (head == nullptr && tail == nullptr) 
			return 0;

		if (head == tail)
			return 1;

		int count = 0;

		Node* current = head;

		while (current->next != NULL) {
			current = current->next;
			count++;
		}

		return count;
	}

};

class InverseKinematicsSystem : public olc::PixelGameEngine
{
	olc::vf2d basePoint = olc::vf2d(320, 320);

	Arm arm;
	
	Node* baseNode;
	Node* grabNode;

	int numNodes = 4;

public:
	InverseKinematicsSystem()
	{
		sAppName = "InverseKinematicsSystem";
	}

public:
	bool OnUserCreate() override
	{

		Node* startNode = new Node(basePoint.x, basePoint.y, 3, 0);
		Node* secondStartNode = new Node(startNode, 3);

		//Add first node to list
		arm.AddFirst(startNode);
		if (arm.size == 1) {
			std::cout << "Added Node \n";
		}

		//Add second node to list as we need a reference node to create new nodes from
		arm.AddLast(secondStartNode);
		if (arm.size == 2) {
			std::cout << "Added Second Node \n";
		}

		for (int i = 0; i < numNodes - 2; ++i) {
			Node newNode = Node((*secondStartNode).prev, 3);
			arm.AddLast(&newNode);
			std::cout << "Added New Node " << i << "\n";
		}

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		Clear(olc::BLACK);

		//Set first and last references every update
		baseNode = arm.head;
		baseNode->prev = nullptr;
		grabNode = arm.tail;
		grabNode->next = nullptr;

		grabNode->Follow(olc::vf2d(GetMouseX(), GetMouseY()));
		grabNode->CalculatePointB();
		grabNode->CalculateCenter();

		DrawCircle(grabNode->center, grabNode->radius, olc::WHITE);
		//DrawCircle(grabNode->pointA, 1, olc::GREEN);
		//DrawCircle(grabNode->pointB, 1, olc::RED);
		//DrawLine(grabNode->pointA, grabNode->pointB, olc::GREEN);

		DrawCircle(GetMouseX(), GetMouseY(), 3,olc::YELLOW);
		DrawLine(grabNode->center, GetMousePos(), olc::GREEN);
		
		//baseNode->Follow(baseNode->next);
		baseNode->CalculatePointB();
		baseNode->CalculateCenter();

		DrawCircle(baseNode->center, baseNode->radius + 2, olc::BLUE);
		DrawLine(baseNode->center, baseNode->next->center);
		//DrawLine(baseNode->pointA, baseNode->pointB, olc::GREEN);
		//DrawCircle(baseNode->pointA, 1, olc::GREEN);
		//DrawCircle(baseNode->pointB, 1, olc::RED);
		//DrawLine(baseNode->pointA, baseNode->pointB, olc::RED);

		Node* current = baseNode;

		//for (int i = numNodes - 1; i >= 0; --i) {

		//		std::cout << "Looping " << i << "\n";
		//		backCurrent->Follow(backCurrent->prev);
		//		backCurrent->CalculatePointB();
		//		backCurrent->CalculateCenter();

		//		DrawCircle(backCurrent->center, 3, olc::WHITE);

		//		//Draw pointA and pointB
		//		DrawCircle(backCurrent->pointA, 1, olc::GREEN);
		//		DrawCircle(backCurrent->pointB, 1, olc::RED);

		//		backCurrent = backCurrent->prev;
		//	
		//}

		while (current->next != nullptr) {

			current->Follow(current->next);
			current->CalculatePointB();
			current->CalculateCenter();

			DrawCircle(current->center, 3, olc::WHITE);
			//DrawLine(current->pointA, current->pointB, olc::GREEN);
			DrawLine(current->center, current->next->center, olc::GREEN);

			//Draw pointA and pointB
			//DrawCircle(current->pointA, 1, olc::GREEN);
			//DrawCircle(current->pointB, 1, olc::RED);

			current = current->next;
		}

		//baseNode->SetPointA(basePoint);
		//baseNode->pointA = olc::vf2d(basePoint.x, basePoint.y);

		
		return true;
	}

	void RenderNode(Node* node) {
		DrawCircle(node->center, node->radius, olc::WHITE);
	}
};

int main()
{
	InverseKinematicsSystem demo;
	if (demo.Construct(640, 360, 2, 2))
		demo.Start();
	return 0;
}
