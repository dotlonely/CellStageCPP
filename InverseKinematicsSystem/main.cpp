#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const float PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062;

struct Node {
	Node* next = 0;
	Node* prev = 0;
	float radius = 4.0;
	//float offset = (radius * 2.0);
	float angle = 1.0;
	olc::vf2d center = olc::vf2d(100, 100);
	olc::vf2d pointA = olc::vf2d(1, 1);
	olc::vf2d pointB = olc::vf2d(1, 1);
	float length = 10.0;

	Node(float x, float y, float radius, float angle) {
		pointA.x = x;
		pointA.y = y;
		this->radius = radius;
		this->angle = angle;
		CalculatePointB();
	}

	Node(Node* prev, float radius) {
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

	void Follow(Node* node) {
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

struct Arm{
	Node* head;
	Node* tail;
	int size = 0;

	void AddFirst(Node* node) {
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


	void AddLast(Node* node) {
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

	olc::vf2d followPoint = olc::vf2d(320, 130);

	Arm arm;
	
	Node* baseNode;
	Node* grabNode;

	int numNodes = 5;
	int maxIterations = 10;
	int iterations = 0;

	float moveSpeed = .05;

public:
	InverseKinematicsSystem()
	{
		sAppName = "InverseKinematicsSystem";
	}

public:
	bool OnUserCreate() override
	{
		Node* startNode = new Node(basePoint.x, basePoint.y, 2.0, 0.0);
		Node* secondNode = new Node(startNode, 3.0);
		Node* thirdNode = new Node(secondNode, 4.0);
		Node* fourthNode = new Node(thirdNode, 5.0);
		Node* fifthNode = new Node(fourthNode, 3.0);

		//Add first node to list
		arm.AddFirst(startNode);
		if (arm.size == 1) {
			std::cout << "Added Node \n";
		}

		//Add second node to list as we need a reference node to create new nodes from
		arm.AddLast(secondNode);
		if (arm.size == 2) {
			std::cout << "Added Second Node \n";
		}

		arm.AddLast(thirdNode);
		if (arm.size == 3) {
			std::cout << "Added Third Node \n";
		}
		
		arm.AddLast(fourthNode);
		if (arm.size == 4) {
			std::cout << "Added Fourth Node \n";
		}
		
		arm.AddLast(fifthNode);
		if (arm.size == 5) {
			std::cout << "Added Fifth Node \n";
		}

		// Problem might stem from all added nodes having to initially reference the same node
		// when populating list ( Update: when adding them all individually they all get drawn to screen ) 
		// So this loop was causing an issue as they all had the same prev node reference.

		/*for (int i = 0; i < numNodes - 2; ++i) {
			Node newNode = Node((*secondStartNode).prev, 3.0);
			newNode.radius += i;
			arm.AddLast(&newNode);
			std::cout << "Added New Node " << i << "\n";
		}*/
	

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

		if (GetKey(olc::Key::W).bHeld) {
			followPoint.y -= moveSpeed;
		}
		
		if (GetKey(olc::Key::S).bHeld) {
			followPoint.y += moveSpeed;
		}

		if (GetKey(olc::Key::A).bHeld) {
			followPoint.x -= moveSpeed;
		}
		
		if (GetKey(olc::Key::D).bHeld) {
			followPoint.x += moveSpeed;
		}

		//Set first and last references every update
		baseNode = arm.head;
		//baseNode->prev = NULL;
		grabNode = arm.tail;
		//grabNode->next = NULL;

		grabNode->Follow(followPoint);
		grabNode->CalculatePointB();
		grabNode->CalculateCenter();

		DrawCircle(followPoint.x, followPoint.y, 1.0, olc::YELLOW);
		//DrawLine(grabNode->pointA, GetMousePos(), olc::WHITE);
		//DrawLine(grabNode->pointB, GetMousePos());
		DrawCircle(grabNode->pointA, 1, olc::RED);
		DrawCircle(grabNode->pointB, 1, olc::BLUE);
		DrawCircle(grabNode->center, grabNode->radius, olc::WHITE);
		
		baseNode->Follow();
		baseNode->CalculatePointB();
		baseNode->CalculateCenter();

		DrawCircle(baseNode->pointA, 1, olc::RED);
		DrawCircle(baseNode->pointB, 1, olc::BLUE);
		DrawCircle(baseNode->center, baseNode->radius, olc::WHITE);

		//DrawLine(baseNode->pointA, baseNode->next->pointA);
		//DrawLine(baseNode->pointB, baseNode->next->pointB);
		//DrawLine(baseNode->center, baseNode->next->center, olc::GREEN);

		// Attempt at while loop solution, definitely the closest I have gotten

		Node* current = baseNode->next;

		while (current->next != NULL) {

		
			current->Follow();
			current->CalculatePointB();
			current->CalculateCenter();

			DrawCircle(current->pointA, 1, olc::RED);
			DrawCircle(current->pointB, 1, olc::BLUE);
			//DrawLine(current->center, current->next->center, olc::GREEN);
			//DrawLine(current->pointA, current->next->pointA, olc::RED);
			//DrawLine(current->pointB, current->next->pointB, olc::BLUE);
			RenderNode(current);

			current = current->next;
		}


		//end while loop attempt
		
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
