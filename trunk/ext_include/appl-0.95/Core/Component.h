#ifndef Component_H
#define Component_H

#include <vector>
#include <string>

using namespace std;

class Component
{
private:
	vector<Component *> subComponents;
	Component *parent;
	string name;

public:
    Component(void);

    virtual ~Component(void);
	
    Component* getParentComponent();

    void add(Component *childComponent);

    string getName();
	
    Component* getComponent(string name);

	// component
	virtual void initialize() = 0;

};


#endif

