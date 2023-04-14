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
		if(!std::isnan(pointA.x) && !std::isnan(pointA.x))
		{
			CalculatePointB();
		}
		else 
		{
			pointB = olc::vf2d(10, 10);
		}
	}

	Node(Node* prev, float radius) {
		this->prev = prev;
		this->radius = radius;
		pointA = prev->pointB;
		if (!std::isnan(pointA.x) && !std::isnan(pointA.y)) {
			CalculatePointB();
		}
		else 
		{
			pointB = olc::vf2d(10, 10);
		}
	}


	void CalculatePointB() {
		//if (!std::isnan(pointA.x) && (!std::isnan(pointA.y))) {
			float dx = length * std::cos(angle * (PI / 180.0));
			float dy = length * std::sin(angle * (PI / 180.0));
			pointB.x = pointA.x - dx;
			pointB.y = pointA.y - dy;
		//}
		/*else {
			pointB.x = pointA.x - 5.0;
			pointB.y = pointA.y - 5.0;
		}*/
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

	Arm arm;
	
	Node* baseNode;
	Node* grabNode;

	int numNodes = 5;
	int maxIterations = 10;
	int iterations = 0;

public:
	InverseKinematicsSystem()
	{
		sAppName = "InverseKinematicsSystem";
	}

public:
	bool OnUserCreate() override
	{
		Node* startNode = new Node(basePoint.x, basePoint.y, 3.0, 0.0);
		Node* secondNode = new Node(startNode, 3.0);
		Node* thirdNode = new Node(secondNode, 3.0);
		Node* fourthNode = new Node(thirdNode, 3.0);
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

		//Set first and last references every update
		baseNode = arm.head;
		//baseNode->prev = NULL;
		grabNode = arm.tail;
		//grabNode->next = NULL;

		grabNode->Follow(olc::vf2d(GetMouseX(), GetMouseY()));
		grabNode->CalculatePointB();
		grabNode->CalculateCenter();
		grabNode->SetPointA(grabNode->prev->pointB);

		DrawCircle(grabNode->center, grabNode->radius, olc::RED);

		DrawCircle(GetMouseX(), GetMouseY(), 3.0, olc::YELLOW);
		DrawLine(grabNode->center, GetMousePos(), olc::GREEN);
		//DrawLine(grabNode->center, grabNode->prev->center);
		
		baseNode->Follow(baseNode->next);
		baseNode->CalculatePointB();
		baseNode->CalculateCenter();

		DrawCircle(baseNode->center, baseNode->radius, olc::BLUE);
		DrawLine(baseNode->center, baseNode->next->center, olc::GREEN);

		/*Node* current = grabNode;

		for (int i = numNodes - 2; i >= 0; --i) {

				std::cout << "Looping " << i << "\n";
				current->Follow(current->prev);
				current->CalculatePointB();
				current->CalculateCenter();

				DrawCircle(current->center, 3, olc::WHITE);
				DrawLine(current->center, current->prev->center, olc::GREEN);

				current = current->prev;
			
		}*/


		// Attempt at while loop solution, definitely the closest I have gotten

		Node* current = grabNode;

		while (current != baseNode) {

		
			current->Follow(current->prev);
			current->CalculatePointB();
			current->CalculateCenter();

			DrawCircle(current->center, 3.0, olc::WHITE);
			DrawLine(current->center, current->prev->center, olc::GREEN);


			current = current->prev;
	
			
			/*std::cout << iterations << "\n";
			iterations++;

			if (iterations >= 30) {
				iterations = 0;
				break;
			}*/
		}


		//end while loop attempt






		baseNode->SetPointA(basePoint);

		
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
