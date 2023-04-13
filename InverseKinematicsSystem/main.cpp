#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const float PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062;

struct Node {
	Node* next = nullptr;
	Node* prev = nullptr;
	float radius = 4;
	float offset = (radius * 2.0);
	float angle = 90;
	olc::vf2d center;
	olc::vf2d pointA = olc::vf2d(center.x , center.y + offset);
	olc::vf2d pointB;
	float length = 10;

	Node(float x, float y, float radius, float angle) {
		this->center.x = x;
		this->center.y = y;
		this->radius = radius;
		this->angle = angle;
		CalculatePointB();
	}

	Node(Node* prev, float radius) {
		this->next = next;
		this->prev = prev;
		this->radius = radius;
		pointA = prev->pointB;
		CalculatePointB();
	}


	void CalculatePointB() {
		float dx = length * std::cos(angle * (PI / 180));
		float dy = length * std::sin(angle * (PI / 180));
		pointB.x = center.x - dx;
		pointB.y = center.y - dy;
	}

	void CalculateCenter() {
		center = olc::vf2d((pointB - pointA) / 2.0);
	}

	void Follow(olc::vf2d target) {
		olc::vf2d dir = (target - pointA).norm();
		angle = std::atan2(dir.y, dir.x) * (180 / PI);
		dir = dir * length;
		dir *= -1;

		pointA = target + dir;
	}

	void Follow(Node* node) {
		Node temp = *node;
		olc::vf2d target = olc::vf2d(temp.pointA.x, temp.pointA.y);
		Follow(target);

		Follow(olc::vf2d((*node).pointA.x, (*node).pointA.y));
	}

	void SetPointA(olc::vf2d position) {
		pointA = position;
		CalculatePointB();
	}

	void UpdateNode() {
		Follow(next);
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

	Arm arm;
	
	Node* first;
	Node* last;

	int numNodes = 5;

public:
	InverseKinematicsSystem()
	{
		sAppName = "InverseKinematicsSystem";
	}

public:
	bool OnUserCreate() override
	{

		Node* baseNode = new Node(320, 320, 2, 0);
		Node* tail = new Node(baseNode, 3);


		//Add first node to list
		arm.AddFirst(baseNode);
		if (arm.size == 1) {
			std::cout << "Added First Node \n";
		}


		//Add second node to list as we need a reference node to create new nodes from
		arm.AddLast(tail);
		if (arm.size == 2) {
			std::cout << "Added Second Node \n";
		}


		for (int i = 2; i < numNodes; ++i) {
			Node newNode = Node((*tail).prev, 3);
			arm.AddLast(&newNode);
			std::cout << "Added New Node " << i << "\n";
		}


		//Set first and last references 
		first = arm.head;
		last = arm.tail;

		first->length = 10;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		Clear(olc::BLACK);

		last->Follow(olc::vf2d(GetMouseX(), GetMouseY()));
		last->CalculatePointB();
		//last->CalculateCenter();
		
		DrawCircle(first->center, first->radius, olc::BLUE);
		//DrawCircle(last->center, last->radius, olc::BLUE);

		Node* current = last;

		for (int i = numNodes - 1; i >= 0; --i) {

			if (current->prev != NULL) {
				//DrawLine(current->center, current->prev->center, olc::WHITE);
				DrawCircle(current->pointA, 2, olc::YELLOW);
				DrawCircle(current->pointB, 2, olc::RED);
			}

			current = current->prev;
		}

		//last->SetPointA(olc::vf2d(320, 130));

		
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
